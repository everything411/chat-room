import socket 
import select
import sys
import threading
import os
import json
from cmd import Cmd

# Copy from Zhang Dalao's testing_client2 on GitLab.  Zhang Dalao NEWBIE!

class Client(Cmd):
    """
    the Client
    """

    intro = "[Client] 欢迎使用我们的聊天室！\n[ver 0.0.1] 目前只支持收发基本文字消息。\n"
    buffersize = 2048
    prompt = '<You> '

    def __init__(self, host):
        super().__init__()
        self.__socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

        # Receive host socket like ('127.0.0.1', 8888)
        self.__host = host

        # User's nickname, updated when user logs in 
        self.__nickname = None

        # User is online or not
        self.__isonline = False


    def __receive_message_thread(self):
        """
        Serve as the thread of receiving message, launching when the user logs in.
        """
        while self.__isonline:
             try:
                buffer = self.__socket.recv(self.buffersize).decode()
                # Only for text message. Need updating if file transmission used.
                js = json.loads(buffer)
                if js['type'] == 'broadcast' or js['type'] == 'send':
                    print('<{0}>: {1}'.format(js['user'], js['content']))
                else:
                    '[Client] 无法从服务器获取信息'
            except:
                print('[Client] 无法从服务器获取信息')
    

    def __send_whisper_message_thread(self, who, message):
        """
        Serve as the thread of sending private message to one user, launching when stated.
        """
        if self.__isonline:
            try:
                self.__socket.sendto('broadcast {0} {1}'.format(who, message).encode(), self.__host)
                buffer = self.__socket.recv(self.buffersize).decode()
                js = json.loads(buffer)
                if js['type'] == 'send':
                    print('<You> tell <{0}>: {1}'.format(js['user'], js['content']))
                else:
                    print('[Client] 信息发送失败！')
            except:
                print('[Client] 信息发送失败！')



    def __send_broadcast_message_thread(self, message):
        """
        Serve as the thread of sending broadcast message to all users, launching when stated.
        """
        if self.__isonline:
            try:
                self.__socket.sendto('broadcast {0}'.format(message).encode(), self.__host)
                buffer = self.__socket.recv(self.buffersize).decode()
                js = json.loads(buffer)
                if js['type'] == 'broadcast':
                    print('<You>: {0}'.format(js['content']))
                else:
                    print('[Client] 信息发送失败！')
            except:
                print('[Client] 信息发送失败！')

    def do_help(self, *args):
        """
        Print help index to stdout.
        """
        command = args.split(' ')[0]
        if command == '':
            print('[Help] login {nickname} - 登录到聊天室，{nickname}是你选择的昵称'\n)
            print('[Help] send {message} - 发送消息，{message}是你输入的消息\n')
            print('[Help] sendto {who} {message} - 私发消息，{who}是用户名，{message}是你输入的消息\n')
            print('[Help] exit - 退出登录状态')
            #print('[Help] catusers - 查看所有用户')
            #print('[Help] catip who - 查看用户IP，who为用户名')
            #print('[Help] sendfile who filedir - 向某用户发送文件，who为用户名，filedir为文件路径')
            #print('[Help] getfile filename who yes/no - 接收文件，filename 为文件名,who为发送者，yes/no为是否接收')
        elif command == 'login' or command == 'log':
            print('[Help] login {nickname} - 登录到聊天室，{nickname}是你选择的昵称\n')
        elif command == 'send':
            print('[Help] send {message} - 发送消息，{message}是你输入的消息\n')
        elif command == 'sendto':
            print('[Help] sendto {who} {message} - 发送私发消息，{message}是你输入的消息\n')
        else:
            print('[Help] 没有查询到你想要了解的指令,请尝试直接按下回车或输入"login"查看登录方法\n')


    def do_login(self, *args):
        """
        log in the server.
        """
        try:
            nickname = args.split(' ')[0]
            passwd = args.split(' ')[1]
        except:
            print("[Client] 检查你的login格式是否有误！输入'login'以查看帮助")

        try:
            # Send request to server
            self.__socket.sendto(
            "login {0} {1}".format(nickname, passwd).encode(),
            self.__host
            )
            print('[Client] 正尝试与服务器连接...')

            # Try to receive JSON from server
            # Receive Json
            buffer = self.__socket.recv(self.buffersize).decode()
            obj = json.loads(buffer)
            if obj['status'] == 'success':
                self.__nickname = nickname
                self.__isonline = True
                print('[Client] 已成功连接！您是第{0}位注册的用户，昵称为{1}。'.format(obj['uid'], obj['user']))
            else:
                print('[Client] 连接失败...')
        except:
            print('[Client] 发生了未知错误，连接失败...')


    def do_send(self, *args):
        """
        Send message to all online users.
        """
        if self.__nickname == None:
            print("[Client] 请先登录！输入'login'以获取帮助")
            return None
        else:
            message = args
            thread = threading.Thread(target=self.__send_broadcast_message_thread, args=(message,))
            thread.setDaemon(True)
            thread.start()
            

    
    def do_sendto(send, *args):
        """
        Send message to an specific user.
        """
        if self.__nickname == None:
            print("[Client] 请先登录！输入'login'以获取帮助")
            return None
        else:
            who = args.split()[0]
            message = args.split()[1]
            thread = threading.Thread(target=self.__send_whisper_message_thread, args=(who, message))
            thread.setDaemon(True)
            thread.start()

    