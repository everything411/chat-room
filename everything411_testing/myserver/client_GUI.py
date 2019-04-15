#!/usr/bin/env python
# -*- coding: utf-8 -*-
import socket
import threading
import tkinter as tk
import tkinter.scrolledtext as tkst
import json

op_login = 'login '
op_broadcast = 'broadcast '
op_sendto = 'send '


class Client(tk.Tk):
    def __init__(self, *args, **kwargs):
        tk.Tk.__init__(self, *args, **kwargs)
        self.prompt = ''
        self.__socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.__login = False

        self.message_line = 0
        self.name = tk.StringVar()
        self.password = tk.StringVar()
        self.serveraddr = tk.StringVar()
        self.serverport = tk.StringVar()

        self.resizable(False, False)
        container = tk.Frame(self)
        container.pack(side='top', fill='both', expand=True)
        container.grid_rowconfigure(0, weight=1)
        container.grid_columnconfigure(0, weight=1)

        self.frames = {}

        frame_name = LoginFrame.__name__
        frame = LoginFrame(container, self)
        self.frames[frame_name] = frame

        frame_name = ChattingFrame.__name__
        frame = ChattingFrame(container, self)
        self.frames[frame_name] = frame

        self.raise_frame("LoginFrame")

    def raise_frame(self, frame_name):
        for frame in self.frames.values():
            frame.grid_remove()
        frame = self.frames[frame_name]
        frame.grid(row=0, column=0, sticky='ewsn')
        frame.tkraise()

    def get_frame_by_name(self, frame_name):
        for frame in self.frames.values():
            if str(frame.__class__.__name__) == frame_name:
                return frame
        print(frame_name + "NOT FOUND!")
        return None

    def login(self, user_name, addr, port):
        self.prompt = '[@' + user_name + ']> '
        try:
            self.__socket.connect((addr, port))
            # if connection succeeds, send login message
            send_message = (op_login + user_name).encode()
            self.__socket.sendall(send_message)
            data = self.__socket.recv(1024)
            json_data = json.loads(data)
            if json_data['status'] == 'error':
                raise ValueError(json_data['value'])
            elif json_data['status'] == 'success':
                self.__login = True
                self.raise_frame("ChattingFrame")
                message = '[System] Login Success. Welcome, [' + \
                    json_data['user'] + '] !\n'
                self.get_frame_by_name(
                    'ChattingFrame').add_message(message, "green")
                thread = threading.Thread(target=self.receive_message_thread)
                thread.setDaemon(True)
                thread.start()
        except ValueError as ve:
            self.get_frame_by_name('LoginFrame').add_message(ve, "red")
            self.name.set("")
            self.password.set("")
            self.__socket.close()
            self.__socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        except ConnectionRefusedError as cre:
            self.get_frame_by_name('LoginFrame').add_message(cre, "red")
            self.serveraddr.set("127.0.0.1")
            self.serverport.set("9877")

    def logout(self):
        self.connected = False
        self.__login = False
        self.__socket.close()
        self.__socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.destroy()

    def display_broadcast(self, message):
        json_data = json.loads(message)
        if json_data['user'] == self.name.get():
            self.get_frame_by_name("ChattingFrame").\
                add_message('[You@' + json_data['user'] + ']> ' +
                            json_data['content'] + '\n', "grey")
        else:
            self.get_frame_by_name("ChattingFrame").\
                add_message('[@' + json_data['user'] + ']> ' +
                            json_data['content'] + '\n', "black")

    def display_message(self, message):
        json_data = json.loads(message)
        if json_data['user'] == self.name.get():
            self.get_frame_by_name("ChattingFrame").\
                add_message('(Private)[You@' + json_data['user'] + ']> ' +
                            json_data['content'] + '\n', "grey")
        else:
            self.get_frame_by_name("ChattingFrame").\
                add_message('(Private)[@' + json_data['user'] + ']> ' +
                            json_data['content'] + '\n', "black")

    def display_system_message(self, message):
        json_data = json.loads(message)
        if json_data['type'] == 'logininfo':
            self.get_frame_by_name("ChattingFrame").\
                add_message('[System] [' + json_data['user'] +
                            '] has joined the chat\n', "blue")
        elif json_data['type'] == 'exitinfo':
            self.get_frame_by_name("ChattingFrame"). \
                add_message('[System] [' + json_data['user'] +
                            '] has left the chat\n', "blue")
        elif json_data['type'] == 'error':
            self.get_frame_by_name("ChattingFrame"). \
                add_message('[ERROR]' +
                            json_data['value'] + '\n', "red")

    def receive_message_thread(self):
        while self.__login:
            try:
                data = self.__socket.recv(40960).decode()
            except Exception:
                print("[Client] Connection Close")
                break
            try:
                json_data = json.loads(data)
                if json_data['type'] == 'logininfo' or json_data['type'] == 'exitinfo' or json_data['type'] == 'error':
                    self.display_system_message(data)
                elif json_data['type'] == 'broadcast':
                    self.display_broadcast(data)
                elif json_data['type'] == 'message':
                    self.display_message(data)
            except ValueError:
                    self.display_system_message('{"type":"error","status":"error","value":"ClientERRInvalidJSON"}')

    def send_private_message(self, message):
        send_message = (op_sendto + message).encode()
        self.__socket.sendall(send_message)

    def broadcast_message(self, message):
        send_message = (op_broadcast + message).encode()
        self.__socket.sendall(send_message)


