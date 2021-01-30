#!/usr/bin/env python3

import socket, threading, json, argparse

def listen_on_s(ss, s):
    global BUFFER_SIZE
    while True:
        data = s.recv(BUFFER_SIZE)
        if not data:
            break
        ss.sendall(data)

def listen_on_ss(ss, s):
    global BUFFER_SIZE
    while True:
        data = ss.recv(BUFFER_SIZE)   
        if not data:
            break
        s.sendall(data)

parser = argparse.ArgumentParser()
parser.add_argument("--config", help="path to config file", default="config.json")
args = parser.parse_args()

config_file = open(args.config, 'r')
config_json = json.loads(config_file.read())

SERVER_HOST = config_json["server"]["host"]
SERVER_PORT = config_json["server"]["port"]
CLIENT_HOST = config_json["client"]["host"]
CLIENT_PORT = config_json["client"]["port"]
if "buffer_size" in config_json:
    BUFFER_SIZE = config_json["buffer_size"]
else:
    BUFFER_SIZE = 4096
print("======> [INFO] Config loaded")

config_file.close()

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as ss:
    ss.bind((SERVER_HOST, SERVER_PORT))
    ss.listen()
    print("======> [INFO] Server started")
    while True:
        conn, addr = ss.accept()
        print("======> [INFO] New connection")
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((CLIENT_HOST, CLIENT_PORT))
        s_thread = threading.Thread(target=listen_on_s, args=(conn, s), daemon=True)
        ss_thread = threading.Thread(target=listen_on_ss, args=(conn, s), daemon=True)
        s_thread.start()
        ss_thread.start()
