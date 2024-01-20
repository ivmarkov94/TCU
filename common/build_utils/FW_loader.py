import os
import sys
import time
import math
from typing import Dict, List
from argparse import ArgumentParser
from com_port_console import com_port_console as com

NLINE = com.NLINE
SKIP = 0


class LineType:
    DATA = 0  # данные
    FILE_END = 1  # маркер конца файла
    SEG_ADDR = 2  # адрес сегмента
    SEG_START_ADDR = 3  # сегментный адрес старта
    LINE_ADDR = 4  # линейный адрес
    LINE_START_ADDR = 5  # линейный адрес старта


class BLCmd:
    BL_CMD_FW_ST = "_fws"
    BL_CMD_GO_TO_BL = "_gbl"
    BL_CMD_SET_FLASH_ADDR = "_sfa"
    BL_CMD_SET_FLASH_FRAME_SIZE = "_sff"
    BL_CMD_ERASE_FLASH = "_efl"
    BL_CMD_GET_AND_FLASH_FW_FRAME = "_gfw"
    BL_CMD_READY_ST = "_rds"
    BL_CMD_DONE_ST = "_dns"
    BL_CMD_ERROR_ST = "_ers"
    BL_CMD_CRC_ERR_ST = "_crc"
    BL_CMD_FORMAT = "_lll"


class HexFile:
    def __init__(self, fw_path: str, dbg_info=False):
        self.lines_data: List[List[str]] = []
        self.lines_size: List[int] = []
        self.lines_offset: List[int] = []
        self.fw_start_addr = int()
        self.fw_entry_addr = int()
        self.dbg_info = dbg_info

        hexf = open(fw_path, "r")
        hex_lines = hexf.readlines()
        hexf.close()
        self._lines_parsing(hex_lines)

    def _lines_parsing(self, lines: List[str]) -> bool:
        for str_line in lines:
            crc_res = sum([int(str_line[i : i + 2], 16) for i in range(1, len(str_line[1:]), 2)])
            if crc_res % 256 == 0:
                size = int(str_line[1:3], 16)
                offset = int(str_line[3:7], 16)
                type = int(str_line[7:9], 16)
                data = [str_line[9 + 2 * i : 11 + 2 * i] for i in range(size)]
                if type == LineType.LINE_ADDR:
                    self.fw_start_addr = int("".join(data) + str("0000"), 16)
                elif type == LineType.LINE_START_ADDR:
                    self.fw_entry_addr = "".join(data)
                elif type == LineType.DATA:
                    self.lines_data.append(data)
                    self.lines_size.append(size)
                    self.lines_offset.append(offset)
                elif type == LineType.FILE_END:
                    end_found = True
                else:
                    raise Exception(f"Неиспользуемый тип строки {type}")
            else:
                raise Exception(f"HEX файл поврежден, CRC error line:{i+1}")
        if end_found == False:
            raise Exception(f"HEX файл поврежден, конец файла не найден")

    def _fill_line_gaps(self, line_indx: int, num_bytes: int):
        self.lines_size[line_indx] += num_bytes
        self.lines_data[line_indx] += "FF" * num_bytes
        if self.dbg_info == True:
            print(f"[gap filled] addr: {hex(self.lines_size[line_indx])} filled by {num_bytes} of 0xFF")

    def fill_memory_gaps(self):
        line_indx = 0
        while line_indx < (len(self.lines_data) - 1):
            addr_that = self.lines_offset[line_indx]
            addr_next = self.lines_offset[line_indx + 1]
            line_size = self.lines_size[line_indx]

            if 16 >= (addr_next - addr_that) > line_size:
                add_bytes = (addr_next - addr_that) - line_size
                self._fill_line_gaps(line_indx, add_bytes)

            elif (addr_next - addr_that) > 16:  # пропущено больше 16 байт
                raise Exception("эту часть нужно дописать")
            line_indx += 1


