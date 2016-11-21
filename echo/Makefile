################################################################################
# Makefile                                                                     #
#                                                                              #
# Description: This file contains the make rules.                              #
#                                                                              #
# Author: Shiyu Zhang <1181856726@qq.com>                                     #
#                                                                              #
################################################################################

default: echo_server echo_client

echo_server:
	@gcc echo_server.c -o echo_server -Wall -Werror

echo_client:
	@gcc echo_client.c -o echo_client -Wall -Werror

clean:
	@rm echo_server echo_client
