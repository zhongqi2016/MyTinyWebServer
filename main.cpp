#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <assert.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "WebServer.h"
static bool stop = false;

static void handle_term(int sig) {
    stop = true;
}


int main(int argc,char* argv[]) {
    WebServer myServ=WebServer(63344);
    myServ.start();
//    signal(SIGTERM,handle_term);
//
//    if(argc<=3){
//        printf("usage: ip_address port number backlog\n");
//        return 1;
//    }
//    const char * ip=argv[1];
//    int port=atoi(argv[2]);
//    int backlog=atoi(argv[3]);
//
//    int sock=socket(PF_INET,SOCK_STREAM,0);
//    assert(sock>=0);
//
//    //创建一个IPv4 socket地址
//    struct  sockaddr_in address;
//    bzero(&address, sizeof(address));
//    address.sin_family=AF_INET;
//    inet_pton(AF_INET,ip,&address.sin_addr);
//    address.sin_port= htons(port);
//
//    int ret=bind(sock,(struct sockaddr*)&address, sizeof(address));
//    assert(ret!=-1);
//    ret= listen(sock,backlog);
//    assert(ret!=-1);
//
//    while(!stop){
//        sleep(1);
//    }
//    close(sock);

    return 0;
}
