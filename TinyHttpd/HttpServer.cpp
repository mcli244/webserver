#include "HttpServer.h"

using namespace std;
using namespace Tinyhttpd;

void print_sockaddr_in(const struct sockaddr_in *addr, const char *description) {
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip));
    printf("%s: IP=%s, Port=%d\n", description, ip, ntohs(addr->sin_port));
}

void onMessage(void *user, const char *dat, size_t size, int connfd)
{
    struct sockaddr_in peer_addr = {0};
    socklen_t  addr_len = sizeof(peer_addr);
    HttpServer *httpServer = (HttpServer *)user;

    if (getpeername(connfd, (struct sockaddr *)&peer_addr, &addr_len) == 0) {
        print_sockaddr_in(&peer_addr, "onMessage");
    } else {
        perror("getpeername failed");
    }

    printf("fd:%d dat[%ld]:%s\r\n", connfd, size, dat);

    httpServer->response(connfd, (char *)"Hello!", 200);
    //write(connfd, dat, size);
}

void newClient(void *user, int connfd)
{
    struct sockaddr_in peer_addr = {0};
    socklen_t  addr_len = sizeof(peer_addr);

    if (getpeername(connfd, (struct sockaddr *)&peer_addr, &addr_len) == 0) {
        print_sockaddr_in(&peer_addr, "newClient");
    } else {
        perror("getpeername failed");
    }
}

void disConectClient(void *user, int connfd)
{
    struct sockaddr_in peer_addr = {0};
    socklen_t  addr_len = sizeof(peer_addr);

    if (getpeername(connfd, (struct sockaddr *)&peer_addr, &addr_len) == 0) {
        print_sockaddr_in(&peer_addr, "disConectClient");
    } else {
        perror("getpeername failed");
    }
}

HttpServer::HttpServer(std::string _ip, int _port)
{
    tcpServer = new TcpServer(_ip, _port);

    tcpServer->setOnMessgeCallback(std::bind(onMessage, this, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    tcpServer->setNewConnectCallback(newClient);
    tcpServer->setDisconnectCallback(disConectClient);
};


HttpServer::~HttpServer()
{
    delete tcpServer;
};

void HttpServer::response(int connfd, char *message, int status)
{
    char buf[512];
    sprintf(buf, "HTTP/1.1 %d OK\r\nConnection: Close\r\n"
    "content-length:%ld\r\n\r\n", status, strlen(message));

    sprintf(buf, "%s%s", buf, message);
    write(connfd, buf, strlen(buf));
}

void HttpServer::response(int connfd, int size, int status)
{
    char buf[128];
    sprintf(buf, "HTTP/1.1 %d OK\r\nConnection: Close\r\n"
    "content-length:%d\r\n\r\n", status, size);
    write(connfd, buf, strlen(buf));
}

void HttpServer::run(void)
{
    tcpServer->run();
}
