import unittest
from MySerial import SerialPacket

class TestSerialPacket(unittest.TestCase):
    """
    单元测试类，用于测试 SerialPacket 类的功能。
    """

    def setUp(self):
        """
        在每个测试方法执行前初始化测试环境。
        这里创建一个 SerialPacket 实例，并模拟串口操作。
        """
        self.packet = SerialPacket(port="COM6", baudrate=115200, timeout=0.1)

    def tearDown(self):
        """
        在每个测试方法执行后清理测试环境。
        关闭串口连接。
        """
        self.packet.close()

    def test_insert_byte(self):
        """
        测试 insert_byte 方法：
        1. 验证在包头后插入单个字节的功能。
        2. 验证插入后索引的正确性。
        """
        # 插入一个字节
        self.packet.insert_byte(0x01)
        self.assertEqual(self.packet.data[0], 0x01)
        self.assertEqual(self.packet.index, 1)

        # 插入另一个字节
        self.packet.insert_byte(0x02)
        self.assertEqual(self.packet.data[1], 0x02)
        self.assertEqual(self.packet.index, 2)

    def test_insert_two_bytes(self):
        """
        测试 insert_two_bytes 方法：
        1. 验证在包头后插入两个字节的功能。
        2. 验证插入后索引的正确性。
        """
        # 插入两个字节
        self.packet.insert_two_bytes([0x01, 0x02])
        self.assertEqual(self.packet.data[0], 0x01)
        self.assertEqual(self.packet.data[1], 0x02)
        self.assertEqual(self.packet.index, 2)

    def test_insert_bytes(self):
        """
        测试 insert_bytes 方法：
        1. 验证在指定位置插入多个字节的功能。
        2. 验证插入后索引的正确性。
        """
        # 插入多个字节
        self.packet.insert_bytes(0, [0x01, 0x02, 0x03])
        self.assertEqual(self.packet.data[0], 0x01)
        self.assertEqual(self.packet.data[1], 0x02)
        self.assertEqual(self.packet.data[2], 0x03)
        self.assertEqual(self.packet.index, 3)

    def test_num_to_bytes(self):
        """
        测试 num_to_bytes 方法：
        1. 验证将16位整数拆分为高8位和低8位的功能。
        2. 验证输入值超出范围时的异常处理。
        """
        # 测试正常输入
        result = self.packet.num_to_bytes(0x1234)
        self.assertEqual(result, [0x12, 0x34])

        # 测试边界输入
        result = self.packet.num_to_bytes(0xFFFF)
        self.assertEqual(result, [0xFF, 0xFF])

        # 测试异常输入
        with self.assertRaises(ValueError):
            self.packet.num_to_bytes(0x10000)

    def test_build_packet(self):
        """
        测试 __build_packet 方法：
        1. 验证生成完整数据包的功能。
        2. 验证数据包包含包头、数据和包尾。
        """
        # 插入一些数据
        self.packet.insert_byte(0x01)
        self.packet.insert_byte(0x02)

        # 构建数据包
        packet = self.packet._SerialPacket__build_packet()
        self.assertEqual(packet[0], 0xFF)  # 包头
        self.assertEqual(packet[1], 0xAA)  # 包头
        self.assertEqual(packet[2], 0x01)  # 数据
        self.assertEqual(packet[3], 0x02)  # 数据
        self.assertEqual(packet[4], 0x55)  # 包尾
        self.assertEqual(packet[5], 0xFE)  # 包尾

    def test_send_packet(self):
        """
        测试 send_packet 方法：
        1. 验证发送数据包的功能。
        2. 验证发送后数据包被清空。
        """
        # 插入一些数据
        self.packet.insert_byte(0x01)
        self.packet.insert_byte(0x02)

        # 发送数据包
        self.packet.send_packet()
        self.assertEqual(len(self.packet.data), 0)  # 数据包被清空
        self.assertEqual(self.packet.index, 0)      # 索引被重置

if __name__ == "__main__":
    unittest.main()