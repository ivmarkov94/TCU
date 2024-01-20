import serial
import time
from typing import Dict, List

NLINE = "\r\n"


class ComPortConsole:
    def __init__(self, port="", baud=115200) -> None:
        self.baudrate = int(baud)
        self.port = port
        self.timeout = 0.05
        self.serialDut = None
        self.initialised = False
        pass

    def setup(self):
        try:
            self.serialDut = serial.Serial(port=self.port, baudrate=self.baudrate, timeout=self.timeout)
        except serial.serialutil.SerialException:
            print(f"Could not open the port {self.port}")
            return False
        self.initialised = True

        return True

    def close(self):
        self.serialDut.close()

    def rx_line(self, rx_timeout=0.4, expect_reply=None, end=NLINE):
        if self.initialised is True:
            if expect_reply != None:
                rx_data_size = len(expect_reply)
                rx_data_size += len(end)
                if rx_timeout < 2:
                    rx_timeout = 2
            self.serialDut.timeout = rx_timeout
            return self.serialDut.readline(rx_data_size).decode(errors="ignore").replace(end, "")

    def rx_lines(self, exp_str="", rx_timeout=0.3) -> str:
        if self.initialised is True:
            if exp_str != "":
                str_len = len(exp_str + NLINE)
                rx_timeout = 0.5
            else:
                str_len = 1000
            self.serialDut.timeout = rx_timeout
            return self.serialDut.read(str_len).decode(errors="ignore")

    def tx_line(self, cmd, end=NLINE):
        if self.initialised is True:
            cmd += end
            self.serialDut.reset_input_buffer()
            self.serialDut.write(str.encode(cmd))

    def tx_data(self, date_list: List[int]):
        if self.initialised is True:
            self.serialDut.reset_input_buffer()
            self.serialDut.write(bytes(date_list))


if __name__ == "__main__":
    com = ComPortConsole(port="COM5")
    com.setup()
    com.tx_line("h")
    print(com.rx_line())
    com.close()
