#!/usr/bin/python

import os
import socket
import time

HOST = '127.0.0.1'
PORT = 9999

case = []

f = open('cp2_case.txt', 'r')
content = f.read()
pos0 = content.find('{')
while pos0 >= 0:
	pos1 = content.find('}', pos0 + 1)
	case.append(content[pos0 + 1 : pos1].replace('\n', '\r\n'))
	pos0 = content.find('{', pos1 + 1)
f.close()

test_case = {}
test_case['1']  = [0]
test_case['2']  = [1, 2, 3, 4, 5]
test_case['3']  = [6, 7, 8]
test_case['4']  = [9, 10, 11]
test_case['5']  = [12]
test_case['6']  = [13]
test_case['7']  = [14, 15, 16, 17, 18]
test_case['8']  = [19, 20, 21, 22, 23, 24, 25, 26]
test_case['9']  = [27, 28]
test_case['10'] = [29]
test_case['11'] = [30, 31, 32]
test_case['12'] = [33, 34, 35]
test_case['13'] = [36]
test_case['14'] = [37]
test_case['15'] = [38, 39]
test_case['16'] = [40, 41]

while True:
	print 'Test case:'
	print '1 : test any SP/CRLF before Request-Line'
	print '2 : test request method syntax'
	print '3 : test http version syntax'
	print '4 : test Request-Line lack some'
	print '5 : test any SP in Request-Line'
	print '6 : test no header'
	print '7 : test inproper CRLF in header'
	print '8 : test important field in header'
	print '9 : test header too long'
	print '10: test message body too long'
	print '11: test parse across buffer'
	print '12: test persistent connection'
	print '13: test HEAD'
	print '14: test GET'
	print '15: test POST'
	print '16: test resource'
	print '17: test too many clients'
	print '18: close'
	choice = raw_input('Input choice: ')

	if choice == '18':
		break
	elif choice == '17':
		os.system('python test_too_many_clients.py %s %d 1018' % (HOST, PORT))
	else:
		sock = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
		sock.connect((HOST, PORT))

		for i in test_case[choice]:
			sock.sendall(case[i])
			time.sleep(1)

		sock.close()