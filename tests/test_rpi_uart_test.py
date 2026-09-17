import os
import pty
import subprocess
import threading
import unittest
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
SCRIPT = REPO_ROOT / "files" / "utilities" / "rpi_uart_test.py"


class RpiUartTestScriptTest(unittest.TestCase):
    def run_script(self, with_echo: bool) -> subprocess.CompletedProcess[str]:
        master_fd, slave_fd = pty.openpty()
        slave_name = os.ttyname(slave_fd)

        if with_echo:
            def echo_loop() -> None:
                while True:
                    try:
                        data = os.read(master_fd, 1024)
                    except OSError:
                        return
                    if not data:
                        return
                    try:
                        os.write(master_fd, data)
                    except OSError:
                        return

            threading.Thread(target=echo_loop, daemon=True).start()

        try:
            return subprocess.run(
                [
                    "python3",
                    str(SCRIPT),
                    slave_name,
                    "--attempts",
                    "1",
                    "--timeout",
                    "0.25",
                    "--message",
                    "UART_TEST",
                ],
                cwd=REPO_ROOT,
                capture_output=True,
                text=True,
                check=False,
            )
        finally:
            os.close(slave_fd)
            os.close(master_fd)

    def test_loopback_pass_returns_zero(self) -> None:
        result = self.run_script(with_echo=True)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn("Attempt 1: PASS", result.stdout)

    def test_loopback_fail_returns_one(self) -> None:
        result = self.run_script(with_echo=False)
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn("Attempt 1: FAIL", result.stdout)


if __name__ == "__main__":
    unittest.main()
