//
// Created by zhongqi on 28.02.2022.
//

#include "EpollControl.h"


EpollControl::~EpollControl() {
    close(epollFd);
}

 int EpollControl::wait(int timeout) {
    return epoll_wait(epollFd,events,MAX_EVENT_NUMBER,timeout);
}

 bool EpollControl::addFd(int fd) const {
    if (fd < 0) return false;
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = EPOLLIN | EPOLL_CTL_ADD | EPOLLRDHUP;
    return epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &ev) == 0;
}

 bool EpollControl::delFd(int fd) {
    if (fd < 0) return false;
    epoll_event ev = {0};
    return epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, &ev) == 0;
}

 bool EpollControl::modFd(int fd, int events) {
    if (fd < 0) return false;
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    return epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &ev) == 0;
}

 int EpollControl::getEventDataFd(int i) const {
    return events[i].data.fd;
}
 uint32_t EpollControl::getEvent(int i) const {
    return events[i].events;
}



