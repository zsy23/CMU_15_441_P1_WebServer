/******************************************************************************
* echo_server.c                                                               *
*                                                                             *
* Description: This file contains the C source code for an echo server.  The  *
*              server runs on a hard-coded port and simply write back anything*
*              sent to it by connected clients.                               * 
*              It supports concurrent clients with select.                    *
*                                                                             *
* Author: Shiyu Zhang <1181856726@qq.com>                                     *
*                                                                             *
*******************************************************************************/

#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>

#define ECHO_PORT 9999
#define BUF_SIZE 4096

int close_socket(int sock)
{
    if (close(sock))
    {
        fprintf(stderr, "Failed closing socket: %s\n", strerror(errno));
        return 1;
    }
    return 0;
}

int main(int argc, char* argv[])
{
    int sock, client_sock;
    int maxi, maxfd;
    int i, nready;
    fd_set allset, rset;
    int client[FD_SETSIZE];
    ssize_t readret;
    socklen_t cli_size;
    struct sockaddr_in addr, cli_addr;
    char buf[BUF_SIZE];
    
    /* all networked programs must create a socket */
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
    {
        fprintf(stderr, "Failed creating socket: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(ECHO_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    /* servers bind sockets to ports---notify the OS they accept connections */
    if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)))
    {
        close_socket(sock);
        fprintf(stderr, "Failed binding socket: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    if (listen(sock, FD_SETSIZE))
    {
        close_socket(sock);
        fprintf(stderr, "Error listening on socket: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    /* configure select */
    for (i = 0; i < FD_SETSIZE; ++i) client[i] = -1;
    maxi = -1;
    maxfd = sock;
    FD_ZERO(&allset);
    FD_SET(sock, &allset);    

    /* finally, loop waiting for input and then write it back */
    while (1)
    {
        rset = allset;
        nready = select(maxfd + 1, &rset, NULL, NULL, NULL);

        if (FD_ISSET(sock, &rset))
        {
            cli_size = sizeof(cli_addr);
            if ((client_sock = accept(sock, (struct sockaddr *) &cli_addr, &cli_size)) == -1)
                fprintf(stderr, "Error accepting connection: %s\n", strerror(errno));
            else
            {        
                fprintf(stdout, "Connected by %s:%u\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));

                /* workaround solution to situation where select() return while not actually be ready to read*/
                fcntl(client_sock, F_SETFL, O_NONBLOCK); 
              
                for (i = 0; i < FD_SETSIZE; ++i)
                {
                    if (client[i] == -1)
                    {
                        client[i] = client_sock;
                        break;
                    }
                }
                
                if (i == FD_SETSIZE)
                {
                    close_socket(client_sock);
                    fprintf(stderr, "Error too many clients\n");
                }
                else
                {
                    if (maxi < i) maxi = i;
                    if (maxfd < client_sock) maxfd = client_sock;
                    FD_SET(client_sock, &allset);
                }            
            }

            if (--nready <= 0) continue;
        }        
         
        for (i = 0; i <= maxi; ++i)
        {
            if (client[i] >= 0 && FD_ISSET(client[i], &rset))
            {
                if ((readret = recv(client[i], buf, BUF_SIZE, 0)) > 0)
                {
                    if (send(client[i], buf, readret, 0) != readret)
                        fprintf(stderr, "Error sending to client: %s\n", strerror(errno));
                    memset(buf, 0, BUF_SIZE);
                }                                       

                if (readret == -1)
                    fprintf(stderr, "Error reading from client socket: %s\n", strerror(errno));

                if (readret == 0)
                {
                    fprintf(stdout, "Close client's fd %d\n", client[i]);
                    close_socket(client[i]);
                    FD_CLR(client[i], &allset);
                    client[i] = -1;
                }
                if (--nready <= 0) break;
            }
        }
    }

    close_socket(sock);

    return EXIT_SUCCESS;
}
