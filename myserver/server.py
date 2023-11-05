#!/usr/bin/env python
# -*- coding: utf-8 -*-
# originally written by BigCat(Nya@git.bitnp.net)
# PYTQL
# modified version

import socket
import select
import json
import sqlite3


class user:
    def __init__(self, name, passwd):
        self.name = name
        self.passwd = passwd


PORT = 9877
RECV_BUFFER = 4096
connection_list = []
user_list = []
user_dict = {}
client_dict = {}

server = socket.socket()
server.bind(('0.0.0.0', PORT))
server.listen()

connection_list.append(server)

print('Server Running On PORT {0}'.format(PORT))


def find_user_login(name, passwd):
    for i in user_list:
        if name == i.name and passwd == i.passwd:
            return i
    return None


def find_user_name(name):
    for i in user_list:
        if name == i.name:
            return i
    return None


def get_error(err):
    return json.dumps(
        {"type": "error", "status": "error", "value": err}).encode()


def userinit():
    userconf = open("user.conf")
    line_lis = userconf.readlines()
    for i in line_lis:
        __user = user(i.split()[0], i.split()[1])
        user_list.append(__user)
    userconf.close()


def userlist(sock):
    i.send(
        json.dumps({"type": "broadcast", "user": "SYSTEM",
                    "content": repr([i.name for i in user_list])}).encode())


def broadcast(sock, message):
    if sock not in user_dict:
        sock.send(get_error("ERRClientNotLogin"))
    else:
        global server, connection_list
        json_data = json.dumps(
            {"type": "broadcast", "user": user_dict[sock].name,
             "content": message.split(' ', 1)[1]}).encode()
        for i in connection_list:
            if i is not server:
                i.send(json_data)


def send(sock, message):
    message = message.split(' ', 2)
    if len(message) < 3:
        sock.send(get_error("ERRSyntax"))
    else:
        send_to = find_user_name(message[1])
        if send_to:
            if send_to not in client_dict:
                json_data = get_error("ERRUserNotOnline")
            else:
                json_data = json.dumps(
                    {"type": "message", "user": user_dict[sock].name, "content": message[2]}).encode()
                client_dict[send_to].send(json_data)
            if user_dict[sock] != send_to:
                sock.send(json_data)
        else:
            sock.send(get_error("ERRNoSuchUser"))


def useraddr(sock, message):
    message = message.split(' ', 1)
    if len(message) < 2:
        sock.send(get_error("ERRSyntax"))
    else:
        send_to = find_user_name(message[1])
        if send_to:
            if send_to not in client_dict:
                json_data = get_error("ERRUserNotOnline")
            else:
                json_data = json.dumps(
                    {"type": "useraddr", "user": user_dict[sock].name, "uid": -1,
                     "address": sock.getpeername()[0], "port": sock.getpeername()[1]}).encode()
            sock.send(json_data)
        else:
            sock.send(get_error("ERRNoSuchUser"))


def login(sock, message):
    global server, connection_list
    user_info = message.split()
    if len(user_info) == 3:
        conn_user = find_user_login(user_info[1], user_info[2])
        if not conn_user:
            json_data = get_error("ERRUser")
        else:
            if sock in user_dict:
                json_data = get_error("ERRClientHasLogin")
            elif conn_user in client_dict:
                json_data = get_error("ERRUserIsOnline")
            else:
                user_dict[sock] = conn_user
                client_dict[conn_user] = sock
                json_data = json.dumps(
                    {"type": "login", "user": user_info[1], "status": "success"}).encode()
    else:
        json_data = get_error("ERRSyntax")
    try:
        sock.send(json_data)
    except Exception as e:
        print(e)


if __name__ == "__main__":
    userinit()
    # print(repr(user_list))
    while True:
        try:
            readable, writable, exceptional = select.select(
                connection_list, [], [])
            for i in readable:
                if i is server:
                    try:
                        client, client_addr = i.accept()
                        connection_list.append(client)
                    except:
                        print('A user tried to connect but failed.')
                else:
                    try:
                        data = i.recv(RECV_BUFFER).decode()
                        if data:
                            if data.startswith("login"):
                                login(i, data)
                            elif data.startswith("broadcast"):
                                broadcast(i, data)
                            elif data.startswith("send"):
                                send(i, data)
                            elif data.startswith("userlist"):
                                userlist(i)
                            elif data.startswith("useraddr"):
                                useraddr(i, data)
                            else:
                                i.send(get_error("ERRSyntax").encode())
                        else:
                            raise ConnectionError("miaomiaomiao")
                    except ConnectionError as e:
                        print(e)
                        if i in user_dict:
                            client_dict.pop(user_dict[i])
                            user_dict.pop(i)
                        i.close()
                        connection_list.remove(i)
        except KeyboardInterrupt:
            server.close()
            exit()
