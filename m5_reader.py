import serial
import time
import struct
import crc8

"""
Command/response structure:
BB 00 03 00 01 00 04 7E
-- -- -- -- -- -- -- --
|  |  |  |  |  |  |  + Trailer
|  |  |  |  |  |  + -- Checksum
|  |  |  |  |  + -- Payload/parameters
|  |  |  |  + -- Payload size LSB
|  |  |  + -- Payload size MSB
|  |  +-- Command
|  +-- Type
+-- Header
"""

class M5:
    # cmd, payload
    CMD_HARDWARE_VERSION= (0x3, b'\x00')
    CMD_SOFTWARE_VERSION= (0x3, b'\x01')

    def __init__(self, stream):
        self.__stream = stream
        self.__data = bytes()
        self.__stream.reset_input_buffer()
        self.__timeout = 1.0

    def __reset(self):
        self.__data = bytes()
        self.__stream.reset_input_buffer()

    def __get_response(self):
        end_time = time.time() + self.__timeout

        while end_time > time.time():
            self.__data += self.__stream.read()

            try:
                # Naive approach, 0x7E might also be in the payload (as could be 0xBB)
                prefix_index = self.__data.index(0xBB)
                suffix_index = self.__data.index(0x7E)
                frame_data = self.__data[prefix_index:suffix_index]
                self.__data = self.__data[suffix_index + 1:]
                return frame_data
            except IndexError:
                pass

        return None

    @staticmethod
    def checksum(data: bytes):
        return sum(data)

    def send_command(self, cmd: bytes, payload: bytes):
        full_cmd = b'\xBB'
        full_cmd += 0x00  # Type command
        full_cmd += cmd
        full_cmd += len(payload) >> 8
        full_cmd += len(payload) & 0xFF
        full_cmd += payload
        full_cmd += checksum(full_cmd[1:])
        full_cmd += checksum(full_cmd[1:])
        full_cmd += 0x7E
        print(full_cmd)
        self.__stream.write(full_cmd)
        return self.__get_response()

    def print_info(self):
        self.__reset()
        hw_info = self.send_command(CMD_HARDWARE_VERSION[0], CMD_HARDWARE_VERSION[1])



        ser.write(bytes([0xBB, 0x00, 0x22, 0x00, 0x00, 0x22, 0x7E]))
        print(f"Single polling: {get_response()}")

        ser.write(bytes([0xBB, 0x00, 0x39, 0x00, 0x09, 0x00, 0x00, 0xFF, 0xFF, 0x03, 0x00, 0x00, 0x00, 0x02, 0x45, 0x7E]))
        print(f"Label storage area: {get_response()}")
        time.sleep(1)


    def read_tag(self):

def build_command(cmd, address=255, payload=bytes()):
    def compute_crc(data):
        crc_value = 0xFFFF

        for ucI in range(1, len(data)): # Skip Len byte
            crc_value ^= data[ucI]
            for ucJ in range(0, 8):
                crc_value >>= 1
                if crc_value & 1:
                    crc_value ^= 0x8408

        return crc_value & 0xFFFF

    num_bytes = len(payload) + 4
    data = struct.pack("BBB", num_bytes, address, cmd)
    data += payload
    data += struct.pack("!H", compute_crc(data))
    print(data)
    return data

ser = serial.Serial('COM6', 115200, timeout=2)  # open serial port

def get_response():
    read_bytes = bytes()
    read_byte = ser.read()
    while read_byte != b'\x7E':
        read_bytes += read_byte
        read_byte = ser.read()

    return read_bytes

ser.write(bytes([0xBB, 0x00, 0x03, 0x00, 0x01, 0x00, 0x04, 0x7E]))
print(f"Hardware version: {get_response()}")
ser.write(bytes([0xBB, 0x00, 0x03, 0x00, 0x01, 0x01, 0x05, 0x7E]))
print(f"Software version: {get_response()}")

while True:
    try:
        ser.write(bytes([0xBB, 0x00, 0x22, 0x00, 0x00, 0x22, 0x7E]))
        print(f"Single polling: {get_response()}")

        ser.write(bytes([0xBB, 0x00, 0x39, 0x00, 0x09, 0x00, 0x00, 0xFF, 0xFF, 0x03, 0x00, 0x00, 0x00, 0x02, 0x45, 0x7E]))
        print(f"Label storage area: {get_response()}")
        time.sleep(1)
    except KeyboardInterrupt:
        break

"""
#cmd_data = build_command(0x21)
#for i in range(0, 10):
#    ser.write(cmd_data)
#    time.sleep(0.1)

cmd_data = build_command(0x21)
for i in range(0, 255):
    ser.write(bytes([i]))
    time.sleep(0.02)

#print(ser.name)         # check which port was really used
#ser.write(b'hello')     # write a string
"""
ser.close()
if __name == "__main__":
hash = crc8.crc8()
hash.update(b'123')
assert hash.hexdigest() == 'c0'