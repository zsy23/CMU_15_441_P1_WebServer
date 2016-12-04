#!/usr/bin/python

from socket import *
import sys
import time

serverHost = gethostbyname(sys.argv[1])
serverPort = int(sys.argv[2])
numConnections = int(sys.argv[3])

socketList = []

for i in xrange(numConnections):
    s = socket(AF_INET, SOCK_STREAM)
    s.connect((serverHost, serverPort))
    socketList.append(s)

time.sleep(10)

for i in xrange(numConnections):
    socketList[i].close()
