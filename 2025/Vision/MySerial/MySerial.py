import serial
import time
'''
说明手册:
@ 代码配置:
setup:
1. 初始化

while:
2. 插入数据(最好遵从:数据个数(即高低位总个数,如:[0]:2 , [1]:0x01 [2]:0x02))
(建议为第一位插入数据个数 , 后面一律insert_two_bytes(num_to_bytes(原始数据)))
3. 发送数据包

teardown:
4. 关闭串口

@ 使用pySerial库
只需要packet.ser.函数即可(多了个ser)

@ 串口协议:
1. 包头: 0xFF 0xAA
2. 包尾: 0x55 0xFE
3. 数据:数据个数 + 若干组高低位
        帧头 数据个数 高位  低位 高位 低位   帧尾
举例:0xFF 0xAA 0x04  0x00 0xFD 0x00 0x09 0x55 0xFE

'''

class SerialPacket:
    def __init__(self, port="COM6", baudrate=115200, timeout=0.1):
        """初始化串口"""
        try:
            self.ser = serial.Serial(port, baudrate, timeout=timeout)
            if self.ser.is_open:
                print(f"串口 {port} 已成功打开！")
            else:
                print(f"串口 {port} 打开失败！")
        except serial.SerialException as e:
            print(f"串口 {port} 打开失败，错误信息：{e}")
        except Exception as e:
            print(f"发生未知错误：{e}")
        # 初始化包头和包尾
        self.header = bytearray([0xFF, 0xAA])
        self.tail   = bytearray([0x55, 0xFE])
        self.data   = bytearray()
        self.index  = 0 # 数据插入位置
    
    def __clear_packet(self):
        """清空包数据，只保留包头"""
        self.data = bytearray()
        self.index = 0 # 数据插入位置清零
    
    def insert_byte(self, value):
        """在包头后插入单个字节"""
        self.data.insert(self.index, value)  # 插入到数据部分开头（包头之后）
        self.index += 1
    
    def insert_two_bytes(self, values):
        """在包头后插入两个字节"""
        self.data.insert(self.index, values[0])  # 插入到数据部分开头（包头之后）
        self.index += 1
        self.data.insert(self.index, values[1])  # 插入到数据部分开头（包头之后）
        self.index += 1

    def insert_bytes(self, index, values):
        """在指定位置插入多个字节"""
        for i, val in enumerate(values):
            self.data.insert(index + i, val)
            self.index += 1
    
    def num_to_bytes(self , value):
        """发送16位整数并拆分为高8位和低8位"""
        if not 0 <= value <= 0xFFFF:
            raise ValueError("输入值必须在 0~65535 之间")

        high_byte = (value >> 8) & 0xFF  # 高8位
        low_byte = value & 0xFF          # 低8位

        return [high_byte, low_byte]

    def __build_packet(self):
        """生成完整数据包"""
        return self.header + self.data + self.tail
    
    def send_packet(self):
        """发送完整数据包"""
        # 构建帧头 + 数据包 + 帧尾
        packet = self.__build_packet()
        # 发送数据包
        self.ser.write(packet)

        # # 十六进制美化输出
        # hex_str = ' '.join([f'{b:02X}' for b in packet])
        # print(f"发送: {hex_str}")

        # 清空数据包
        self.__clear_packet()

    def close(self):
        """关闭串口"""
        if self.ser.is_open:
            self.ser.close()

# 测试代码(配合STM32串口协议使用)
if __name__ == "__main__":
    # 先导入测试库
    import keyboard
    import time
    # 示例代码
    # 1. 创建串口对象
    packet = SerialPacket(port="COM6", baudrate=115200, timeout=0.1)

    num = 0x04  # 计划插入两个实体数据(也就是四个高低位)
    a = 0

    while True :
        if keyboard.is_pressed('s'):
            time.sleep(0.2) # 按键防抖
            # 2-1. 插入单个字节(已经有了包头包尾,这里是直接在包头后面插入)
            packet.insert_byte(num)
            
            # 2-2. 在指定位置(这里是数据包的第一个字节位置)插入多个字节(已经有了包头包尾和num(0号位),所以这里是直接在num后面插入),少用
            packet.insert_bytes(1, packet.num_to_bytes(253))
            
            # 2-3. 直接在数据位插入两个字节(适合高低位)
            packet.insert_two_bytes(packet.num_to_bytes(a))
            a += 1   # 搞点变化而已

            # 3. 发送数据包
            packet.send_packet()

        if keyboard.is_pressed('esc'):
            print("退出程序")
            break

    # 4. 关闭串口
    packet.close()