class FWLoadUtiles:
    def __init__(self, interface: str, dbg_info=False):
        self.fw_frame_data: List[List[int]] = []
        self.fw_frame_addr: List[str] = list()
        self.fw_frame_size: List[str] = list()
        self.console = 0
        self.dbg_info = dbg_info

        self.init_com_interface(interface)

    def _append_fw_frame(self, frame_data: List[int]):
        self.fw_frame_data.append(frame_data)
        self.fw_frame_size.append(str(len(frame_data)))

    def init_fw_frames(self, file: HexFile, frame_max_size: int, sign_end_addr: int):
        frame_data: List[int] = []
        last_line_indx = len(file.lines_data) - 1
        for line_indx in range(len(file.lines_data)):
            line_size = file.lines_size[line_indx]

            if (len(frame_data) + line_size) > frame_max_size:
                # to confine max frame size
                self._append_fw_frame(frame_data)
                frame_data = []

            if len(self.fw_frame_size) == 0 and len(frame_data) >= sign_end_addr:
                # create first frame with signature at the end of frame
                self._append_fw_frame(frame_data)
                frame_data = []

            if len(frame_data) == 0:
                self.fw_frame_addr.append(str(file.fw_start_addr + file.lines_offset[line_indx]))
                if len(self.fw_frame_addr) == 1 and int(hex(int(self.fw_frame_addr[0]))[-4:]) <= 0:
                    raise Exception(
                        f"Uncorrect start address: {hex(int(self.fw_frame_addr[0]))}.\nLook like application without bootloader"
                    )
            frame_data += [int(file.lines_data[line_indx][byte_indx], 16) for byte_indx in range(line_size)]

            if len(frame_data) >= frame_max_size:
                self._append_fw_frame(frame_data)
                frame_data = []
            else:
                if line_indx == (last_line_indx):
                    # creale last fw_frame
                    self._append_fw_frame(frame_data)
                    frame_data = []
                else:  # check gaps between lines addresses
                    line_offset = file.lines_offset[line_indx]
                    line_offset_next = file.lines_offset[line_indx + 1]
                    if (line_offset_next - line_offset) != line_size:
                        print(f"{hex(line_offset_next)} - {hex(line_offset)} != {hex(line_size)}")
                        raise Exception("Gaps in HEX file. Need fill gaps")

        if self.dbg_info == True:
            for i in range(len(self.fw_frame_addr)):
                print(
                    "Frame start address:",
                    hex(int(self.fw_frame_addr[i])),
                    " Frame size:",
                    self.fw_frame_size[i],
                )

    def init_com_interface(self, interface: str):
        interface_name = interface[: interface.find("=")]
        interface_settings = interface[interface.find("=") + 1 :]
        if interface_name == "COM_PORT":
            self.console = com.ComPortConsole(port=interface_settings)
            if self.console.setup() == False:
                raise Exception(f"Invalid port {interface_settings}. Existing options: BLE and COM_PORT")
            else:
                print(f"{interface_settings} connected")

        elif interface_name == "BLE":
            self.console = 1  # TODO

        else:
            raise Exception(f"Invalid interface:{interface}. Existing options: BLE and COM_PORT")

    def flash_firmware(self) -> bool:
        if self.bl_switch() == True:
            if self.erase_flash() == True:
                start_time = time.time()
                for i in range(len(self.fw_frame_addr)):
                    indx = len(self.fw_frame_addr) - 1 - i
                    # start_oper_time = time.time()
                    if self.set_flash_addr(self.fw_frame_addr[indx]) == False:
                        break
                    # print(f"set_flash_addr: {(time.time() - start_oper_time):.1f}sec")
                    # start_oper_time = time.time()
                    if self.set_frame_size(self.fw_frame_size[indx]) == False:
                        break
                    # print(f"set_frame_size: {(time.time() - start_oper_time):.1f}sec")
                    # start_oper_time = time.time()
                    if self.send_fw_frame(self.fw_frame_data[indx]) == False:
                        break
                    # print(f"send_fw_frame: {(time.time() - start_oper_time):.1f}sec")
                    # start_oper_time = time.time()
                    if self.flash_fw_frame() == False:
                        break
                    # print(f"flash_fw_frame: {(time.time() - start_oper_time):.1f}sec")
                    print(
                        f"addr: {hex(int(self.fw_frame_addr[indx]))}, size: {int(self.fw_frame_size[indx])} {i+1}/{len(self.fw_frame_addr)} done"
                    )
                print(f"Flash time: {(time.time() - start_time):.1f}sec")

    def bl_switch(self, timeout_s=0.5):
        result = False

        self.send_cmd(BLCmd.BL_CMD_FW_ST, crc=False)
        if self.console.rx_line() == "BL":
            result = True
        else:
            self.send_cmd(BLCmd.BL_CMD_GO_TO_BL, crc=False)
            time.sleep(timeout_s)
            self.send_cmd(BLCmd.BL_CMD_FW_ST, crc=False)
            if self.console.rx_line() == "BL":
                result = True

        if result == True:
            print("Target is in BL mode")
        else:
            print("BL mode swithing error")
        return result

    def erase_flash(self):
        self.send_cmd(BLCmd.BL_CMD_ERASE_FLASH)
        s = self.console.rx_line()
        if s == BLCmd.BL_CMD_DONE_ST:
            print("[Flash erase] Process success")
            return True
        else:
            print("{Flash erase] Process error, Answer was not got.")
            return False

    def set_flash_addr(self, addr: str):
        self.send_cmd(BLCmd.BL_CMD_SET_FLASH_ADDR + "=" + addr)
        s = self.console.rx_line()
        if s == addr:
            return True
        elif s == BLCmd.BL_CMD_ERROR_ST:
            print(f"[Set flash address] Address out of range. Val={hex(int(addr))}")
            return False
        else:
            print(f"[Set flash address] Process error, Answer was not got.")
            return False

    def set_frame_size(self, size: str):
        self.send_cmd(BLCmd.BL_CMD_SET_FLASH_FRAME_SIZE + "=" + size)
        s = self.console.rx_line()
        if s == size:
            return True
        elif s == BLCmd.BL_CMD_ERROR_ST:
            print(f"[Set flash size] size out of range. Val={size}")
            return False
        else:
            print(f"[Set flash size] Process error, Answer was not got.")
            return False

    def send_fw_frame(self, data_list: List[int]):
        self.send_cmd(BLCmd.BL_CMD_GET_AND_FLASH_FW_FRAME)
        s = self.console.rx_line()
        if s == BLCmd.BL_CMD_READY_ST:
            self.send_data(data_list)
            s = self.console.rx_line(10)
            if s == BLCmd.BL_CMD_DONE_ST:
                return True
            elif s == BLCmd.BL_CMD_CRC_ERR_ST:
                print("[Send fw frame] CRC error during sending")
                return False
            elif s == BLCmd.BL_CMD_ERROR_ST:
                print("[Send fw frame] Send error. Timeout between byte frames > 270ms or data was lost")
                return False
            else:
                print("[Send fw frame] Send error. Timeout rx_line()")
        else:
            print("[Send fw frame] Ready status was not get")
            return False

    def flash_fw_frame(self):
        s = self.console.rx_line()
        if s == BLCmd.BL_CMD_DONE_ST:
            return True
        else:
            print("flash fw frame error")
            return False

    def send_cmd(self, cmd: str, crc=True, end=NLINE):
        data_list: List[int] = []
        for s in cmd:
            data_list.append(ord(s))
        if crc:
            data_list.append(self.get_cmd_crc(cmd))
        for s in end:
            data_list.append(ord(s))
        self.console.tx_data(data_list)

    def send_data(self, data_list: List[int], crc=True):
        if crc:
            data_list.append(self.get_data_crc(data_list))
        self.console.tx_data(data_list)

    def get_cmd_crc(self, cmd: str) -> int:
        crc = 0
        for s in cmd:
            crc += ord(s)
        if crc % 256 != 0:
            crc = 256 - crc % 256
        else:
            crc = 0
        return crc

    def get_data_crc(self, data_list: List[int]) -> int:
        crc = 0
        for i in range(len(data_list)):
            crc += data_list[i]
        if crc % 256 != 0:
            crc = 256 - crc % 256
        else:
            crc = 0
        return crc


