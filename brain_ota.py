import serial
import time
import sys
import os

try:
    from ymodem.Socket import ModemSocket
except ImportError:
    print("[-] 导入失败。请确保安装了最新版 ymodem: pip install ymodem")
    sys.exit(1)

# ================= 配置区域 =================
COM_PORT = 'COM3'              # 串口号
BAUD_RATE = 115200             # 波特率
FILE_PATH = './brain_servo_canfd/Debug/brain_servo_canfd.bin'  # 你的 APP 固件相对或绝对路径
# ============================================

def main():
    if not os.path.exists(FILE_PATH):
        print(f"[-] 找不到固件文件 {FILE_PATH}")
        sys.exit(1)

    try:
        # 使用极短的 timeout 以实现非阻塞轮询
        ser = serial.Serial(COM_PORT, BAUD_RATE, timeout=0.05)
        print(f"[+] 串口 {COM_PORT} 已打开，自动化 OTA 状态机启动...")
    except Exception as e:
        print(f"[-] 串口打开失败: {e}")
        sys.exit(1)

    # 初始状态
    state = "TRIGGER_OTA"
    buffer = b""
    last_tx_time = 0

    print(f"\n>>> [状态切换] {state} : 尝试触发板子进入 Bootloader")

    try:
        while state != "DONE":
            now = time.time()

            # ==========================================
            # 1. 持续非阻塞读取串口，并实时打印
            # ==========================================
            if ser.in_waiting > 0:
                data = ser.read(ser.in_waiting)
                buffer += data
                
                # 限制 buffer 长度，防止内存无限增长，同时保留足够上下文
                if len(buffer) > 4096:
                    buffer = buffer[-4096:]
                
                # 实时输出到控制台
                sys.stdout.write(data.decode('utf-8', errors='ignore'))
                sys.stdout.flush()

            # ==========================================
            # 2. 状态机逻辑判定区
            # ==========================================
            if state == "TRIGGER_OTA":
                # 每隔 2 秒发一次 OTA，直到看到主菜单
                if now - last_tx_time > 2:
                    ser.write(b"OTA\r\n")
                    last_tx_time = now
                
                if b"Main Menu" in buffer:
                    print("\n\n>>> [状态切换] WAIT_MENU_END : 检测到菜单头，等待菜单完全打印完毕...")
                    state = "WAIT_MENU_END"
                    buffer = b""
                    last_tx_time = now

            elif state == "WAIT_MENU_END":
                # ST 菜单的最后一行是 "============="。
                # 看到它，或者干等 1.5 秒，确保板子的 Flush 操作已经执行完毕
                if b"==========================================================" in buffer or (now - last_tx_time > 1.5):
                    print("\n\n>>> [状态切换] SEND_1_YMODEM : 菜单打印完毕，准备发送 '1'")
                    state = "SEND_1_YMODEM"
                    buffer = b""
                    last_tx_time = 0 # 将时间归零，强制立即发第一次

            elif state == "SEND_1_YMODEM":
                # 每隔 1 秒发一次 '1'，直到板子确切回复 Waiting
                if now - last_tx_time > 1:
                    ser.write(b"1")
                    last_tx_time = now
                
                if b"Waiting for the file" in buffer:
                    print("\n\n>>> [状态切换] DO_YMODEM : 板子就绪，移交 Ymodem 协议...")
                    time.sleep(0.5) # 给板子吐出字符 'C' 留出时间
                    state = "DO_YMODEM"
                    buffer = b""

            elif state == "DO_YMODEM":
                # Ymodem 需要的底层读写函数
                def getc(size, timeout=2):
                    ser.timeout = timeout
                    return ser.read(size) or None

                def putc(data, timeout=2):
                    ser.write_timeout = timeout
                    ser.write(data)
                    return len(data)

                def progress(task_index, file_name, total, success):
                    if total > 0:
                        pct = (success / total) * 100
                        sys.stdout.write(f"\r[YMODEM] 传输 {file_name} ... {pct:.1f}% ({success}/{total} 包)")
                        sys.stdout.flush()

                # 实例化并阻塞发送
                modem = ModemSocket(getc, putc)
                status = modem.send([FILE_PATH], callback=progress)
                print() # 换行
                
                # 恢复非阻塞超时设置
                ser.timeout = 0.05 

                if status:
                    print("\n[+] Ymodem 传输成功！")
                    print(">>> [状态切换] WAIT_FLASH_DONE : 等待 Flash 烧录确认")
                    state = "WAIT_FLASH_DONE"
                    buffer = b""
                else:
                    print("\n[-] 传输失败，退回菜单状态重试...")
                    state = "WAIT_MENU_END" 
                    buffer = b""

            elif state == "WAIT_FLASH_DONE":
                # 等待板子把固件写完并提示成功
                if b"Programming Completed Successfully" in buffer:
                    print("\n\n>>> [状态切换] WAIT_MENU_END_AGAIN : 烧录成功，等待重新显示菜单...")
                    state = "WAIT_MENU_END_AGAIN"
                    buffer = b""
                    last_tx_time = now

            elif state == "WAIT_MENU_END_AGAIN":
                # 再次等待底部边框，防止发送 3 的时候被吃掉
                if b"==========================================================" in buffer or (now - last_tx_time > 2):
                    print("\n\n>>> [状态切换] SEND_3_JUMP : 发送 '3' 尝试跳转 APP")
                    state = "SEND_3_JUMP"
                    buffer = b""
                    last_tx_time = 0

            elif state == "SEND_3_JUMP":
                # 每隔 1 秒发一次 '3'
                if now - last_tx_time > 1:
                    ser.write(b"3")
                    last_tx_time = now

                # 根据你之前加的遗言，以及 APP 的启动提示来判断是否跳转成功
                if b"JUMPING NOW" in buffer or b"Servo Ready" in buffer or b"Start program execution" in buffer:
                    print("\n\n>>> [状态切换] DONE : 完美！OTA 自动化流程全部闭环！🎉")
                    state = "DONE"

            time.sleep(0.01) # 防止死循环占满 CPU

    except KeyboardInterrupt:
        print("\n[-] 用户手动中断。")
    finally:
        ser.close()
        print("[*] 串口已释放。")

if __name__ == "__main__":
    main()