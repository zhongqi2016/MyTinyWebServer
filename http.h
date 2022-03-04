//
// Created by 吴中奇 on 2022/1/26.
//

#ifndef WEBSERVER_HTTP_H
#define WEBSERVER_HTTP_H

#include <cstring>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <cstdarg>
#include <cerrno>
#include <atomic>


class http {
public:
    static const int READ_BUFFER_SIZE = 2048;
    static const int WRITE_BUFFER_SIZE = 1024;
    static constexpr const char *srcDir = "./files";
    static std::atomic<int> userCount;
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
    void init();
    void init(int _sockfd, sockaddr_in &_address);

    void closeClient();

    bool read();

    bool write();

    bool process();

    [[nodiscard]] int getFd() const { return sockfd; }

    [[nodiscard]] size_t bytesToWrite() const { return m_iv[0].iov_len + m_iv[1].iov_len; }

    [[nodiscard]] bool getKeepAlive() const { return keepAlive; }

private:

    HTTP_CODE processRead();

    LINE_STATUS parse_line();

    HTTP_CODE parse_requestline(char *temp);

    HTTP_CODE do_request();

    HTTP_CODE parse_headers(const char *temp);


    bool process_write(HTTP_CODE ret);

    bool addResponse(const char *format, ...);

    bool addStatusLine(int status, const char *title);

    bool addHeaders(size_t content_len);

    bool addContentLength(size_t content_len);

    bool addContentType();

    bool addLinger();

    bool addBlankLine();

    bool addContent(const char *content);


private:
    bool isClose;
    bool keepAlive;
    int sockfd;
    sockaddr_in address;
    int start_line;
    int checked_index;
    int read_index;
    int write_index;
    char *method;
    char *url;
    char *version;
    char *fileAddress;
    struct stat fileStat;
    char readBuffer[READ_BUFFER_SIZE];
    char writeBuffer[WRITE_BUFFER_SIZE];
    CHECK_STATE checkState;
    struct iovec m_iv[2];
    int count_iv;
};


#endif //WEBSERVER_HTTP_H
