//
// Created by 吴中奇 on 2022/1/26.
//

#include <iostream>
#include "http.h"

std::atomic<int> http::userCount;

void http::init(int _sockfd, sockaddr_in &_address) {
    sockfd = _sockfd;
    address = _address;
    memset(readBuffer, '\0', READ_BUFFER_SIZE);
    read_index = 0;
    write_index = 0;
    checked_index = 0;
    start_line = 0;
    isClose = false;
    keepAlive = false;
    ++userCount;

//    read();
//    checkState = CHECK_STATE_REQUESTLINE;
//    process();
//    write();

}

void http::closeClient() {
    if (!isClose) {
        sockfd=-1;
        isClose = true;
        --userCount;
        close(sockfd);
    }
}

bool http::read() {
    if (read_index >= READ_BUFFER_SIZE) {
        return false;
    }
    int dataRead = 0;
    memset(readBuffer,'\0',READ_BUFFER_SIZE);
    //LT read

//    dataRead = recv(sockfd, readBuffer + read_index, READ_BUFFER_SIZE - read_index, 0);
//    read_index += dataRead;
//    if (dataRead == -1) {
//        printf("read failed,fd=%d, errno=%d\n", sockfd, errno);
//        return false;
//    } else if (dataRead == 0) {
//        printf("client has closed the connection\n");
//        return false;
//    }

    //ET read
    while (true) {
        dataRead = recv(sockfd, readBuffer + read_index, READ_BUFFER_SIZE - read_index, 0);
        if (dataRead < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }
            return false;
        } else if (dataRead == 0) {
            printf("client has closed the connection\n");
            return false;
        }
        read_index += dataRead;
    }
    return true;
}


