import serial
import serial.tools.list_ports
import time
import sys
import os
import threading
import tkinter as tk
from tkinter import ttk, filedialog, scrolledtext, messagebox

try:
    from ymodem.Socket import ModemSocket
except ImportError:
    print("[-] 导入失败。请确保安装了最新版 ymodem: pip install ymodem")
    sys.exit(1)


class FlasherTab:
    """ OTA/Boot 双区智能烧录标签页 """
    def __init__(self, parent, main_app):
        self.frame = ttk.Frame(parent)
        self.main_app = main_app
        self.is_running = False
        self.flash_thread = None
        self.target_filepath = ""

        self.setup_ui()

    def setup_ui(self):
        # 1. 配置区域
        config_frame = ttk.LabelFrame(self.frame, text="烧录配置", padding=(10, 10))
        config_frame.pack(fill=tk.X, padx=10, pady=5)

        ttk.Label(config_frame, text="固件文件夹:").grid(row=0, column=0, sticky=tk.W, pady=5)
        self.folder_var = tk.StringVar(value=os.getcwd())
        self.folder_entry = ttk.Entry(config_frame, textvariable=self.folder_var, width=55)
        self.folder_entry.grid(row=0, column=1, padx=5, pady=5, sticky=tk.W)
        
        self.browse_btn = ttk.Button(config_frame, text="选择目录...", command=self.browse_folder)
        self.browse_btn.grid(row=0, column=2, padx=5, pady=5)

        ttk.Label(config_frame, text="💡 提示: 文件夹内需包含针对两区的固件文件 (如带 _A.bin 和 _B.bin 字样)", 
                  foreground="gray").grid(row=1, column=1, columnspan=2, sticky=tk.W)

        # 2. 模式选择
        mode_frame = ttk.LabelFrame(self.frame, text="触发模式", padding=(10, 5))
        mode_frame.pack(fill=tk.X, padx=10, pady=5)

        self.mode_var = tk.StringVar(value="OTA")
        ttk.Radiobutton(mode_frame, text="OTA 模式 (应用正在运行，发送 'OTA' 触发跳转)", variable=self.mode_var, value="OTA").pack(anchor=tk.W, pady=2)
        ttk.Radiobutton(mode_frame, text="首下 / Boot 模式 (发送 'U' 拦截启动或已停留在 Boot 菜单)", variable=self.mode_var, value="BOOT").pack(anchor=tk.W, pady=2)

        # 3. 操作与进度
        action_frame = ttk.Frame(self.frame)
        action_frame.pack(fill=tk.X, padx=10, pady=10)

        self.start_btn = ttk.Button(action_frame, text="▶ 智能匹配并烧录", command=self.start_flashing)
        self.start_btn.pack(side=tk.LEFT, padx=5)

        self.stop_btn = ttk.Button(action_frame, text="⏹ 中断", command=self.stop_flashing, state=tk.DISABLED)
        self.stop_btn.pack(side=tk.LEFT, padx=5)

        self.progress_var = tk.DoubleVar()
        self.progress_bar = ttk.Progressbar(action_frame, variable=self.progress_var, maximum=100)
        self.progress_bar.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=15)

        self.percent_label = ttk.Label(action_frame, text="0.0%")
        self.percent_label.pack(side=tk.LEFT, padx=5)

        # 4. 日志区域
        log_frame = ttk.LabelFrame(self.frame, text="烧录控制台日志", padding=(5, 5))
        log_frame.pack(fill=tk.BOTH, expand=True, padx=10, pady=5)

        self.log_text = scrolledtext.ScrolledText(log_frame, state='disabled', bg="#1e1e1e", fg="#d4d4d4", font=("Consolas", 10))
        self.log_text.pack(fill=tk.BOTH, expand=True)
        
        # 定义日志颜色
        self.log_text.tag_config("red", foreground="#ff5555")
        self.log_text.tag_config("green", foreground="#55ff55")
        self.log_text.tag_config("yellow", foreground="#ffff55")

    def browse_folder(self):
        folder = filedialog.askdirectory(title="选择固件存放目录")
        if folder:
            self.folder_var.set(folder)

    def log(self, message, color=None):
        def append():
            self.log_text.config(state='normal')
            if color:
                self.log_text.insert(tk.END, message, color)
            else:
                self.log_text.insert(tk.END, message)
            self.log_text.see(tk.END)
            self.log_text.config(state='disabled')
        self.main_app.root.after(0, append)

    def update_progress(self, percent, text):
        def update():
            self.progress_var.set(percent)
            self.percent_label.config(text=text)
        self.main_app.root.after(0, update)

    def toggle_ui(self, running):
        state = tk.DISABLED if running else tk.NORMAL
        self.start_btn.config(state=state)
        self.browse_btn.config(state=state)
        self.stop_btn.config(state=tk.NORMAL if running else tk.DISABLED)
        self.main_app.toggle_top_config(running)

    def find_firmware(self, folder, target_bank):
        """ 在目录下寻找匹配 Bank 名字的固件 """
        valid_files = []
        try:
            for f in os.listdir(folder):
                if f.lower().endswith('.bin'):
                    name = f.upper()
                    if f"_{target_bank}" in name or f"BANK{target_bank}" in name or name.endswith(f"{target_bank}.BIN"):
                        valid_files.append(os.path.join(folder, f))
        except Exception as e:
            self.log(f"[-] 目录读取失败: {e}\n", "red")
            return None

        if valid_files:
            return valid_files[0]
        return None

    def start_flashing(self):
        port, baud = self.main_app.get_port_config()
        folder_path = self.folder_var.get()
        mode = self.mode_var.get()

        if not port:
            messagebox.showwarning("警告", "请在顶部选择串口！")
            return
        if not os.path.exists(folder_path):
            messagebox.showwarning("警告", f"找不到目录:\n{folder_path}")
            return

        # 检查并抢占串口
        if self.main_app.debugger_tab.running:
            self.main_app.debugger_tab.close_serial()
            self.log("[*] 已自动关闭调试页的串口连接以便专用于烧录任务。\n", "yellow")

        self.log_text.config(state='normal')
        self.log_text.delete(1.0, tk.END)
        self.log_text.config(state='disabled')

        self.is_running = True
        self.toggle_ui(True)
        self.update_progress(0, "0.0%")
        
        self.flash_thread = threading.Thread(target=self.flash_task, args=(port, baud, folder_path, mode), daemon=True)
        self.flash_thread.start()

    def stop_flashing(self):
        self.is_running = False
        self.log("\n[-] 收到中断请求，正在安全停止...\n", "red")

    def flash_task(self, port, baud, folder_path, mode):
        ser = None
        try:
            ser = serial.Serial(port, int(baud), timeout=0.05)
            self.log(f"[+] 串口 {port} 已打开，波特率 {baud}\n", "green")
        except Exception as e:
            self.log(f"[-] 串口打开失败: {e}\n", "red")
            self.main_app.root.after(0, lambda: self.toggle_ui(False))
            return

        buffer = b""
        last_tx_time = 0
        state = "TRIGGER_OTA" if mode == "OTA" else "PROBE_STAGE"
        self.log(f">>> [启动] 模式: {mode}，初始状态: {state}\n", "yellow")

        try:
            while self.is_running and state != "DONE":
                now = time.time()

                if ser.in_waiting > 0:
                    data = ser.read(ser.in_waiting)
                    buffer += data
                    if len(buffer) > 4096: buffer = buffer[-4096:]
                    self.log(data.decode('utf-8', errors='ignore'))

                # 1. 触发引导
                if state == "TRIGGER_OTA":
                    if now - last_tx_time > 2:
                        ser.write(b"OTA\r\n")
                        last_tx_time = now
                    
                    if b"Main Menu" in buffer:
                        state = "WAIT_MENU_1"
                        buffer = b""
                        last_tx_time = now

                elif state == "PROBE_STAGE":
                    if b"Press 'U'" in buffer:
                        ser.write(b"U")
                        buffer = b""
                        state = "WAIT_MENU_1"
                    elif b"Main Menu" in buffer:
                        state = "WAIT_MENU_1"
                        buffer = b""
                    elif now - last_tx_time > 1.5:
                        ser.write(b"\r\n")
                        last_tx_time = now

                # 2. 菜单加载后，查询当前运行区
                elif state == "WAIT_MENU_1":
                    if b"==========================================================" in buffer or (now - last_tx_time > 2):
                        self.log("\n>>> [1/5] 菜单加载完成，发送 '6' 查询 Active Bank...\n", "yellow")
                        ser.write(b"6")
                        state = "WAIT_BANK_QUERY"
                        buffer = b""
                        last_tx_time = now

                # 3. 解析双区信息，寻找固件
                elif state == "WAIT_BANK_QUERY":
                    target_bank = None
                    
                    if b"Active: BankA" in buffer:
                        target_bank = "B"
                        self.log("\n>>> [2/5] 检测到运行 BankA，将要烧录至 BankB...\n", "green")
                    elif b"Active: BankB" in buffer:
                        target_bank = "A"
                        self.log("\n>>> [2/5] 检测到运行 BankB，将要烧录至 BankA...\n", "green")
                    elif b"Active: None" in buffer or b"No Program" in buffer:
                        # 核心修复：处理空片/首次下载的情况
                        target_bank = "A" 
                        self.log("\n>>> [2/5] 检测到空片 (Active: None)，首次下载默认烧录至 BankA...\n", "yellow")

                    if target_bank:
                        # 查找目录中对应的固件
                        fw_path = self.find_firmware(folder_path, target_bank)
                        if not fw_path:
                            self.log(f"[-] 严重错误: 文件夹中未找到匹配 Bank{target_bank} 的固件！\n", "red")
                            self.is_running = False
                            break
                        
                        self.target_filepath = fw_path
                        self.log(f"[+] 自动选中目标固件: {fw_path}\n", "green")
                        state = "WAIT_MENU_2"
                        buffer = b""
                        last_tx_time = now
                    elif now - last_tx_time > 3:
                        ser.write(b"6")
                        last_tx_time = now

                # 4. 再次等待菜单加载完毕，进入 Ymodem 下载
                elif state == "WAIT_MENU_2":
                    if b"==========================================================" in buffer or (now - last_tx_time > 2.5):
                        self.log("\n>>> [3/5] 发送 '1' 触发 Ymodem 协议下载...\n", "yellow")
                        ser.write(b"1")
                        state = "WAIT_YMODEM_START"
                        buffer = b""
                        last_tx_time = now

                elif state == "WAIT_YMODEM_START":
                    if b"Waiting for the file to be sent" in buffer:
                        self.log("\n>>> [4/5] 板子就绪，移交协议引擎传输...\n", "green")
                        time.sleep(0.5) 
                        state = "DO_YMODEM"
                        buffer = b""

                # 5. 执行 Ymodem
                elif state == "DO_YMODEM":
                    def getc(size, timeout=2):
                        if not self.is_running: return None
                        ser.timeout = timeout
                        return ser.read(size) or None

                    def putc(data, timeout=2):
                        if not self.is_running: return 0
                        ser.write_timeout = timeout
                        ser.write(data)
                        return len(data)

                    def progress(task_index, file_name, total, success):
                        if total > 0:
                            pct = (success / total) * 100
                            self.update_progress(pct, f"{pct:.1f}% ({success}/{total})")

                    modem = ModemSocket(getc, putc)
                    try:
                        status = modem.send([self.target_filepath], callback=progress)
                    except Exception as e:
                        status = False
                        self.log(f"\n[-] Ymodem 异常: {e}\n", "red")

                    ser.timeout = 0.05 

                    if status and self.is_running:
                        self.log("\n[+] 传输完成！等待 Flash 写入和校验...\n", "green")
                        self.update_progress(100, "100% (校验中)")
                        state = "WAIT_FLASH_DONE"
                    else:
                        if self.is_running:
                            self.log("\n[-] 传输中断，将重试...\n", "red")
                            state = "WAIT_MENU_1" 
                        buffer = b""

                # 6. 等待写入成功标识
                elif state == "WAIT_FLASH_DONE":
                    if b"Programming Completed Successfully" in buffer or b"Successfully" in buffer:
                        self.log("\n>>> [5/5] Flash烧录成功！准备执行启动命令...\n", "yellow")
                        state = "WAIT_MENU_3"
                        buffer = b""
                        last_tx_time = now

                # 7. 再次等待菜单，发送 3 跳转
                elif state == "WAIT_MENU_3":
                    if b"====" in buffer or (now - last_tx_time > 2.5):
                        state = "SEND_3_JUMP"
                        buffer = b""
                        last_tx_time = 0

                elif state == "SEND_3_JUMP":
                    if now - last_tx_time > 1:
                        ser.write(b"3")
                        last_tx_time = now

                    if any(x in buffer for x in [b"JUMPING", b"Start program", b"[APP]"]):
                        self.log("\n==================================\n", "green")
                        self.log(">>> 🎉 固件刷入并跳转成功！\n", "green")
                        self.log("==================================\n", "green")
                        self.update_progress(100, "启动完成!")
                        state = "DONE"

                time.sleep(0.01)

        except Exception as e:
            self.log(f"\n[-] 运行中发生异常: {e}\n", "red")
        finally:
            if ser and ser.is_open:
                ser.close()
                self.log("\n[*] 释放 OTA 串口。\n")
            self.is_running = False
            self.main_app.root.after(0, lambda: self.toggle_ui(False))


