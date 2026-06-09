#!/usr/bin/env python3
"""
reset_and_monitor.py - Reset ESP32-S3 via DTR/RTS and monitor serial output.

Usage:
    python3 reset_and_monitor.py           # Normal reset (no bootloader)
    python3 reset_and_monitor.py --boot    # Enter bootloader mode
    python3 reset_and_monitor.py --listen  # Just listen, no reset

Press Ctrl+C to stop.
"""

import serial
import sys
import time
import argparse

SERIAL_PORT = "/dev/cu.usbmodem1101"
BAUDRATE = 115200


def reset_normal(ser):
    """Normal reset: do NOT enter bootloader."""
    print("[RESET] Normal reset (no bootloader)...")
    # Start from idle state
    ser.dtr = True
    ser.rts = True
    time.sleep(0.05)
    # Assert reset (EN=LOW) while keeping GPIO0=HIGH
    ser.dtr = True   # GPIO0=HIGH
    ser.rts = False  # EN=LOW
    time.sleep(0.2)
    # Release reset
    ser.dtr = True
    ser.rts = True
    time.sleep(0.5)
    print("[RESET] Done.")


def reset_bootloader(ser):
    """Enter bootloader mode (for uploading)."""
    print("[RESET] Entering bootloader...")
    ser.dtr = True
    ser.rts = True
    time.sleep(0.05)
    # Assert reset
    ser.dtr = False  # GPIO0=LOW
    ser.rts = False  # EN=LOW
    time.sleep(0.2)
    # Release reset, keep BOOT low
    ser.dtr = False  # GPIO0=LOW
    ser.rts = True   # EN=HIGH
    time.sleep(0.5)
    print("[RESET] Bootloader mode.")


def main():
    parser = argparse.ArgumentParser(description="Reset and monitor ESP32-S3")
    parser.add_argument("--port", default=SERIAL_PORT, help="Serial port")
    parser.add_argument("--boot", action="store_true", help="Enter bootloader mode")
    parser.add_argument("--listen", action="store_true", help="Just listen, no reset")
    args = parser.parse_args()

    print(f"Opening {args.port} at {BAUDRATE}...")
    try:
        ser = serial.Serial(args.port, BAUDRATE, timeout=0.1)
    except serial.SerialException as e:
        print(f"Failed to open serial port: {e}")
        sys.exit(1)

    if not args.listen:
        if args.boot:
            reset_bootloader(ser)
        else:
            reset_normal(ser)
    else:
        print("[LISTEN] No reset, just monitoring...")

    print("Monitoring serial output (Press Ctrl+C to stop)...")
    print("-" * 50)

    last_line_time = time.time()
    try:
        while True:
            line = ser.readline().decode("utf-8", errors="ignore").strip()
            if line:
                print(line)
                last_line_time = time.time()
            elif time.time() - last_line_time > 5:
                print("[.] ...waiting for output...")
                last_line_time = time.time()
    except KeyboardInterrupt:
        print("\nStopped.")
    finally:
        ser.close()


if __name__ == "__main__":
    sys.exit(main())