if __name__ == "__main__":
    parser = ArgumentParser(description="FW loader. BLE or UART interface")
    parser.add_argument(
        "-fwp",
        "--fw_path",
        action="store",
        help="FW full path with name. Example C:\\fw\\target.hex",
        required=False,
    )
    parser.add_argument(
        "-fsz",
        "--frame_size",
        action="store",
        help="Frame size for flashing",
        required=False,
    )

    parser.add_argument(
        "-iv",
        "--interface_var",
        action="store",
        help="There are BLE and COM_PORT=COM5",
        required=False,
    )
    parser.add_argument(
        "-sgea",
        "--sign_end_addr",
        action="store",
        help="Flash signature end address in HEX file. Example 0x120",
        required=False,
    )

    args = parser.parse_args()

    if args.fw_path == None:
        file_folder_path = os.path.dirname(os.path.realpath(__file__))
        files = os.listdir(file_folder_path)
        for i in range(len(files)):
            if files[i].endswith(".hex") == True:
                file_name = files[i]
                break
        fw_path = file_folder_path + f"\\{file_name}"
    else:
        fw_path = args.fw_path

    if args.frame_size == None:
        fw_frame_size = 1024
    else:
        fw_frame_size = int(args.frame_size)
    if args.interface_var == None:
        interface = "COM_PORT=COM6"
    else:
        interface = args.interface_var

    if args.sign_end_addr == None:
        fw_sign_end_addr = 0x120
    else:
        fw_sign_end_addr = int(args.sign_end_addr, 16)

    fw_loader = FWLoadUtiles(interface, dbg_info=True)
    hex_file = HexFile(fw_path, dbg_info=True)
    hex_file.fill_memory_gaps()

    fw_loader.init_fw_frames(hex_file, fw_frame_size, fw_sign_end_addr)
    fw_loader.flash_firmware()
