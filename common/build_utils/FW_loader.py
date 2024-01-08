import os
import sys
import time
import math
from typing import Dict, List
from argparse import ArgumentParser
from com_port_console import com_port_console as com

NLINE = com.NLINE

DEF_DATA = 0  # данные
DEF_FILE_END = 1  # маркер конца файла
DEF_SEG_ADDR = 2  # адрес сегмента
DEF_SEG_START_ADDR = 3  # сегментный адрес старта
DEF_LINE_ADDR = 4  # линейный адрес
DEF_LINE_START_ADDR = 5  # линейный адрес старта


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
    BL_CMD_SIZE = 4


class HexLine:
    def __init__(self, line: str):
        self.valid = False
        self.byte_num = str()
        self.offset = str()
        self.type_rec = DEF_DATA
        self.data_list: List[str] = list()

        crc_res = 0
        for i in range(1, len(line[1:-1]), 2):
            crc_res += int(line[i : i + 2], 16)

        if crc_res % 256 == 0:
            self.valid = True
            self.byte_num = line[1:3]
            self.offset = line[3:7]
            self.type_rec = int(line[7:9], 16)
            for i in range(int(self.byte_num, 16)):
                self.data_list.append(line[9 + 2 * i : 11 + 2 * i])


class HexFile:
    def __init__(self, fw_path: str):
        self.fw_data_list: List[HexLine] = list()
        self.load_start_addr = str()
        self.fw_entry_addr = str()

        hexf = open(fw_path, "r")
        lines = hexf.readlines()
        hexf.close()
        for i in range(len(lines)):
            n_obj = HexLine(lines[i])
            if n_obj.valid == True:
                if n_obj.type_rec == DEF_LINE_ADDR:
                    for i in range(len(n_obj.data_list)):
                        self.load_start_addr += n_obj.data_list[i]
                    self.load_start_addr += n_obj.offset

                elif n_obj.type_rec == DEF_LINE_START_ADDR:
                    for i in range(len(n_obj.data_list)):
                        self.fw_entry_addr += n_obj.data_list[i]

                elif n_obj.type_rec == DEF_DATA:
                    self.fw_data_list.append(n_obj)

                elif n_obj.type_rec == DEF_FILE_END:
                    end_found = True

                else:
                    raise Exception(f"Неиспользуемый тип строки {n_obj.type_rec}")
            else:
                raise Exception(f"HEX файл поврежден, line {i+1}")
        if end_found == False:
            raise Exception(f"HEX файл поврежден, конец файла не найден")

    def _fill_line_gaps(self, line_indx: int, add_bytes: int):
        self.fw_data_list[line_indx].byte_num = hex(
            int(self.fw_data_list[line_indx].byte_num, 16) + add_bytes
        ).replace("0x", "")
        for _ in range(add_bytes):
            self.fw_data_list[line_indx].data_list.append("FF")
        print(
            f"[filled] Addr: {hex(int(self.fw_data_list[line_indx].offset,16))} filled by {add_bytes} of 0xFF"
        )

    def fill_memory_gaps(self):
        line_indx = 0
        while line_indx < (len(self.fw_data_list) - 1):
            addr_that = int(self.fw_data_list[line_indx].offset, 16)
            addr_next = int(self.fw_data_list[line_indx + 1].offset, 16)
            byte_num = int(self.fw_data_list[line_indx].byte_num, 16)

            if 16 >= (addr_next - addr_that) > byte_num:
                add_bytes = (addr_next - addr_that) - byte_num
                self._fill_line_gaps(line_indx, add_bytes)

            elif (addr_next - addr_that) > 16:  # пропущено больше 16 байт
                raise Exception("эту часть нужно дописать")
                add_line_num = math.ceil((addr_next - addr_that) / 16) - 1
                for i in range(add_line_num):
                    fw_data_null = HexLine(
                        ":"
                        + "10"
                        + hex(addr_that + 16 * (i + 1))
                        + "00"
                        + "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF10"
                    )
                    (
                        self.fw_data_list[: line_indx + 1]
                        + [fw_data_null]
                        + self.fw_data_list[line_indx:]
                    )
                    print(f"[Added] Addr: {hex(addr_that+16*(i+1))} with 16 of 0xFF")
                    line_indx += 1

            line_indx += 1


