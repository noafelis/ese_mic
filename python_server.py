#!/usr/bin/env python3

#############################################################################
# adapted from https://pythontic.com/modules/socket/udp-client-server-example
#############################################################################

import sys
import socket
import selectors
import types
 
localIP = ''
localPort = 31717
bufferSize = 1024
 
# Create a datagram socket
UDPServerSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)

# Bind to address and ip
UDPServerSocket.bind((localIP, localPort))
print("UDP server up and listening")

# Listen for incoming datagrams
while(True):
	message, address = UDPServerSocket.recvfrom(bufferSize)
	adc_value = message.decode('utf-8', errors="ignore")
	print("Ambient noise: ", adc_value)
