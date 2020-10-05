#!/usr/bin/env python3

#############################################################################
# adapted from https://pythontic.com/modules/socket/udp-client-server-example
#############################################################################

import sys
import socket
import selectors
import types
 

#localIP  = "192.168.0.136"
localIP = ''

localPort = 31717

bufferSize = 1024

 

#msgFromServer       = "Hello UDP Client"

#bytesToSend         = str.encode(msgFromServer)

 

# Create a datagram socket

UDPServerSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)

 

# Bind to address and ip

UDPServerSocket.bind((localIP, localPort))

 

print("UDP server up and listening")

 

# Listen for incoming datagrams

while(True):

    message, address = UDPServerSocket.recvfrom(bufferSize)

    adc_value = message.decode('utf-8')

#    clientMsg = "Message from Client:{}".format(message)
#    clientIP  = "Client IP Address:{}".format(address)

    if(adc_value == '0'):
        print("No valid noise measurement available")
    else:
        print("Ambient noise: ", adc_value)
#    print(clientIP)

   

    # Sending a reply to client

#    UDPServerSocket.sendto(bytesToSend, address)
