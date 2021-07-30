import serial
import time

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

_DEBUG = True


def bytes_to_hex_str(data: bytes):
    hex_bytes = list(map(lambda b: '{:02X}'.format(b), data[3:]))
    return ' '.join(hex_bytes)


class DataFrame:
    def __init__(self, data: bytes):
        self.type = data[0]
        self.cmd = data[1]
        payload_size = (data[2] << 8) + data[3]
        self.payload = data[4: 4 + payload_size]


class ChecksumError(BaseException):
    def __init__(self, frame_data, expected_checksum, trimmed_data):
        self.trimmed_data = trimmed_data
        self.expected_checksum = expected_checksum
        self.data = frame_data
        if _DEBUG: print(f"CHECKSUM FRAME {bytes_to_hex_str(frame_data)}")

    def __str__(self):
        return f"Invalid data frame (checksum error, expected {hex(self.expected_checksum)})."


class M5:
    # cmd, payload
    CMD_HARDWARE_VERSION = (0x3, b'\x00')
    CMD_SOFTWARE_VERSION = (0x3, b'\x01')
    CMD_MANUFACTURER_NAME = (0x3, b'\x02')
    CMD_SINGLE_POLL = (0x22, bytes())

    def __init__(self, stream):
        self.__stream = stream
        self.__data = bytes()
        self.__stream.reset_input_buffer()
        self.__timeout = 5.0

    def __reset(self):
        self.__data = bytes()
        self.__stream.reset_input_buffer()

    @staticmethod
    def get_response_frame(data: bytes):
        # TODO: only return for expected command response?
        try:
            prefix_index = data.index(0xBB)
        except ValueError:
            return None, data

        frame = None
        trimmed_data = data[prefix_index:]

        if len(data) > prefix_index + 4:
            payload_size = data[prefix_index + 3] << 8
            payload_size += data[prefix_index + 4]
            # header data + payload + checksum + trailer
            trailer_index = prefix_index + 4 + payload_size + 2

            if len(data) > trailer_index and data[trailer_index] == 0x7E:
                frame_data = data[prefix_index + 1:trailer_index]
                trimmed_data = data[trailer_index + 1:]

                expected_checksum = M5.checksum(frame_data[:-1])
                if expected_checksum != frame_data[-1]:
                    raise ChecksumError(frame_data, expected_checksum, trimmed_data)
                if _DEBUG: print(f"RESPONSE FRAME {bytes_to_hex_str(data[prefix_index + 1:trailer_index])}")
                frame = DataFrame(data[prefix_index + 1:trailer_index - 1])
            elif len(data) > trailer_index:
                # Apparently this frame is not valid, find next start
                frame, trimmed_data = M5.get_response_frame(data[prefix_index + 1:])

        return frame, trimmed_data

    def __get_response(self):
        end_time = time.time() + self.__timeout

        while end_time > time.time():
            self.__data += self.__stream.read()
            frame, trimmed_data = self.get_response_frame(self.__data)
            self.__data = trimmed_data
            if frame is not None:
                return frame

        return None

    @staticmethod
    def checksum(data: bytes):
        # Data should be everything from 'Type' to end of payload
        return sum(data) & 0xFF

    def send_command(self, cmd: bytes, payload: bytes, get_response=True):
        full_cmd = b'\xBB'
        full_cmd += b'\x00'  # Type command
        full_cmd += bytes([cmd])
        full_cmd += bytes([len(payload) >> 8])
        full_cmd += bytes([len(payload) & 0xFF])
        full_cmd += payload
        full_cmd += bytes([self.checksum(full_cmd[1:])])
        full_cmd += b'\x7E'

        if _DEBUG: print(f"CMD: {bytes_to_hex_str(full_cmd)}")
        self.__stream.write(full_cmd)
        if get_response:
            return self.__get_response()

        return None

    def print_info(self):
        self.__reset()
        info = self.send_command(self.CMD_HARDWARE_VERSION[0], self.CMD_HARDWARE_VERSION[1])
        print(f"Hardware version: {info.payload[1:].decode()}")

        info = self.send_command(self.CMD_SOFTWARE_VERSION[0], self.CMD_SOFTWARE_VERSION[1])
        print(f"Software version: {info.payload[1:].decode()}")

        info = self.send_command(self.CMD_MANUFACTURER_NAME[0], self.CMD_MANUFACTURER_NAME[1])
        print(f"Manufacturer: {info.payload[1:].decode()}")

    def read_tag(self):
        response = self.send_command(self.CMD_SINGLE_POLL[0], self.CMD_SINGLE_POLL[1])
        print(f"SINGLE POLL RESPONSE: {bytes_to_hex_str(response.payload)}")
        if len(response.payload) == 17:
            print(f"RSSI: {response.payload[0]}")
            print(f"PC  : {hex((response.payload[1] << 8) + response.payload[2])}")
            print(f"EPC : {bytes_to_hex_str(response.payload[3:])}")


if __name__ == "__main__":
    ser = serial.Serial('COM6', 115200, timeout=0.25)

    m5 = M5(ser)
    m5.print_info()
    try:
        while True:
            m5.read_tag()
    except KeyboardInterrupt:
        pass

    ser.close()
