//
// Created by 吴中奇 on 2022/1/26.
//

#ifndef WEBSERVER_HTTP_H
#define WEBSERVER_HTTP_H

#include <string.h>
#include <stdio.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <cstdarg>
#include <errno.h>


class http {
public:
    static const int READ_BUFFER_SIZE = 2048;
    static const int WRITE_BUFFER_SIZE = 1024;
    const char* srcDir="./files";
    enum CHECK_STATE {
        CHECK_STATE_REQUESTLINE = 0, CHECK_STATE_HEADER
    };
    enum LINE_STATUS {
        LINE_OK = 0, LINE_BAD, LINE_OPEN
    };
    enum HTTP_CODE {
        NO_REQUEST = 0,
        FILE_REQUEST,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURCE,
        FORBIDDEN_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION
    };
public:
    void init(int _sockfd);

private:
    bool read_once();

    HTTP_CODE parse_content();

    LINE_STATUS parse_line();

    HTTP_CODE parse_requestline(char *temp, CHECK_STATE &checkState);

    HTTP_CODE do_request();

    void parsePath();

    HTTP_CODE parse_headers(char *temp);

    void process();

    bool process_write(HTTP_CODE ret);

    void serve_static();

    bool addResponse(const char *format, ...);

    bool addStatusLine(int status, const char *title);

    bool addHeaders(int content_len);

    bool addContentLength(int content_len);

    bool addContentType();

    bool addLinger();

    bool addBlankLine();

    bool addContent(const char *content);

    bool write();


private:
    bool m_linger;
    int sockfd;
    int start_line;
    int checked_index;
    int read_index;
    int write_index;

    int bytes_to_send;
    int bytes_have_send;

    char *method;
    char *url;
    char *version;
    char *fileAddress;
    struct stat fileStat;
    sockaddr_in address;
    char readBuffer[READ_BUFFER_SIZE];
    char writeBuffer[WRITE_BUFFER_SIZE];
    CHECK_STATE checkState;
    struct iovec m_iv[2];
    int count_iv;
};


#endif //WEBSERVER_HTTP_H
