//
// Created by 吴中奇 on 2022/1/24.
//

#ifndef WEBSERVER_WEBSERVER_H
#define WEBSERVER_WEBSERVER_H
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <assert.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <thread>

#include "http.h"

class WebServer {
public:
    WebServer(int _port):port(_port){}
    void start() const;
private:
    int port;
};


#endif //WEBSERVER_WEBSERVER_H
