#!/usr/bin/env python
# -*- coding: utf-8 -*-
# originally written by BigCat(Nya@git.bitnp.net)
# PYTQL
# modified version

import socket
import select
import json
import sqlite3

PORT = 9877
RECV_BUFFER = 4096
connection_list = []
conn_uid_list = []
user_list = []

server = socket.socket()
server.bind(('0.0.0.0', PORT))
server.listen()

connection_list.append(server)
conn_uid_list.append(-1)

print('Server Running On PORT {0}'.format(PORT))


def userinit(cursor):

    # cursor.execute('''CREATE TABLE USER
    #        (ID INT PRIMARY KEY     NOT NULL,
    #        ONLINE boolean      KEY     ,
    #        NAME            CHAR(50)    NOT NULL,
    #        PASSWORD        CHAR(50));''')

    # cursor.execute(
    #     "INSERT INTO USER (ID,NAME,PASSWORD) VALUES (1, 'test', 'test')")
    # cursor.execute(
    #     "INSERT INTO USER (ID,NAME,PASSWORD) VALUES (2, 'miao', 'miao')")
    # cursor.execute(
    #     "INSERT INTO USER (ID,NAME,PASSWORD) VALUES (3, 'weak', 'password')")
    # cursor.execute(
    #     "INSERT INTO USER (ID,NAME,PASSWORD) VALUES (4, 'bigcat', 'bigcat')")
    cursor.execute("update user set online = 0")
    # userconf = open("user.conf")
    # line_lis = userconf.readlines()
    # for i in line_lis:
    #     __user = user()
    #     __user.username = i.split()[0]
    #     __user.password = i.split()[1]
    #     user_list.append(__user)
    # userconf.close()


def broadcast(sock, message):
    global server, connection_list
    json_data = json.dumps(
        {"type": "broadcast", "user": "null", "content": message}).encode()
    for i in connection_list:
        if i is not server:
            try:
                i.send(json_data)
            except:
                i.close()
                connection_list.remove(i)


def login(sock, message):
    global server, connection_list
    json_data = json.dumps(
        {"type": "login", "user": "null", "status": "success"}).encode()
    for i in connection_list:
        if i is sock:
            try:
                i.send(json_data)
                return
            except:
                i.close()
                connection_list.remove(i)


if __name__ == "__main__":
    userdb = sqlite3.connect("user.db")
    dbcursor = userdb.cursor()
    userinit(dbcursor)
    userdb.commit()
    sel = dbcursor.execute("select * from user").fetchall()
    for i in sel:
        print(i)
    # exit()
    while True:
        readable, writable, exceptional = select.select(
            connection_list, [], [])
        # print("connection_list:")
        # print(connection_list)
        # print("readable_list:")
        # print(readable)
        for i in readable:
            if i is server:
                try:
                    client, client_addr = i.accept()
                    print('User {0} Entered the Chat Room.'.format(
                        client_addr))
                    broadcast(
                        client, 'User {0} Entered the Chat Room.'.format(client_addr))
                    connection_list.append(client)
                except:
                    print('A user tried to connect but failed.')
            else:
                try:
                    data = i.recv(RECV_BUFFER).decode()
                    if data:
                        if data.startswith("login"):
                            login(i, data)
                        else:
                            broadcast(i, data)
                    else:
                        raise Exception("miaomiaomiao")
                except:
                    print('User {0} is offline.'.format(i.getpeername()))
                    broadcast(server, 'User {0} is offline.'.format(
                        i.getpeername()))
                    i.close()
                    connection_list.remove(i)
                    continue
    userdb.close()
    server.close()
