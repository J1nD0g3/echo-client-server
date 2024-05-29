#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef __linux__
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#endif // __linux
#ifdef WIN32
#include <winsock2.h>
#include "../mingw_net.h"
#endif // WIN32
#include <iostream>
#include <thread>

#ifdef WIN32
void myerror(const char* msg) { fprintf(stderr, "%s %lu\n", msg, GetLastError()); }
#else
void myerror(const char* msg) { fprintf(stderr, "%s %s %d\n", msg, strerror(errno), errno); }
#endif

void usage() {
    printf("syntax: echo-client <ip> <port>\n");
    printf("sample: echo-client 127.0.0.1 1234\n");
}

struct Param {
    uint32_t ip{0};
    uint16_t port{0};

    bool parse(int argc, char* argv[]){
        for (int i = 1; i < argc; i++) {
            int res = inet_pton(AF_INET, argv[i++], &ip);
            switch (res) {
                case 1: break;
                case 0: fprintf(stderr, "not a valid network address\n"); return false;
                case -1: myerror("inet_pton"); return false;
            }
            port = atoi(argv[i++]);
        }
        return (ip != 0) && (port != 0);
    }
}param;

void recvThread(int sd) {
    printf("connected\n");
    fflush(stdout);
    static const int BUFSIZE = 65536;
    char buf[BUFSIZE];
    while (true) {
        ssize_t res = ::recv(sd, buf, BUFSIZE - 1, 0);
        if (res == 0 || res == -1) {
            fprintf(stderr, "recv return %zd", res);
            myerror(" ");
            break;
        }
        buf[res] = '\0';
        printf("%s", buf);
        fflush(stdout);
    }
    printf("disconnected\n");
    fflush(stdout);
    ::close(sd);
    exit(0);
}

int main(int argc, char* argv[]){
    if (!param.parse(argc, argv)) {
        usage();
        return -1;
    }

#ifdef WIN32
    WSAData wsaData;
    WSAStartup(0x0202, &wsaData);
#endif // WIN32

    int sd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sd == -1) {
        perror("socket");
        return -1;
    }


#ifdef __linux__
    //
    // setsockopt
    //
    {
        int optval = 1;
        int res = ::setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
        if (res == -1) {
            myerror("setsockopt");
            return -1;
        }
    }
#endif // __linux

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(param.port);
    addr.sin_addr.s_addr = param.ip;
    memset(&addr.sin_zero, 0, sizeof(addr.sin_zero));


    //
    // connect
    //
    {
        int res = ::connect(sd, (struct sockaddr *)&addr, sizeof(addr));
        if (res == -1) {
            myerror("connect");
            return -1;
        }
    }

    std::thread t(recvThread, sd);
    t.detach();

    while (true) {
        std::string s;
        std::getline(std::cin, s);
        s += "\r\n";
        ssize_t res = ::send(sd, s.data(), s.size(), 0);
        if (res == 0 || res == -1) {
            fprintf(stderr, "send return %zd", res);
            myerror(" ");
            break;
        }
    }
    ::close(sd);

}
