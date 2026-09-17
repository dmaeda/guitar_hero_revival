# Raspberry Pi UART Test

This repository now includes a simple UART loopback test utility for Raspberry Pi:

`files/utilities/rpi_uart_test.py`

## What it does

The script opens a UART device, configures the selected baud rate, sends a short message, and checks whether the exact same bytes are received back.

This is intended for a basic **loopback test**, where:

- **TX** is connected to **RX**
- **GND** is connected to **GND**

## Raspberry Pi setup

Enable the serial port on the Raspberry Pi before running the test:

1. Run `sudo raspi-config`
2. Open `Interface Options`
3. Open `Serial Port`
4. Disable the login shell over serial
5. Enable the serial hardware
6. Reboot if prompted

A common UART device path on Raspberry Pi is `/dev/serial0`, but the script does not auto-detect it: pass the device path explicitly.

## Run the test

From the repository root:

```bash
python3 files/utilities/rpi_uart_test.py /dev/serial0
```

Example with a different baud rate and message:

```bash
python3 files/utilities/rpi_uart_test.py /dev/serial0 --baud 9600 --message HELLO_UART
```

`--message` must be a non-empty string.

## Expected result

If the UART loopback wiring and port configuration are correct, each attempt should print `PASS`.

If the test fails, verify:

- UART is enabled on the Raspberry Pi
- TX and RX are crossed correctly
- Ground is shared
- The selected UART device is correct
- The baud rate matches the expected configuration
