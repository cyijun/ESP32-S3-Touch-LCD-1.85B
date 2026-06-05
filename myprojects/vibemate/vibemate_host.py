#!/usr/bin/env python3
"""
VibeMate Host Server

Handles UDP discovery, TCP audio streaming, and control messages for
VibeMate ESP32-S3 smartwatch voice communication.

Protocol:
- UDP port 3721: device discovery (broadcast)
- TCP port 3722: audio frames + control messages
- Audio: 16kHz, 16-bit, mono, 20ms frames (640 bytes PCM)
"""

import asyncio
import collections
import json
import socket
import struct
import sys
import time

import numpy as np
import sounddevice as sd

# ---------------------------------------------------------------------------
# Constants
# ---------------------------------------------------------------------------

UDP_DISCOVERY_PORT = 3721
TCP_AUDIO_PORT = 3722

SAMPLE_RATE = 16000
BLOCKSIZE = 320  # 20ms @ 16kHz
CHANNELS = 1
DTYPE = np.int16

FRAME_MAGIC = b"VM"
FRAME_AUDIO_UPLINK = 0x01   # watch -> host
FRAME_AUDIO_DOWNLINK = 0x02  # host -> watch
FRAME_CONTROL = 0x03

# ---------------------------------------------------------------------------
# AudioRingBuffer
# ---------------------------------------------------------------------------

class AudioRingBuffer:
    """Thread-safe ring buffer for audio samples using collections.deque."""

    def __init__(self, maxlen: int = 50):
        self._deque: collections.deque = collections.deque(maxlen=maxlen)
        self._lock = asyncio.Lock()
        self._not_empty = asyncio.Event()

    def put(self, pcm: np.ndarray) -> None:
        self._deque.append(pcm)
        self._not_empty.set()

    def get(self) -> np.ndarray | None:
        try:
            return self._deque.popleft()
        except IndexError:
            self._not_empty.clear()
            return None

    async def get_wait(self, frames: int, timeout: float = 0.1) -> np.ndarray | None:
        try:
            await asyncio.wait_for(self._not_empty.wait(), timeout=timeout)
        except asyncio.TimeoutError:
            return None
        return self.get()

# ---------------------------------------------------------------------------
# UDP Discovery
# ---------------------------------------------------------------------------

class UDPDiscoveryProtocol(asyncio.DatagramProtocol):
    def __init__(self, host: "VibeMateHost"):
        self.host = host

    def datagram_received(self, data: bytes, addr: tuple[str, int]) -> None:
        try:
            msg = json.loads(data.decode())
        except (json.JSONDecodeError, UnicodeDecodeError):
            return

        if msg.get("type") == "discover" and msg.get("device") == "vibemate":
            reply = json.dumps({
                "type": "announce",
                "name": socket.gethostname(),
                "ip": self.host.local_ip,
                "port": TCP_AUDIO_PORT,
                "version": 1,
            })
            self.transport.sendto(reply.encode(), addr)
            print(f"[UDP] Replied discovery to {addr[0]}:{addr[1]}")

    def connection_made(self, transport) -> None:
        self.transport = transport

# ---------------------------------------------------------------------------
# VibeMateHost
# ---------------------------------------------------------------------------