class FWLoadUtiles:
    def __init__(self, interface: str):
        self.fw_frame_data: List[int] = []
        self.fw_frame_addr: List[str] = list()
        self.fw_frame_size: List[str] = list()
        self.console = 0

        self.init_com_interface(interface)

    def _append_fw_frame(self, frame_data: List[int]):
        self.fw_frame_data.append(frame_data)
        self.fw_frame_size.append(str(len(frame_data)))

    def init_fw_frames(self, file: HexFile, frame_size: int, sign_end_addr: int, en_info=False):
        frame: List[int] = []
        last_line_indx = len(file.fw_data_list) - 1
        for line_indx in range(len(file.fw_data_list)):
            line_bytes = int(file.fw_data_list[line_indx].byte_num, 16)
            line_offset = int(file.fw_data_list[line_indx].offset, 16)

            if len(frame) == 0:
                self.fw_frame_addr.append(
                    str(int(file.load_start_addr, 16) + line_offset)
                )

            if (len(frame) + line_bytes) <= frame_size:
                for byte_indx in range(line_bytes):
                    frame.append(
                        int(file.fw_data_list[line_indx].data_list[byte_indx], 16)
                    )
            else:
                self._append_fw_frame(frame)
                frame: List[int] = []

            if len(self.fw_frame_size)==0 and len(frame) >= sign_end_addr:
                #create first frame with signature at the end of frame
                self._append_fw_frame(frame)
                frame: List[int] = []

            if len(frame) >= frame_size:
                self._append_fw_frame(frame)
                frame: List[int] = []
            else:
                if line_indx == (last_line_indx):
                    self._append_fw_frame(frame)
                    frame: List[int] = []
                else:
                    line_offset_next = int(file.fw_data_list[line_indx + 1].offset, 16)
                    if (line_offset_next - line_offset) != line_bytes:
                        print(
                            f"{hex(line_offset_next)} - {hex(line_offset)} != {hex(line_bytes)}"
                        )
                        raise Exception("Gaps in HEX file. Need fill gaps")

        if en_info == True:
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
                raise Exception(
                    f"Invalid port {interface_settings}. Existing options: BLE and COM_PORT"
                )
            else:
                print(f"{interface_settings} connected")

        elif interface_name == "BLE":
            self.console = 1  # TODO

        else:
            raise Exception(
                f"Invalid interface:{interface}. Existing options: BLE and COM_PORT"
            )

    def flash_firmware(self) -> bool:
        if self.bl_switch() == True:
            if self.erase_flash() == True:
                for i in range(len(self.fw_frame_addr) - 1, -1, -1):
                    if self.set_flash_addr(self.fw_frame_addr[i]) == False:
                        break
                    if self.set_frame_size(self.fw_frame_size[i]) == False:
                        break
                    if self.send_fw_frame(self.fw_frame_data[i]) == False:
                        break
                    if self.flash_fw_frame() == False:
                        break
                    print(
                        f"addr: {hex(int(self.fw_frame_addr[i]))}, size: {int(self.fw_frame_size[i])} done"
                    )

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
        s = self.console.rx_line(cmd_len=BLCmd.BL_CMD_SIZE, rx_timeout=2)
        if s == BLCmd.BL_CMD_DONE_ST:
            print("Flash erased")
            return True
        else:
            print("Flash erase error")
            return False

    def set_flash_addr(self, addr: str):
        self.send_cmd(BLCmd.BL_CMD_SET_FLASH_ADDR + "=" + addr)
        s = self.console.rx_line()
        if s == addr:
            return True
        else:
            print("Set flash address error")
            return False

    def set_frame_size(self, size: str):
        self.send_cmd(BLCmd.BL_CMD_SET_FLASH_FRAME_SIZE + "=" + size)
        s = self.console.rx_line()
        if s == size:
            return True
        else:
            print("Set flash size error")
            return False

    def send_fw_frame(self, data_list: List[int]):
        self.send_cmd(BLCmd.BL_CMD_GET_AND_FLASH_FW_FRAME)
        s = self.console.rx_line(cmd_len=BLCmd.BL_CMD_SIZE)
        if s == BLCmd.BL_CMD_READY_ST:
            self.send_data(data_list)
            s = self.console.rx_line(2, BLCmd.BL_CMD_SIZE)
            if s == BLCmd.BL_CMD_DONE_ST:
                return True
            else:
                print("Send fw frame error")
                return False
        else:
            print("Send fw frame error")
            return False

    def flash_fw_frame(self):
        s = self.console.rx_line(2, BLCmd.BL_CMD_SIZE)
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
        help="Flash signature end address in HEX file",
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
        fw_frame_size = 1024 * 8
    else:
        fw_frame_size = args.frame_size

    if args.interface_var == None:
        interface = "COM_PORT=COM5"
    else:
        interface = args.interface_var
    
    if args.sign_end_addr == None:
        fw_sign_end_addr = 0x120
    else:
        fw_sign_end_addr = args.sign_end_addr

    fw_loader = FWLoadUtiles(interface)
    hex_file = HexFile(fw_path)
    hex_file.fill_memory_gaps()

    fw_loader.init_fw_frames(hex_file, fw_frame_size, fw_sign_end_addr, en_info=True)
    fw_loader.flash_firmware()
