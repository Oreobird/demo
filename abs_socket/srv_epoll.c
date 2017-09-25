#if 0

static int epoll_handle_connect(int epollfd, struct epoll_event *ev)
{
    int ret = -1;
    struct sockaddr_in addr;
    unsigned int addr_len = sizeof(addr);
    int conn_sock = accept(sock_fd, (struct sockaddr *)&addr, &addr_len);
    if (conn_sock == -1)
    {
        dbg("accept failed\n");
        return -1;
    }

    set_socket_nonblock(conn_sock);
    ev->events = EPOLLIN | EPOLLET;
    ev->data.fd = conn_sock;
    ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock, ev);
    if (ret == -1)
    {
        dbg("epoll_ctl: conn_sock\n");
        return -1;
    }
    return 0;
}

static int epoll_read(struct epoll_event *ev, struct epoll_event *event, char *buf, int len)
{
    if (event->data.fd >= 0)
    {
        int sockfd = event->data.fd;
        int data_len = 0;
        data_len = read(sockfd, buf, len);
        if (data_len < 0)
        {
            dbg("read data failed\n");
            close(sockfd);
            event->data.fd = -1;
        }
        else if (data_len == 0)
        {
            dbg("client disconnted\n");
            close(sockfd);
            event->data.fd = -1;
        }
        buf[data_len] = '\0';
        dbg("recv data: %s\n", buf);
        ev->data.fd = sockfd;
        ev->events = EPOLLOUT|EPOLLET;
    }
}
static int epoll_write(struct epoll_event *ev, struct epoll_event *event, char *buf, int len)
{
    int data_len = -1;
    int sockfd = event->data.fd;
    data_len = write(sockfd, buf, MIN(strlen(buf), len));
    if (data_len < 0)
    {
        dbg("write data failed\n");
        close(sockfd);
        event->data.fd = -1;
    }

    dbg("send data: %s\n", buf);

    ev->data.fd = sockfd;
    ev->events = EPOLLIN | EPOLLET;
    return 0;
}
static int epoll_handle_data(int epollfd, struct epoll_event *ev, struct epoll_event *event, char *buf, int len)
{
    int ret = -1;

    if (ev->events & EPOLLIN)
    {
        ret = epoll_read(ev, event, buf, len);
    }
    else if (ev->events & EPOLLOUT)
    {
        ret = epoll_write(ev, event, buf, len);
    }

    epoll_ctl(epollfd, EPOLL_CTL_MOD, event->data.fd, ev);
    return ret;
}

static int epoll_loop(void)
{
    struct epoll_event ev, events[MAX_CLIENT];
    int i, nfds, epollfd = -1;
    int ret = -1;
    char buf[1500] = {0};

    epollfd = epoll_create1(0);
    if (epollfd == -1)
    {
        dbg("epoll_create1 failed\n");
        return -1;
    }

    ev.events = EPOLLIN;
    ev.data.fd = sock_fd;
    ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, sock_fd, &ev);
    if (ret == -1)
    {
        dbg("epoll_ctl failed\n");
        return -1;
    }

    while (1)
    {
        nfds = epoll_wait(epollfd, events, MAX_CLIENT, -1);
        if (nfds == -1)
        {
            dbg("epoll_wait\n");
            return -1;
        }

        for (i = 0; i < nfds; i++)
        {
            if (events[i].data.fd == sock_fd)
            {
                epoll_handle_connect(epollfd, &ev);
            }
            else
            {
                epoll_handle_data(epollfd, &ev, &events[i], buf, sizeof(buf));
            }
        }
    }
    return 0;
}

#endif