class VibeMateHost:
    def __init__(self):
        self.local_ip = self._get_local_ip()
        self.mode = "ptt"
        self.connected = False
        self.reader: asyncio.StreamReader | None = None
        self.writer: asyncio.StreamWriter | None = None
        self._recv_task: asyncio.Task | None = None
        self._send_task: asyncio.Task | None = None
        self._shutdown_event = asyncio.Event()

        self.rx_buffer = AudioRingBuffer(maxlen=50)   # audio from watch -> speakers
        self.tx_buffer = AudioRingBuffer(maxlen=50)   # mic -> audio to watch

        self._last_ping = 0.0
        self._stream: sd.RawStream | None = None

    # -- network helpers ----------------------------------------------------

    @staticmethod
    def _get_local_ip() -> str:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        try:
            s.connect(("8.8.8.8", 80))
            ip = s.getsockname()[0]
        except Exception:
            ip = "127.0.0.1"
        finally:
            s.close()
        return ip

    def _build_frame(self, frame_type: int, payload: bytes) -> bytes:
        return FRAME_MAGIC + struct.pack("B", frame_type) + struct.pack(">H", len(payload)) + payload

    def _send_frame(self, frame_type: int, payload: bytes) -> None:
        if self.writer is None:
            return
        try:
            self.writer.write(self._build_frame(frame_type, payload))
        except Exception:
            pass

    def _send_control(self, ctrl: dict) -> None:
        payload = json.dumps(ctrl).encode()
        self._send_frame(FRAME_CONTROL, payload)

    # -- audio callback -----------------------------------------------------

    def audio_callback(self, indata: np.ndarray, outdata: np.ndarray,
                       frames: int, time_info, status) -> None:
        if status:
            print(f"[Audio] status: {status}", file=sys.stderr)

        # Play audio received from watch
        pcm = self.rx_buffer.get()
        if pcm is not None and len(pcm) == frames:
            outdata[:] = pcm
        else:
            outdata.fill(0)

        # In duplex mode, capture mic and queue for transmission
        if self.connected and self.mode == "duplex":
            self.tx_buffer.put(indata.copy())

    # -- TCP handlers -------------------------------------------------------

    async def _recv_frames(self) -> None:
        """Parse incoming frames from the watch."""
        while self.connected and self.reader is not None:
            try:
                header = await self.reader.readexactly(5)
            except (asyncio.IncompleteReadError, ConnectionResetError, OSError):
                break

            magic = header[0:2]
            if magic != FRAME_MAGIC:
                print("[TCP] Sync lost, skipping byte")
                continue

            frame_type = header[2]
            payload_len = struct.unpack(">H", header[3:5])[0]

            try:
                payload = await self.reader.readexactly(payload_len)
            except (asyncio.IncompleteReadError, ConnectionResetError, OSError):
                break

            if frame_type == FRAME_AUDIO_UPLINK:
                pcm = np.frombuffer(payload, dtype=DTYPE).reshape(-1, CHANNELS)
                self.rx_buffer.put(pcm)
            elif frame_type == FRAME_CONTROL:
                try:
                    ctrl = json.loads(payload.decode())
                except (json.JSONDecodeError, UnicodeDecodeError):
                    continue
                await self._handle_control(ctrl)

        print("[TCP] Receive loop ended")
        self._disconnect()

    async def _handle_control(self, ctrl: dict) -> None:
        cmd = ctrl.get("cmd")

        if cmd == "hello":
            self.mode = ctrl.get("mode", "ptt")
            self._send_control({
                "cmd": "hello_ack",
                "status": "ready",
                "sample_rate": SAMPLE_RATE,
            })
            print(f"[Control] hello -> mode={self.mode}")

        elif cmd == "ping":
            self._send_control({"cmd": "pong"})

        elif cmd == "ptt":
            state = ctrl.get("state")
            print(f"[Control] PTT {state}")

        elif cmd == "mode":
            new_mode = ctrl.get("type", "ptt")
            self.mode = new_mode
            print(f"[Control] mode -> {new_mode}")

    async def _send_frames(self) -> None:
        """Send audio downlink + heartbeat to the watch."""
        while self.connected:
            pcm = await self.tx_buffer.get_wait(BLOCKSIZE, timeout=0.005)
            if pcm is not None and len(pcm) == BLOCKSIZE:
                self._send_frame(FRAME_AUDIO_DOWNLINK, pcm.tobytes())

            # Heartbeat every 3s
            now = time.time()
            if now - self._last_ping > 3:
                self._send_control({"cmd": "pong"})
                self._last_ping = now

            await asyncio.sleep(0.005)

        print("[TCP] Send loop ended")

    async def handle_client(self, reader: asyncio.StreamReader,
                            writer: asyncio.StreamWriter) -> None:
        if self.connected:
            print("[TCP] Rejecting extra client (already connected)")
            writer.close()
            await writer.wait_closed()
            return

        self.connected = True
        self.reader = reader
        self.writer = writer
        self._last_ping = time.time()
        peer = writer.get_extra_info("peername")
        print(f"[TCP] Client connected from {peer}")

        self._recv_task = asyncio.create_task(self._recv_frames())
        self._send_task = asyncio.create_task(self._send_frames())

        try:
            await self._recv_task
        except asyncio.CancelledError:
            pass

        self._disconnect()
        print(f"[TCP] Client disconnected from {peer}")

    def _disconnect(self) -> None:
        if not self.connected:
            return
        self.connected = False

        for task in (self._recv_task, self._send_task):
            if task is not None and not task.done():
                task.cancel()

        self._recv_task = None
        self._send_task = None

        if self.writer is not None:
            self.writer.close()
        self.reader = None
        self.writer = None

    # -- lifecycle ----------------------------------------------------------

    async def run(self) -> None:
        loop = asyncio.get_running_loop()

        # UDP discovery
        udp_transport, _ = await loop.create_datagram_endpoint(
            lambda: UDPDiscoveryProtocol(self),
            local_addr=("0.0.0.0", UDP_DISCOVERY_PORT),
        )
        print(f"[UDP] Discovery listening on :{UDP_DISCOVERY_PORT}")

        # TCP server
        tcp_server = await asyncio.start_server(
            self.handle_client, "0.0.0.0", TCP_AUDIO_PORT
        )
        print(f"[TCP] Audio server listening on :{TCP_AUDIO_PORT}")

        # Audio stream
        self._stream = sd.RawStream(
            samplerate=SAMPLE_RATE,
            blocksize=BLOCKSIZE,
            channels=CHANNELS,
            dtype=DTYPE,
            callback=self.audio_callback,
        )
        self._stream.start()
        print(f"[Audio] Stream started: {SAMPLE_RATE}Hz, {BLOCKSIZE} samples/block")

        print("\nVibeMate Host is running. Press Ctrl+C to stop.\n")

        try:
            await self._shutdown_event.wait()
        except asyncio.CancelledError:
            pass
        finally:
            print("\n[Shutdown] Stopping...")
            self._disconnect()
            udp_transport.close()
            tcp_server.close()
            await tcp_server.wait_closed()
            if self._stream is not None:
                self._stream.stop()
                self._stream.close()
            print("[Shutdown] Done.")

    def shutdown(self) -> None:
        self._shutdown_event.set()


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main() -> None:
    host = VibeMateHost()

    loop = asyncio.new_event_loop()
    asyncio.set_event_loop(loop)

    main_task = loop.create_task(host.run())

    def on_sigint() -> None:
        print("\n[Signal] Ctrl+C received")
        host.shutdown()
        main_task.cancel()

    try:
        for sig in (signal.SIGINT, signal.SIGTERM):
            loop.add_signal_handler(sig, on_sigint)
    except NotImplementedError:
        pass  # Windows

    try:
        loop.run_until_complete(main_task)
    except asyncio.CancelledError:
        pass
    finally:
        loop.close()


if __name__ == "__main__":
    import signal
    main()
