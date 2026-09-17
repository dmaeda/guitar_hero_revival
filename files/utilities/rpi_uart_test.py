#!/usr/bin/env python3

import argparse
import os
import select
import sys
import termios
import time


BAUD_RATES = {
    9600: termios.B9600,
    19200: termios.B19200,
    38400: termios.B38400,
    57600: termios.B57600,
    115200: termios.B115200,
}


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Run a simple UART loopback test on a Raspberry Pi serial port."
    )
    parser.add_argument("device", help="UART device path, for example /dev/serial0")
    parser.add_argument(
        "--baud",
        type=int,
        default=115200,
        choices=sorted(BAUD_RATES),
        help="Baud rate to use (default: 115200)",
    )
    parser.add_argument(
        "--message",
        default="GUITAR_HERO_UART_TEST",
        help="Message to transmit during the test",
    )
    parser.add_argument(
        "--attempts",
        type=int,
        default=3,
        help="Number of loopback attempts to run (default: 3)",
    )
    parser.add_argument(
        "--timeout",
        type=float,
        default=2.0,
        help="Timeout in seconds for each loopback attempt (default: 2.0)",
    )
    return parser.parse_args()


def configure_port(fd: int, baud_rate: int) -> None:
    attrs = termios.tcgetattr(fd)

    attrs[0] = 0
    attrs[1] = 0
    attrs[2] &= ~(termios.CSIZE | termios.PARENB | termios.CSTOPB)
    attrs[2] |= termios.CS8 | termios.CLOCAL | termios.CREAD
    if hasattr(termios, "CRTSCTS"):
        attrs[2] &= ~termios.CRTSCTS
    attrs[3] = 0
    attrs[4] = BAUD_RATES[baud_rate]
    attrs[5] = BAUD_RATES[baud_rate]
    attrs[6][termios.VMIN] = 0
    attrs[6][termios.VTIME] = 0

    termios.tcflush(fd, termios.TCIOFLUSH)
    termios.tcsetattr(fd, termios.TCSANOW, attrs)


def read_exact(fd: int, size: int, timeout: float) -> bytes:
    deadline = time.monotonic() + timeout
    data = bytearray()

    while len(data) < size:
        remaining = deadline - time.monotonic()
        if remaining <= 0:
            break

        readable, _, _ = select.select([fd], [], [], remaining)
        if not readable:
            break

        chunk = os.read(fd, size - len(data))
        if not chunk:
            continue
        data.extend(chunk)

    return bytes(data)


def write_all(fd: int, payload: bytes, timeout: float) -> int:
    deadline = time.monotonic() + timeout
    total_written = 0

    while total_written < len(payload):
        remaining = deadline - time.monotonic()
        if remaining <= 0:
            break

        _, writable, _ = select.select([], [fd], [], remaining)
        if not writable:
            break

        total_written += os.write(fd, payload[total_written:])

    return total_written


def run_loopback(fd: int, payload: bytes, attempts: int, timeout: float) -> bool:
    success = True

    for attempt in range(1, attempts + 1):
        termios.tcflush(fd, termios.TCIOFLUSH)
        written = write_all(fd, payload, timeout)
        received = read_exact(fd, len(payload), timeout)

        if written != len(payload) or received != payload:
            success = False
            print(f"Attempt {attempt}: FAIL")
            print(f"  Sent:     {payload!r}")
            print(f"  Received: {received!r}")
        else:
            print(f"Attempt {attempt}: PASS")

    return success


def main() -> int:
    args = parse_args()

    if args.attempts < 1:
        print("--attempts must be at least 1", file=sys.stderr)
        return 2

    payload = args.message.encode("utf-8")
    if not payload:
        print("--message must not be empty", file=sys.stderr)
        return 2

    try:
        fd = os.open(args.device, os.O_RDWR | os.O_NOCTTY | os.O_NONBLOCK)
    except OSError as exc:
        print(f"Unable to open {args.device}: {exc}", file=sys.stderr)
        return 1

    try:
        configure_port(fd, args.baud)
        print(f"Testing {args.device} at {args.baud} baud")
        print("Tip: connect UART TX to RX for a simple loopback test.")
        return 0 if run_loopback(fd, payload, args.attempts, args.timeout) else 1
    except OSError as exc:
        print(f"UART test failed: {exc}", file=sys.stderr)
        return 1
    finally:
        os.close(fd)


if __name__ == "__main__":
    raise SystemExit(main())