class LoginFrame(tk.Frame):
    def __init__(self, parent, controller):
        tk.Frame.__init__(self, parent)
        self.controller = controller

        self.receive_message_window = tk.Label(
            self, text="Welcome To My Chatroom :)")
        self.receive_message_window['font'] = ('Sarasa Term SC', 10)
        self.receive_message_window.grid(
            row=4, columnspan=2, padx=5, sticky="nsew")

        tk.Label(self, text=" Your username :").grid(row=0, column=0, pady=10)
        entry_name = tk.Entry(self, textvariable=self.controller.name)
        entry_name.grid(row=0, column=1, ipadx=30, padx=15, pady=10)

        # entry_name.bind('<KeyRelease-Return>', self.login)

        tk.Label(self, text=" Your password :").grid(row=1, column=0, pady=10)
        entry_name = tk.Entry(self, textvariable=self.controller.password)
        entry_name.grid(row=1, column=1, ipadx=30, padx=15, pady=10)

        entry_name.bind('<KeyRelease-Return>', self.login)

        tk.Label(self, text=" Server IP:").grid(
            row=2, column=0, pady=10)
        entry_name = tk.Entry(
            self, textvariable=self.controller.serveraddr)
        entry_name.grid(row=2, column=1, ipadx=30, padx=15, pady=10)

        tk.Label(self, text=" Server PORT:").grid(row=3, column=0, pady=10)
        entry_name = tk.Entry(
            self, textvariable=self.controller.serverport)
        entry_name.grid(row=3, column=1, ipadx=30, padx=25, pady=10)

        self.login_button = tk.Button(
            self, text="LOGIN", width=10, command=self.login)
        self.login_button.grid(row=5, column=0, padx=10, pady=10)
        self.logout_button = tk.Button(
            self, text="EXIT", width=10, command=self.logout)
        self.logout_button.grid(row=5, column=1, padx=10, pady=10)

        self.login_button.bind('<Return>', self.login)
        self.logout_button.bind('<Return>', self.logout)
        self.controller.serveraddr.set("127.0.0.1")
        self.controller.serverport.set("9877")

    def login(self, event=None):
        # self.username = self.controller.name.get()
        addr = self.controller.serveraddr.get()
        port = int(self.controller.serverport.get())
        user_name = self.controller.name.get() + ' ' + self.controller.password.get()

        self.controller.connecting_thread = threading.Thread(
            target=self.controller.login, args=[user_name, addr, port])
        self.controller.connecting_thread.setDaemon(True)
        self.controller.connecting_thread.start()

    def logout(self, event=None):
        self.controller.__login = False
        self.controller.destroy()

    def add_message(self, new_message, color="red"):
        self.receive_message_window["text"] = new_message


class ChattingFrame(tk.Frame):
    def __init__(self, parent, controller):
        tk.Frame.__init__(self, parent)
        self.controller = controller

        self.sendto = tk.StringVar()

        self.receive_message_window = tkst.ScrolledText(
            self, width=60, height=20, undo=True)
        self.receive_message_window['font'] = ('Sarasa Term SC', 12)
        self.receive_message_window.grid(
            row=1, columnspan=3, padx=10, pady=10, sticky="nsew")
        self.type_message_window = tk.Text(self, width=40, height=5, undo=True)
        self.type_message_window['font'] = ('Sarasa Term SC', 12)
        self.type_message_window.grid(
            row=2, padx=10, pady=10, rowspan=2, sticky="nsew")

        self.send_button = tk.Button(
            self, text="BROADCAST", width=10, command=self.send_message_from_gui_button)
        self.send_button.grid(row=2, column=1, padx=10, pady=5)

        self.send_button = tk.Button(
            self, text="SEND TO", width=10, command=self.send_private_message_from_gui_button)
        self.send_button.grid(row=3, column=1, padx=10, pady=5)

        self.logout_button = tk.Button(
            self, text="EXIT", width=10, command=self.logout)
        self.logout_button.grid(row=2, column=2, padx=10, pady=5)

        entry_name = tk.Entry(
            self, textvariable=self.sendto)
        entry_name.grid(row=3, column=2, padx=10)

        self.type_message_window.bind(
            '<KeyRelease-Return>', self.send_message_from_gui)
        self.logout_button.bind('<Return>', self.logout)

        # prevent receive_message_window from input
        self.receive_message_window.config(state=tk.DISABLED)

    def send_message_from_gui_button(self, event=None):
        message = self.type_message_window.get("1.0", tk.END + '-1c')
        self.controller.broadcast_message(message)
        self.type_message_window.delete("1.0", tk.END)

    def send_message_from_gui(self, event=None):
        message = self.type_message_window.get("1.0", tk.END + '-1c')
        self.controller.broadcast_message(message)
        self.type_message_window.delete("1.0", tk.END)

    def send_private_message_from_gui_button(self, event=None):
        message = self.sendto.get() + ' ' + self.type_message_window.get("1.0", tk.END + '-1c')
        self.controller.send_private_message(message)
        self.type_message_window.delete("1.0", tk.END)

    def logout(self, event=None):
        self.controller.logout()

    def add_message(self, new_message, color="black"):
        self.controller.message_line += 1
        temp = "tag_" + str(self.controller.message_line)
        self.receive_message_window.tag_config(temp, foreground=color)
        self.receive_message_window.config(state=tk.NORMAL)
        self.receive_message_window.insert(tk.END, new_message, temp)
        self.receive_message_window.config(state=tk.DISABLED)
        self.receive_message_window.see(tk.END)


if __name__ == '__main__':
    client = Client()
    client.title("BITNP Chatroom Room")
    client.mainloop()
