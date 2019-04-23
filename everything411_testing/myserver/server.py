#!/usr/bin/env python
# -*- coding: utf-8 -*-
# originally written by BigCat(Nya@git.bitnp.net)
# PYTQL
# modified version

import socket
import select
import json

s_addr = ('0.0.0.0', 9877)
server = socket.socket()
server.bind(s_addr)
server.listen(5)

readable = []
writable = []
exceptional = []

inputs = [server]
outputs = []

while True:
    readable, writable, exceptional = select.select(inputs, outputs, inputs)
    for i in readable:
        # print(inputs)
        if i is server:
            client, c_addr = i.accept()
            print('User {0} Connected.'.format(c_addr))
            inputs.append(client)
        else:
            try:
                data = i.recv(4096)
                is_disconnect = not data
            except socket.error:
                is_disconnect = True
            if is_disconnect:
                print('User {0} Disconnected...'.format(i.getpeername()))
                inputs.remove(client)
            else:
                json_data = {"type": "broadcast", "content": data.decode()}
                for i in inputs:
                    if i is not server:
                        send_data = json.dumps(json_data)
                        i.sendall(send_data.encode())
