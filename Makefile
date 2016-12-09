################################################################################
# Makefile                                                                     #
#                                                                              #
# Description: This file contains the make rules for the http server.          #
#                                                                              #
# Author: Shiyu Zhang <1181856726@qq.com>                                      #
#                                                                              #
################################################################################

LDFLAGS=-L/usr/local/opt/openssl/lib
CFLAGS=-I/usr/local/opt/openssl/include
GCC=gcc $(CFLAGS) $(LDFLAGS)

default: http_server

http_server:
	@$(GCC) daemonize.c log.c socket.c ssl.c util.c core.c cgi.c http.c http_server.c -o http_server -Wall -Werror -lssl -lcrypto

clean:
	@rm http_server
