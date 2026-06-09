#!/usr/bin/env python3
"""
test_tile_switch.py - Automated test for Pet -> Usage tile switch freeze bug.

Usage:
    python3 test_tile_switch.py

Requires: pip install pyserial

This script connects to the ESP32 via serial and repeatedly switches between
the Pet tile (t2) and Usage tile (t3), monitoring for freezes.
"""

import serial
import sys
import time
import argparse

SERIAL_PORT = "/dev/cu.usbmodem1101"
BAUDRATE = 115200
TIMEOUT = 0.1


def wait_for_output(ser, timeout_sec, expected=None):
    """Read serial output until timeout or expected string is found.
    Returns (lines, found_expected, last_line_time)."""
    lines = []
    found = False
    last_time = time.time()
    deadline = time.time() + timeout_sec

    while time.time() < deadline:
        line = ser.readline().decode("utf-8", errors="ignore").strip()
        if line:
            lines.append(line)
            last_time = time.time()
            if expected and expected in line:
                found = True
                break
    return lines, found, last_time


def hard_reset(ser):
    """Reset ESP32 via RTS pin (same as arduino-cli --chip-reset)."""
    print("Hard resetting board via RTS...")
    ser.setDTR(False)
    ser.setRTS(True)   # EN=LOW, chip in reset
    time.sleep(0.1)
    ser.setRTS(False)  # EN=HIGH, chip out of reset
    time.sleep(0.5)


def main():
    parser = argparse.ArgumentParser(description="Test Pet->Usage tile switch")
    parser.add_argument("--port", default=SERIAL_PORT, help="Serial port")
    parser.add_argument("--cycles", type=int, default=20, help="Number of switch cycles")
    parser.add_argument("--delay", type=float, default=2.0, help="Seconds between switches")
    parser.add_argument("--freeze-threshold", type=float, default=5.0,
                        help="Seconds without output to declare freeze")
    parser.add_argument("--use-device-cycle", action="store_true",
                        help="Use the device's built-in 'cycle' command instead of manual switching")
    parser.add_argument("--no-reset", action="store_true",
                        help="Skip hard reset before test")
    parser.add_argument("--raw", action="store_true",
                        help="Just open serial and print all output (debug mode)")
    args = parser.parse_args()

    print(f"Connecting to {args.port} at {BAUDRATE}...")
    try:
        ser = serial.Serial()
        ser.port = args.port
        ser.baudrate = BAUDRATE
        ser.timeout = TIMEOUT
        # On macOS with ESP32-S3 USB-Serial/JTAG, DTR/RTS default is fine.
        ser.open()
    except serial.SerialException as e:
        print(f"Failed to open serial port: {e}")
        sys.exit(1)

    if args.raw:
        print("Raw mode: printing all serial output. Press Ctrl+C to stop.")
        try:
            while True:
                line = ser.readline().decode("utf-8", errors="ignore").strip()
                if line:
                    print(line)
        except KeyboardInterrupt:
            print("\nStopped.")
        ser.close()
        return 0

    # Hard reset before test (unless --no-reset)
    if not args.no_reset:
        hard_reset(ser)

    # Drain any existing output
    time.sleep(0.5)
    ser.reset_input_buffer()

    # Wait for device to boot
    print("Waiting for device boot (looking for 'VibeMate ready')...")
    boot_deadline = time.time() + 30
    booted = False
    while time.time() < boot_deadline:
        lines, found, _ = wait_for_output(ser, 1.0, expected="VibeMate ready")
        for line in lines:
            print(f"  [BOOT] {line}")
        if found:
            booted = True
            print("Device booted successfully!")
            break

    if not booted:
        print("WARNING: Did not see 'VibeMate ready' - device may already be running.")

    # Echo test: send status command and see if device responds
    print("\nSending echo test command 'status'...")
    ser.write(b"status\n")
    ser.flush()
    lines, found, _ = wait_for_output(ser, 3.0, expected="[MAIN]")
    for line in lines:
        print(f"  {line}")
    if not lines:
        print("  (no response - device may not be responding to commands)")
        print("  Try: python3 test_tile_switch.py --raw")
        ser.close()
        return 1

    # Let things settle
    time.sleep(1)
    ser.reset_input_buffer()

    if args.use_device_cycle:
        # Use the device's built-in auto-cycle
        print(f"\nStarting device auto-cycle mode ({args.cycles} cycles)...")
        ser.write(b"cycle\n")
        ser.flush()
        print("Sent 'cycle' command to device.")

        last_output_time = time.time()
        cycle_count = 0
        test_deadline = time.time() + (args.cycles * args.delay * 2) + 30

        while time.time() < test_deadline and cycle_count < args.cycles:
            lines, _, last_time = wait_for_output(ser, args.freeze_threshold)
            for line in lines:
                print(f"  {line}")
                last_output_time = time.time()
                if "tileview changed" in line:
                    cycle_count += 1
                    print(f"  -> Detected tile change #{cycle_count}")

            if time.time() - last_output_time >= args.freeze_threshold:
                print(f"\n*** FREEZE DETECTED! No output for {args.freeze_threshold}s ***")
                print("Aborting test.")
                ser.close()
                return 1

        print(f"\n{'='*50}")
        print(f"Auto-cycle test: {cycle_count} tile changes detected, 0 freeze(s)")
    else:
        # Manual Pet <-> Usage switching
        print(f"\nStarting {args.cycles} Pet <-> Usage switch cycles...")
        print(f"Delay: {args.delay}s | Freeze threshold: {args.freeze_threshold}s")
        print("=" * 50)

        for cycle in range(1, args.cycles + 1):
            # Switch to Pet
            print(f"\nCycle {cycle}/{args.cycles}: Pet -> Usage")
            ser.write(b"t2\n")
            ser.flush()

            lines, pet_ok, last_time = wait_for_output(ser, args.freeze_threshold,
                                                        expected="tileview changed to tile=2")
            for line in lines:
                print(f"  {line}")

            if not pet_ok:
                if time.time() - last_time >= args.freeze_threshold:
                    print(f"  *** FREEZE going to Pet! ***")
                    print("Aborting test.")
                    ser.close()
                    return 1
                else:
                    print(f"  (no tile=2 confirmation, but output continued)")

            time.sleep(args.delay)

            # Switch to Usage
            ser.write(b"t3\n")
            ser.flush()

            lines, usage_ok, last_time = wait_for_output(ser, args.freeze_threshold,
                                                          expected="tileview changed to tile=3")
            for line in lines:
                print(f"  {line}")

            if not usage_ok:
                if time.time() - last_time >= args.freeze_threshold:
                    print(f"  *** FREEZE going to Usage! ***")
                    print("Aborting test.")
                    ser.close()
                    return 1
                else:
                    print(f"  (no tile=3 confirmation, but output continued)")

            if pet_ok and usage_ok:
                print(f"  -> OK")

            time.sleep(args.delay)

        print("\n" + "=" * 50)
        print(f"Manual switch test: {args.cycles} cycles, 0 freeze(s)")

    print("SUCCESS: No freezes detected!")

    ser.close()
    return 0


if __name__ == "__main__":
    sys.exit(main())
