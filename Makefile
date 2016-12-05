################################################################################
# Makefile                                                                     #
#                                                                              #
# Description: This file contains the make rules for the http server.          #
#                                                                              #
# Author: Shiyu Zhang <1181856726@qq.com>                                      #
#                                                                              #
################################################################################

default: http_server

http_server:
	@gcc util.c log.c socket.c core.c http.c daemonize.c http_server.c -o http_server -Wall -Werror

clean:
	@rm http_server
