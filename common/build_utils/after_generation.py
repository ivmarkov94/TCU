import shutil
import os

current_file = os.path.realpath(__file__)
current_directory = os.path.dirname(current_file)

apl_path = current_directory + "/../../application/"
bl_path  = current_directory + "/../../bootloader/"
path = [apl_path, bl_path]
for i in range(2):
    try:
        local_path = path[i] + "Drivers"
        shutil.rmtree(local_path)
        print(local_path, "delited")
    except FileNotFoundError:
        print("")
    try:
        local_path = path[i] +"startup_stm32f103xb.s.ld"
        os.remove(local_path)
        print(local_path, "delited")
    except FileNotFoundError:
        print("")
    try:
        local_path = path[i] +"STM32F103C8Tx_FLASH.ld"
        os.remove(local_path)
        print(local_path, "delited")
    except FileNotFoundError:
        print("")
    try:
        local_path = path[i] +"startup_stm32f103xb.s"
        os.remove(local_path)
        print(local_path, "delited")
    except FileNotFoundError:
        print("")
    try:
        local_path = path[i] +"Core/Src/system_stm32f1xx.c"
        os.remove(local_path)
        print(local_path, "cleaned")
    except FileNotFoundError:
        print("")

# try:
#     shutil.move(apl_path, current_directory + "/../")
# except FileNotFoundError:
#     print("")

# try:
#     shutil.move(apl_path, current_directory + "/../")
# except FileNotFoundError:
#     print("")


# shutil.copy('C:\\1_HobbyData\\BMW\\Thermostat_Control_Unit\\SW\\TCU\\bootloader\\Drivers', 'C:\\1_HobbyData\\BMW\\Thermostat_Control_Unit\\SW\\TCU\\common\\')