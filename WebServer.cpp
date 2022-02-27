//
// Created by 吴中奇 on 2022/1/24.
//

#include <cassert>
#include "WebServer.h"


bool WebServer::init() {
    http::userCount.store(0);
    threadPool = std::make_unique<ThreadPool>(8);
    if (port > 65535 || port < 1024) {
        printf("Port error!\n");
        return false;
    }
    struct sockaddr_in addressServer{};
    listenFd = socket(PF_INET, SOCK_STREAM, 0);
    if (listenFd < 0) {
        printf("socket error!\n");
        return false;
    }

    bzero(&addressServer, sizeof(addressServer));
    addressServer.sin_family = AF_INET;
    addressServer.sin_port = htons(port);
    addressServer.sin_addr.s_addr = htonl(INADDR_ANY);

    int ret = bind(listenFd, (struct sockaddr *) &addressServer, sizeof(addressServer));
    if (ret == -1) {
        printf("bind error!\n");
        close(listenFd);
        return false;
    }
    ret = listen(listenFd, 5);
    if (ret == -1) {
        printf("listen error!\n");
        close(listenFd);
        return false;
    }

    epollFd = epoll_create(1);
    if (epollFd == -1) {
        printf("Epoll create error!\n");
        close(listenFd);
        return false;
    }
    epoll_event ev = {0};
    ev.data.fd = listenFd;
    ev.events = EPOLLIN;
    ret = epoll_ctl(epollFd, EPOLL_CTL_ADD, listenFd, &ev);
    if (ret != 0) {
        printf("Add listen error");
        close(listenFd);
        return false;
    }
    return true;
//----
/*
    while (true) {
        struct sockaddr_in addressClient;
        int sockClient;
        socklen_t lenAddressClient = sizeof(addressClient);

        sockClient = accept(listenFd,
                            (struct sockaddr *) &addressClient, &lenAddressClient);
        printf("new client ip:%s, port:%d\n", inet_ntoa(addressClient.sin_addr), ntohs(addressClient.sin_port));
        if (sockClient == -1) {
            printf("Accept error!\n");
            break;
        }
        http clientHttp;
        clientHttp.init(sockClient);
    }

    close(listenFd);
    */
}

inline void WebServer::showError(int fd, const char *info) {
    assert(fd > 0);
    printf("%s\n", info);
    if (send(fd, info, strlen(info), 0) < 0) {
        printf("send error to client%d error\n", fd);
    }
    close(fd);
}

void WebServer::dealListen() {
    struct sockaddr_in addrClient{};
    socklen_t lenAddrClient = sizeof(addrClient);

    int connFd = accept(listenFd, (struct sockaddr *) &addrClient, &lenAddrClient);
    epoll_event ev={0};
    ev.data.fd=connFd;
    ev.events=EPOLLIN|EPOLLET|EPOLLRDHUP;
    epoll_ctl(listenFd,EPOLL_CTL_ADD,connFd,&ev);
    if (connFd <= 0) {
        printf("Errno is %d\n",errno);
        return;
    } else if (http::userCount >= MAX_FD) {
        showError(connFd, "Server busy");
        return;
    }
    users[connFd].init(connFd, addrClient);
}

inline void WebServer::closeConn(http *client) {
    client->closeClient();
}

inline void WebServer::onRead(http *client) {
    int readErrno=0;
    if (client->read_once()) {
        client->process();
    } else {
        closeConn(client);
    }

}

inline void WebServer::dealRead(http *client) {
    threadPool->append(std::bind(&WebServer::onRead, client));
}

inline void WebServer::dealWrite(http *client) {
    threadPool->append(std::bind(&WebServer::onWrite, client));
}

inline void WebServer::onWrite(http *client) {
    if (client->write()) {
        client->closeClient();
    }
}

void WebServer::start() {

    while (true) {
        int number = epoll_wait(epollFd, events, MAX_EVENT_NUMBER, -1);
        if ((number < 0) && (errno != EINTR)) {
            printf("epoll failure\n");
            break;
        }
        printf("number of events:%d\n",number);
        for (int i = 0; i < number; ++i) {
            int sockFd = events[i].data.fd;

            printf("server %d,client %d\n",listenFd,sockFd);
            if (sockFd == listenFd) {
                //新的客户连接
                dealListen();
            } else if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                //服务器关闭客户连接
                closeConn(&users[sockFd]);
            } else if (events[i].events & EPOLLIN) {
                //读
                dealRead(&users[sockFd]);
            } else if (events[i].events & EPOLLOUT) {
                //写
                dealWrite(&users[sockFd]);
            }
        }
    }
}