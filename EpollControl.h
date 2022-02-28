//
// Created by zhongqi on 28.02.2022.
//

#ifndef WEBSERVER_EPOLLCONTROL_H
#define WEBSERVER_EPOLLCONTROL_H


#include <vector>
#include <sys/epoll.h>
#include <unistd.h>

class EpollControl {
public:
    static const int MAX_EVENT_NUMBER = 1000;

    explicit EpollControl() : epollFd(epoll_create(1)) {}

    ~EpollControl();

    bool addFd(int fd,bool oneShot);

    bool delFd(int fd);

    bool modFd(int fd,int events);

private:
    int epollFd;
    epoll_event events[MAX_EVENT_NUMBER];
};


#endif //WEBSERVER_EPOLLCONTROL_H
