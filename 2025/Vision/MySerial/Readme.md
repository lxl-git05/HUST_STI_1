# MySerial说明手册:

## 1. 代码配置:

1. setup:

```py
# 1. 创建串口对象
packet = SerialPacket(port="COM6", baudrate=115200, timeout=0.1)
```

2. while:

```py
# 2-1. 插入单个字节(已经有了包头包尾,这里是直接在包头后面插入)
packet.insert_byte(num)

# 2-2. 在指定位置(这里是数据包的第一个字节位置)插入多个字节(已经有了包头包尾和num(0号位),所以这里是直接在num后面插入),少用
packet.insert_bytes(1, packet.num_to_bytes(253))

# 2-3. 直接在数据位插入两个字节(适合高低位)
packet.insert_two_bytes(packet.num_to_bytes(a))
a += 1   # 搞点变化而已

# 3. 发送数据包
packet.send_packet()
```

**插入数据(最好遵从:数据个数(即高低位总个数,如:[0]:2 , [1]:0x01 [2]:0x02))**

**(建议为第一位插入数据个数 , 后面一律insert_two_bytes(num_to_bytes(原始数据)))**

3. 结尾

```py
# 4. 关闭串口
packet.close()
```

## 2. 使用pySerial库

只需要packet.ser.函数即可(多了个ser)

## 3. 串口协议:

1. 包头: 0xFF 0xAA
2. 包尾: 0x55 0xFE
3. 数据:数据个数 + 若干组高低位
                     帧头     数据个数 高位  低位   高位  低位    帧尾
4. **举例:0xFF 0xAA   0x04     0x00 0xFD 0x00 0x09 0x55 0xFE**