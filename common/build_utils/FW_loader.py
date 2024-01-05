import os
import sys
import time
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
    FW_ST = "fw_st?"
    GO_TO_BL = "go_to_bl"


class HexLine:
    def __init__(self, line: str):
        self.valid = False
        self.byte_num = str()
        self.offset = str()
        self.type_rec = DEF_DATA
        self.data_list = list()

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
        self.fw_data_list = list()
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


class FWLoadUtiles:
    def __init__(self, hex_file: HexFile, interface: str):
        self.hex_file = hex_file
        self.console = 0

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

        if self.bl_switch() == True:
            print("Target in BL mode")
        else:
            raise Exception("BL was not found")

    def bl_switch(self, timeout_s=0.5):
        self.console.tx_line(BLCmd.FW_ST)
        s = self.console.rx_lines()
        if s == "BL" + NLINE:
            return True
        else:
            self.console.tx_line(BLCmd.GO_TO_BL)
            time.sleep(timeout_s)
            self.console.tx_line(BLCmd.FW_ST)
            s = self.console.rx_lines()
            if s == "BL" + NLINE:
                return True
        return False


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

    args = parser.parse_args()

    if args.fw_path == None:
        file_folder_path = os.path.dirname(os.path.realpath(__file__))
        files = os.listdir(file_folder_path)
        for i in range(len(files)):
            if files[i].endswith(".hex") == True:
                file_name = files[i]
                break
        fw_path = file_folder_path + f"\\{file_name}"

    if args.frame_size == None:
        fw_frame_size = 1024

    if args.interface_var == None:
        interface = "COM_PORT=COM5"

    print(sys.path)
    hex_file = HexFile(fw_path)
    fw_loader = FWLoadUtiles(hex_file, interface)
