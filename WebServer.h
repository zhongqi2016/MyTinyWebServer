//
// Created by 吴中奇 on 2022/1/24.
//

#ifndef WEBSERVER_WEBSERVER_H
#define WEBSERVER_WEBSERVER_H

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#include <sys/epoll.h>
#include <unordered_map>
#include "http.h"
#include "ThreadPool.h"
#include "EpollControl.h"

class WebServer {
public:
//    static const int MAX_EVENT_NUMBER = 1000;
    static const int MAX_FD = 65535;

    explicit WebServer(int _port) : port(_port) {}

    bool init();

    void start();

private:
    void dealListen();

    static void showError(int fd, const char *info);

    void dealRead(http *client);

    void dealWrite(http *client);

    static void closeConn(http *client);

    static void onProcess(http *client);

    static void onRead(http *client);

    static void onWrite(http *client);

    static int setNonBlock(int fd);

    int port;
    int listenFd;
    int epollFd;
    bool openLinger;
//    epoll_event events[MAX_EVENT_NUMBER];
    std::unordered_map<int, http> users;
    std::unique_ptr<ThreadPool> threadPool;
    static std::unique_ptr<EpollControl> ep_ctl;
};


#endif //WEBSERVER_WEBSERVER_H