class DebuggerTab:
    """ 串口通信与指令配置标签页 """
    def __init__(self, parent, main_app):
        self.frame = ttk.Frame(parent)
        self.main_app = main_app
        self.ser = None
        self.running = False
        self.setup_ui()

    def setup_ui(self):
        # 1. 独立连接控制
        conn_frame = ttk.Frame(self.frame)
        conn_frame.pack(fill=tk.X, padx=10, pady=10)

        self.btn_connect = tk.Button(conn_frame, text="打开调试串口", bg="lightgreen", command=self.toggle_connection, width=15)
        self.btn_connect.pack(side=tk.LEFT, padx=5)

        # 2. 功能操作区
        func_frame = ttk.LabelFrame(self.frame, text="快捷指令与配置", padding=(10, 10))
        func_frame.pack(fill=tk.X, padx=10, pady=5)

        self.btn_read_sn = ttk.Button(func_frame, text="读取 SN 码 (SNREAD)", command=self.read_sn)
        self.btn_read_sn.grid(row=0, column=0, padx=5, pady=5)

        ttk.Label(func_frame, text="输入新 SN:").grid(row=0, column=1, padx=(15, 5))
        self.entry_sn = ttk.Entry(func_frame, width=20)
        self.entry_sn.grid(row=0, column=2, padx=5)
        self.btn_write_sn = ttk.Button(func_frame, text="写入 SN 码", command=self.write_sn)
        self.btn_write_sn.grid(row=0, column=3, padx=5, pady=5)

        ttk.Label(func_frame, text="自定义指令:").grid(row=1, column=0, padx=5, sticky=tk.E, pady=10)
        self.entry_custom = ttk.Entry(func_frame, width=35)
        self.entry_custom.grid(row=1, column=1, columnspan=2, padx=5, sticky=tk.W)
        self.btn_send_custom = ttk.Button(func_frame, text="发送", command=self.send_custom)
        self.btn_send_custom.grid(row=1, column=3, padx=5, pady=10)

        # 3. 日志区
        log_frame = ttk.LabelFrame(self.frame, text="串口数据流", padding=(5, 5))
        log_frame.pack(fill=tk.BOTH, expand=True, padx=10, pady=5)

        self.text_log = scrolledtext.ScrolledText(log_frame, bg="#1e1e1e", fg="#d4d4d4", font=("Consolas", 10))
        self.text_log.pack(fill=tk.BOTH, expand=True)

        self.text_log.tag_config("tx", foreground="#55ffff")
        self.text_log.tag_config("rx", foreground="#d4d4d4")
        self.text_log.tag_config("err", foreground="#ff5555")

        self.btn_clear = ttk.Button(log_frame, text="清空日志", command=lambda: self.text_log.delete(1.0, tk.END))
        self.btn_clear.pack(anchor=tk.E, pady=(5, 0))

    def toggle_connection(self):
        if self.main_app.flasher_tab.is_running:
            messagebox.showwarning("警告", "正在执行烧录任务，请先停止烧录再打开调试！")
            return

        if self.ser is None or not self.ser.is_open:
            port, baud = self.main_app.get_port_config()
            if not port:
                messagebox.showwarning("提示", "请在顶部选择串口")
                return
            try:
                self.ser = serial.Serial(port, int(baud), timeout=1)
                self.running = True
                self.btn_connect.config(text="关闭串口", bg="#ff9999")
                self.log(f"✅ 调试串口已打开: {port} @ {baud}\n", "green")
                
                self.read_thread = threading.Thread(target=self.receive_data, daemon=True)
                self.read_thread.start()
            except Exception as e:
                messagebox.showerror("串口错误", f"无法打开串口 {port}:\n{e}")
        else:
            self.close_serial()

    def receive_data(self):
        while self.running and self.ser and self.ser.is_open:
            try:
                if self.ser.in_waiting > 0:
                    data = self.ser.read(self.ser.in_waiting)
                    decoded_data = data.decode('utf-8', errors='ignore')
                    self.main_app.root.after(0, self.log, decoded_data, "rx")
                else:
                    time.sleep(0.01)
            except Exception as e:
                if self.running:
                    self.main_app.root.after(0, self.log, f"\n[错误] 读取断开: {e}\n", "err")
                    self.main_app.root.after(0, self.close_serial)
                break

    def send_data(self, data_str):
        if self.ser and self.ser.is_open:
            if not data_str.endswith('\n'):
                data_str += '\n'
            try:
                self.ser.write(data_str.encode('utf-8'))
                self.log(f"\n[TX] {data_str.strip()}\n", "tx")
            except Exception as e:
                messagebox.showerror("发送错误", str(e))
        else:
            messagebox.showwarning("提示", "请先打开串口！")

    def read_sn(self):
        self.send_data("SNREAD")

    def write_sn(self):
        sn_value = self.entry_sn.get().strip()
        if sn_value:
            self.send_data(f"SN:{sn_value}")
        else:
            messagebox.showwarning("提示", "请输入要写入的SN码")

    def send_custom(self):
        cmd = self.entry_custom.get().strip()
        if cmd:
            self.send_data(cmd)

    def log(self, message, tag=None):
        self.text_log.insert(tk.END, message, tag)
        self.text_log.see(tk.END)

    def close_serial(self):
        self.running = False
        if self.ser and self.ser.is_open:
            self.ser.close()
        self.btn_connect.config(text="打开调试串口", bg="lightgreen")
        self.log("\n❌ 串口已关闭。\n", "err")


