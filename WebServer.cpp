//
// Created by 吴中奇 on 2022/1/24.
//

#include <cassert>
#include "WebServer.h"

std::unique_ptr<EpollControl> WebServer::ep_ctl = std::make_unique<EpollControl>();

bool WebServer::init() {
    http::userCount.store(0);
    threadPool = std::make_unique<ThreadPool>(8);
    openLinger = true;
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

    //优雅关闭
    struct linger optLinger = {0};
    if (openLinger) {
        //close时阻塞直到超时或数据发送完毕
        optLinger.l_linger = 1;
        optLinger.l_onoff = 1;
    }
    if (setsockopt(listenFd, SOL_SOCKET, SO_LINGER,
                   &optLinger, sizeof(optLinger)) < 0) {
        close(listenFd);
        printf("Init linger error\n");
        return false;
    }

    //端口复用
    int opt = 1;
    if (setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR,
                   (const void *) &opt, sizeof(opt)) < 0) {
        close(listenFd);
        printf("reuseaddr error!\n");
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
    ep_ctl->addFd(listenFd, false);
    setNonBlock(listenFd);
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

    while (true) {
        int connFd = accept(listenFd, (struct sockaddr *) &addrClient, &lenAddrClient);
        ep_ctl->addFd(connFd, true);
        if (connFd < 0) {
            return;
        } else if (http::userCount >= MAX_FD) {
            showError(connFd, "Server busy");
            return;
        }
        setNonBlock(connFd);
        users[connFd].init(connFd, addrClient);
    }
}

inline void WebServer::closeConn(http *client) {
    ep_ctl->delFd(client->getFd());
    client->closeClient();
}

inline void WebServer::onProcess(http *client) {
    if (client->process()) {
        ep_ctl->modFd(client->getFd(), EPOLLOUT);
    } else {
        ep_ctl->modFd(client->getFd(), EPOLLIN);
    }
}

inline void WebServer::onRead(http *client) {
    if (client->read()) {
        onProcess(client);
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
    int ret = client->write();
    if (client->bytesToWrite() == 0) {
        //传输完毕
        if (client->getKeepAlive()) {
            client->init();
            onProcess(client);
            return;
        }
    } else if (ret < 0) {
        return;
    }
    closeConn(client);
}

inline int WebServer::setNonBlock(int fd) {
    assert(fd > 0);
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}

void WebServer::start() {

    while (true) {
        int number = ep_ctl->wait(-1);
        if ((number < 0) && (errno != EINTR)) {
            printf("epoll failure\n");
            break;
        }
        for (int i = 0; i < number; ++i) {
            int sockFd = ep_ctl->getEventDataFd(i);

            if (sockFd == listenFd) {
                //新的客户连接
//                printf("listen\n");
                dealListen();
            } else if (ep_ctl->getEvent(i) & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                //服务器关闭客户连接
//                printf("close\n");
                closeConn(&users[sockFd]);
            } else if (ep_ctl->getEvent(i) & EPOLLIN) {
                //读
//                printf("read\n");
                dealRead(&users[sockFd]);
            } else if (ep_ctl->getEvent(i) & EPOLLOUT) {
                //写
//                printf("write\n");
                dealWrite(&users[sockFd]);
            }
        }
    }
    close(listenFd);
}