//从状态机，解析第一行内容
http::LINE_STATUS http::parse_line() {
    char temp;

    for (; checked_index < read_index; ++checked_index) {
        temp = readBuffer[checked_index];
        if (temp == '\r') {
            //如果\r是最后一个字符，说明没有读到完整的行，返回LINE_open表示还需要继续读取
            if ((checked_index + 1) == read_index) {
                return LINE_OPEN;
            }
                //以\n结尾，说明读到了完整的行
            else if (readBuffer[checked_index + 1] == '\n') {
                readBuffer[checked_index++] = '\0';
                readBuffer[checked_index++] = '\0';
                return LINE_OK;
            }
            //否则说明请求有语法问题
            return LINE_BAD;
        } else if (temp == '\n') {
            if (checked_index > 1 && readBuffer[checked_index - 1] == '\r') {
                readBuffer[checked_index - 1] = '\0';
                readBuffer[checked_index++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
    }
    //所有内容读完没找到\r，说明还需继续读取
    return LINE_OPEN;
}

//分析请求行
http::HTTP_CODE http::parse_requestline(char *temp) {
    //如果请求行没有空格或"\t"则肯定有问题
    url = strpbrk(temp, " \t");
    if (!url) {
        return BAD_REQUEST;
    }
    *url++ = '\0';

    method = temp;
    if (strcasecmp(method, "GET") == 0) {
        printf("Request is GET\n");
    } else if (strcasecmp(method, "POST") == 0) {
        printf("Request is POST\n");
    } else {
        return BAD_REQUEST;
    }

    //返回 str1 中第一个不在字符串 str2 中出现的字符下标
    url += strspn(url, " \t");
    version = strpbrk(url, " \t");
    if (!version) {
        return BAD_REQUEST;
    }
    *version++ = '\0';
    version += strspn(version, " \t");

    //仅HTTP1.1
    if (strcasecmp(version, "HTTP/1.1") != 0) {
        printf("version!=HTTP/1.1\n");
        return BAD_REQUEST;
    }
    if (strncasecmp(url, "http://", 7) == 0) {
        url += 7;
        url = strchr(url, '/');
        //url指向/
    }
    if (!url || url[0] != '/') {
        printf("bad url\n");
        return BAD_REQUEST;
    }

    checkState = CHECK_STATE_HEADER;
    return NO_REQUEST;
}

//分析头部字段
http::HTTP_CODE http::parse_headers(const char *temp) {
    //如果遇到一个空行，说明我们的到了一个正确的HTTP请求
    if (temp[0] == '\0') {
        return GET_REQUEST;
    } else if (strncasecmp(temp, "Connection:", 11) == 0) {
        //比较前n个字符
        temp += 11;
        temp += strspn(temp, " \t");
        if (strncasecmp(temp, "keep-alive", 10)) {
            keepAlive = true;
        }
    } else {
//        printf("Can't handle this header\n");
    }
    return NO_REQUEST;
}

http::HTTP_CODE http::processRead() {
    LINE_STATUS lineStatus = LINE_OK;
    HTTP_CODE httpCode = NO_REQUEST;
    checkState = CHECK_STATE_REQUESTLINE;
    while ((lineStatus = parse_line()) == LINE_OK) {
        char *temp = readBuffer + start_line;
        start_line = checked_index;
        switch (checkState) {
            case CHECK_STATE_REQUESTLINE: {
                httpCode = parse_requestline(temp);
                if (httpCode == BAD_REQUEST) {
                    printf("Bad request line\n");
                    return BAD_REQUEST;
                }
                break;
            }
            case CHECK_STATE_HEADER: {
                httpCode = parse_headers(temp);
                if (httpCode == BAD_REQUEST) {
                    return BAD_REQUEST;
                } else if (httpCode == GET_REQUEST) {
                    return do_request();
                }
                break;
            }
            default: {
                return INTERNAL_ERROR;
            }
        }
    }
    if (lineStatus == LINE_OPEN) {
        return NO_REQUEST;
    } else {
        return BAD_REQUEST;
    }
}

http::HTTP_CODE http::do_request() {
    char *pathOfFile = (char *) malloc(strlen(pathOfFile) + strlen(url) + 1);
    strcpy(pathOfFile, srcDir);
    if (strcmp(url, "/") == 0) {
        strcat(pathOfFile, "/test.html");
    } else {
        strcat(pathOfFile, url);
    }

    //通过stat请求资源文件信息，成功则将信息存储到fileStat
    if (stat(pathOfFile, &fileStat) < 0) {
        printf("找不到资源\n");
        return NO_RESOURCE;
    }
    //若资源文件不可读
    if (!(fileStat.st_mode & S_IROTH)) {
        printf("资源文件不可读\n");
        return FORBIDDEN_REQUEST;
    }
    //若文件类型为目录
    if (S_ISDIR(fileStat.st_mode)) {
        printf("文件类型为目录\n");
        return BAD_REQUEST;
    }

    //只读方式获取文件描述符，用mmap映射到内存
    int fileFd = open(pathOfFile, O_RDONLY);
    fileAddress = (char *) mmap(nullptr, fileStat.st_size, PROT_READ, MAP_PRIVATE, fileFd, 0);
    close(fileFd);
    free(pathOfFile);
    return GET_REQUEST;
}

bool http::addResponse(const char *format, ...) {
    if (write_index >= WRITE_BUFFER_SIZE) {
        return false;
    }
    va_list arg_list;
    va_start(arg_list, format);

    //将数据format从可变参数列表写入缓冲区写，返回写入数据的长度
    int len = vsnprintf(writeBuffer + write_index, WRITE_BUFFER_SIZE - 1 - write_index, format, arg_list);
    if (len >= (WRITE_BUFFER_SIZE - 1 - write_index)) {
        va_end(arg_list);
        return false;
    }
    write_index += len;
    va_end(arg_list);
    return true;
}

inline bool http::addStatusLine(int status, const char *title) {
    return addResponse("%s %d %s\r\n", "HTTP/1.1", status, title);
}

inline bool http::addHeaders(size_t content_len) {
    return addContentLength(content_len) && addLinger() &&
           addBlankLine();
}

inline bool http::addContentLength(size_t content_len) {
    return addResponse("Content-Length:%d\r\n", content_len);
}

inline bool http::addContentType() {
    return addResponse("Content-Type:%s\r\n", "text/html");
}

inline bool http::addLinger() {
    return addResponse("Connection:%s\r\n", keepAlive ? "keep-alive" : "close");
}

inline bool http::addBlankLine() {
    return addResponse("%s", "\r\n");
}

inline bool http::addContent(const char *content) {
    return addResponse("%s", content);
}

bool http::process_write(HTTP_CODE ret) {
    switch (ret) {
        case FORBIDDEN_REQUEST: {
            addStatusLine(403, "forbidden request");
            const char *str403("Don't have permission of file\n");
            addHeaders(strlen(str403));
            if (!addContent(str403)) {
                return false;
            }
            break;
        }
        case BAD_REQUEST: {
            addStatusLine(400, "bad request");
            const char *str400("Bad request\n");
            addHeaders(strlen(str400));
            if (!addContent(str400)) {
                return false;
            }
            break;
        }
        case NO_RESOURCE: {
            addStatusLine(404, "No resource");
            const char *str404("No resource\n");
            addHeaders(strlen(str404));
            if (!addContent(str404)) {
                return false;
            }
            break;
        }
        case GET_REQUEST: {
            addStatusLine(200, "OK");
            if (fileStat.st_size != 0) {
                addHeaders(fileStat.st_size);
                //第一个iovec指向响应报文缓冲区,长度指向write_index;
                m_iv[0].iov_base = writeBuffer;
                m_iv[0].iov_len = write_index;
                //第二个iovec指针指向mmap返回的文件指针
                m_iv[1].iov_base = fileAddress;
                m_iv[1].iov_len = fileStat.st_size;
                count_iv = 2;
//                bytes_to_send = write_index + fileStat.st_size;
                return true;
            } else {
                const char *ok_string = "<html><body></body></html>";
                addHeaders((strlen(ok_string)));
                if (!addContent(ok_string)) {
                    return false;
                }
            }
            break;
        }
        default: {
            return false;
        }
    }
    //指向响应报文缓冲区,长度指向write_index;
    m_iv[0].iov_base = writeBuffer;
    m_iv[0].iov_len = write_index;
    count_iv = 1;
    return true;
}


bool http::write() {
    ssize_t len = -1;
    bool flag = false;
    while (true) {
        len = writev(sockfd, m_iv, count_iv);

        if (len <= 0) {
            flag = false;
//            std::cout <<"errno"<< errno << std::endl;
            break;
        }
        if (m_iv[0].iov_len + m_iv[1].iov_len <= 0) {
            flag = true;
            break;
        }

        //若m_iv[0]已发送完，发送第二个数据
        if (len >= m_iv[0].iov_len) {
            m_iv[1].iov_base = (uint8_t *) m_iv[1].iov_base + (len - m_iv[0].iov_len);
            m_iv[1].iov_len -= (len - m_iv[0].iov_len);
            m_iv[0].iov_len = 0;
        } else {
            //否则继续发送第一个
            m_iv[0].iov_base = (uint8_t *) m_iv[0].iov_base + len;
            m_iv[0].iov_len -= len;
        }

    }

    //取消映射
    if (fileAddress) {
        munmap(fileAddress, fileStat.st_size);
        fileAddress = 0;
    }
    return flag;
}

bool http::process() {
    HTTP_CODE resRead = processRead();
    if (resRead == NO_REQUEST) {
        return false;
    }
    bool resWrite = process_write(resRead);
    if (!resWrite) {
        closeClient();
    }
    return write();
}
