    /*
    else if (!strcmp(cmd, "ipaddr"))
    {
        for (int i = 0; i <= maxi; i++)
        {
            if (client[i].fd != -1 && client[i].new_conn != -1)
            {
                //here sockfd == client[i].fd
                inet_ntop(AF_INET, &client[i].sockaddr.sin_addr, buf, MAXLINE);
                sprintf(send_buffer, "uid %d name %s ip addr %s:%d",
                        client[i].uid, users[client[i].uid].name,
                        buf, ntohs(client[i].sockaddr.sin_port));
                writen(client[connindex].fd, send_buffer,
                       strlen(send_buffer));
                usleep(1000); //ugly
            }
        }
    }
    */