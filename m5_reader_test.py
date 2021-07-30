import unittest
import m5_reader


class M5Tester(unittest.TestCase):
    def test_frame_extraction(self):
        self.assertEqual((None, b'123'),
            m5_reader.M5.get_response_frame(b'123'))

        # Too short
        self.assertEqual((None, b'\xBB12'),
            m5_reader.M5.get_response_frame(b'\xBB12'))

        self.assertEqual((None, b'\xBB\x00\x03\x00\x01'),
            m5_reader.M5.get_response_frame(b'\xBB\x00\x03\x00\x01'))

        self.assertEqual((None, b'\xBB\x00\x03\x00\x01\xA5\xFF'),
            m5_reader.M5.get_response_frame(b'\xBB\x00\x03\x00\x01\xA5\xFF'))

        # Leading data is trimmed
        self.assertEqual((None, b'\xBB\x00\x03\x00\x01\xA5\xFF'),
            m5_reader.M5.get_response_frame(b'1234\xBB\x00\x03\x00\x01\xA5\xFF'))

        # Finally we get a complete frame
        frame, trimmed_data = m5_reader.M5.get_response_frame(b'\xBB\x12\x34\x00\x01\x56\x9D\x7E')
        self.assertEqual(b'', trimmed_data)
        self.assertEqual(0x12, frame.type)
        self.assertEqual(0x34, frame.cmd)
        self.assertEqual(b'\x56', frame.payload)

        # A complete frame with data before and after
        frame, trimmed_data = m5_reader.M5.get_response_frame(b'123\xBB\x12\x34\x00\x01\x56\x9D\x7E456')
        self.assertEqual(b'456', trimmed_data)
        self.assertEqual(0x12, frame.type)
        self.assertEqual(0x34, frame.cmd)
        self.assertEqual(b'\x56', frame.payload)

        # An invalid frame, followed by a complete frame
        frame, trimmed_data = m5_reader.M5.get_response_frame(b'\xBB\xDE\xAD\x00\x01\xBE\x4A\xBB\x12\x34\x00\x01\x56\x9D\x7E456')
        self.assertEqual(b'456', trimmed_data)
        self.assertEqual(0x12, frame.type)
        self.assertEqual(0x34, frame.cmd)
        self.assertEqual(b'\x56', frame.payload)

        # Checksum error
        try:
            m5_reader.M5.get_response_frame(b'123\xBB\x12\x34\x00\x01\x56\xAA\x7E456')
        except m5_reader.ChecksumError as err:
            self.assertEqual(b'456', err.trimmed_data)  # It should allow picking up the pieces
        else:
            assert False, "Expected exception"

        self.assertRaises(m5_reader.ChecksumError, )


if __name__ == "__main__":
    unittest.main()