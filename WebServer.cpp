//
// Created by 吴中奇 on 2022/1/24.
//

#include "WebServer.h"


void WebServer::start() const {
    struct sockaddr_in addressServer;
    int sockServer = socket(PF_INET, SOCK_STREAM, 0);
    if (sockServer < 0) {
        printf("socket error!\n");
        exit(1);
    }

    bzero(&addressServer, sizeof(addressServer));
    addressServer.sin_family = AF_INET;
    addressServer.sin_port = htons(port);
    addressServer.sin_addr.s_addr = htonl(INADDR_ANY);

    int ret = bind(sockServer, (struct sockaddr *) &addressServer, sizeof(addressServer));
    if (ret == -1) {
        printf("bind error!\n");
        exit(1);
    }
    ret = listen(sockServer, 5);
    if (ret == -1) {
        printf("listen error!\n");
        exit(1);
    }

    while (true) {
        struct sockaddr_in addressClient;
        int sockClient;
        socklen_t lenAddressClient = sizeof(addressClient);

        sockClient = accept(sockServer,
                            (struct sockaddr *) &addressClient, &lenAddressClient);
        printf("new client ip:%s, port:%d\n", inet_ntoa(addressClient.sin_addr), ntohs(addressClient.sin_port));
        if (sockClient == -1) {
            printf("Accept error!\n");
            break;
        }
        http clientHttp;
        clientHttp.init(sockClient);
    }

    close(sockServer);
}