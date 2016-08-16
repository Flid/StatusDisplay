import socket 
from random import randint

PORT = 80
# Set your device IP here
HOST = '192.168.11.36'

TYPE_SEND_COLORS = 1

def send_raw_data(data):
	sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	sock.connect((HOST, PORT))
	
	sock.send(data)
	
	data = sock.recv(1000)
	sock.close()


def set_colors():
	msg = bytearray(1+3*8)
	msg[0] = TYPE_SEND_COLORS
	for i in range(8):
		msg[i*3 + 1] = randint(0, 256)  # red
		msg[i*3 + 2] = randint(0, 256)  # green
		msg[i*3 + 3] = randint(0, 256)  # blue
	
	send_raw_data(msg)	


set_colors()
