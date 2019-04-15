#include "srv_header.h"
#include "srv_error.h"

int main(int argc, char **argv)
{
    int i, maxi, maxfd, listenfd, connfd, sockfd;
    int nready;
    // client = malloc(FD_SETSIZE * sizeof(client_t));
    // users = malloc(FD_SETSIZE * sizeof(userdata_t));
    ssize_t n;
    fd_set rset, allset;
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    listen(listenfd, MAXCONN);

    maxfd = listenfd; /* initialize */
    maxi = -1;        /* index into client[] array */

    pset = &allset;

    /* -1 indicates available entry */
    memset(client, -1, FD_SETSIZE * sizeof(client_t));
    memset(users, -1, FD_SETSIZE * sizeof(userdata_t));

    userinit();

    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);

    for (;;)
    {
        rset = allset; /* structure assignment */
        nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
        // puts("select() returned");
        if (FD_ISSET(listenfd, &rset))
        {
            /* new client connection */
            clilen = sizeof(cliaddr);
            connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);

            for (i = 0; i < FD_SETSIZE; i++)
                if (client[i].fd < 0)
                {
                    client[i].fd = connfd; /* save descriptor */
                    client[i].sockaddr = cliaddr;

                    break;
                }
            if (i == FD_SETSIZE)
                fputs("too many clients\n", stderr);

            FD_SET(connfd, &allset); /* add new descriptor to set */
            if (connfd > maxfd)
                maxfd = connfd; /* for select */
            if (i > maxi)
                maxi = i; /* max index in client[] array */

            if (--nready <= 0)
                continue; /* no more readable descriptors */
        }

        for (i = 0; i <= maxi; i++)
        {
            /* check all clients for data */
            if ((sockfd = client[i].fd) < 0)
                continue;
            if (FD_ISSET(sockfd, &rset))
            {
                if ((n = read(sockfd, buf, MAXLINE)) == 0)
                {
                    /* connection closed by client */
                    close(sockfd);
                    FD_CLR(sockfd, &allset);
                    if (client[i].new_conn != -1)
                    {
                        sprintf(send_buffer, "{\n\"type\":\"exitinfo\",\n\"user\":\"%s\",\n\"uid\":%d\n}\n",
                                users[client[i].uid].name, client[i].uid);
                        for (int j = 0; j <= maxi; j++)
                            if (client[j].fd != -1 && client[j].fd != sockfd && client[j].new_conn != -1)
                                sendclient(j); /* broadcast exit infomation */
                    }
                    users[client[i].uid].offline = -1;
                    client[i].fd = -1;
                    client[i].uid = -1;
                    client[i].new_conn = -1;
                }
                else
                {
                    bufinit(buf, n);
                    // puts(buf); //debug
                    parse(buf, i, maxi);
                }

                if (--nready <= 0)
                    break; /* no more readable descriptors */
            }
        }
    }
}