class MainApp:
    """ 主程序框架：管理全局串口配置与切换 Tab """
    def __init__(self, root):
        self.root = root
        self.root.title("Brain Servo V2 - 综合调试与烧录工作站")
        self.root.geometry("850x700")
        self.root.minsize(750, 600)

        # 1. 顶部全局区域
        self.setup_shared_top()

        # 2. 核心分页控件
        self.notebook = ttk.Notebook(self.root)
        self.notebook.pack(fill=tk.BOTH, expand=True, padx=10, pady=5)

        # 3. 实例化两个页面
        self.flasher_tab = FlasherTab(self.notebook, self)
        self.debugger_tab = DebuggerTab(self.notebook, self)

        self.notebook.add(self.flasher_tab.frame, text=" 🚀 OTA 固件烧录 ")
        self.notebook.add(self.debugger_tab.frame, text=" 🛠️ 调试与指令发送 ")

        self.root.protocol("WM_DELETE_WINDOW", self.on_closing)

    def setup_shared_top(self):
        top_frame = ttk.LabelFrame(self.root, text="全局设备配置", padding=(10, 5))
        top_frame.pack(fill=tk.X, padx=10, pady=5)

        ttk.Label(top_frame, text="通讯串口:").pack(side=tk.LEFT, padx=5)
        self.port_var = tk.StringVar()
        self.port_cb = ttk.Combobox(top_frame, textvariable=self.port_var, width=18)
        self.port_cb.pack(side=tk.LEFT, padx=5)

        self.refresh_btn = ttk.Button(top_frame, text="刷新设备", command=self.refresh_ports)
        self.refresh_btn.pack(side=tk.LEFT, padx=5)

        ttk.Label(top_frame, text="通讯波特率:").pack(side=tk.LEFT, padx=(30, 5))
        self.baud_var = tk.StringVar(value="115200")
        self.baud_cb = ttk.Combobox(top_frame, textvariable=self.baud_var, width=12, values=["9600", "115200", "256000", "921600"])
        self.baud_cb.pack(side=tk.LEFT, padx=5)

        self.refresh_ports()

    def refresh_ports(self):
        ports = [port.device for port in serial.tools.list_ports.comports()]
        self.port_cb['values'] = ports
        if ports:
            self.port_cb.current(0)
        else:
            self.port_cb.set('')

    def get_port_config(self):
        return self.port_var.get(), self.baud_var.get()

    def toggle_top_config(self, lock):
        state = tk.DISABLED if lock else tk.NORMAL
        self.port_cb.config(state=state)
        self.refresh_btn.config(state=state)
        # 波特率保持原样，可调整

    def on_closing(self):
        """ 安全关闭所有正在运行的后台线程 """
        self.flasher_tab.stop_flashing()
        self.debugger_tab.close_serial()
        self.root.destroy()


if __name__ == "__main__":
    root = tk.Tk()
    app = MainApp(root)
    root.mainloop()