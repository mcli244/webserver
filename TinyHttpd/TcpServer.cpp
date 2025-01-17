#include "TcpServer.h"

#include <thread>
#include <iostream>

using namespace Tinyhttpd;
using namespace std;

// 设置非阻塞模式
int TcpServer::setNonBlocking(int fd) 
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void TcpServer::addFdToEpoll(int fd, bool enableET) {
    epoll_event event;
    int ret = -1;
    event.data.fd = fd;
    event.events = EPOLLIN;
    if (enableET) {
        event.events |= EPOLLET;
    }
    ret = epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &event);
    if(ret < -1)
    {
        std::cerr << "epoll_ctl failed: " << strerror(errno) << std::endl;
    }
}

TcpServer::TcpServer(const string ip, unsigned port)
    :isStop_(false)
{
   	sockaddr_in servaddr;
    //设置服务端的sockaddr_in
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = inet_addr(ip.c_str());

    printf("Server: %s:%d\r\n", ip.c_str(), port);

    listenFd_ = socket(AF_INET, SOCK_STREAM, 0);
    if(listenFd_ < 0)
    {
        std::cerr << "socket failed: " << strerror(errno) << std::endl;
        return;
    }

    setNonBlocking(listenFd_);
    int ret = bind(listenFd_, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if(ret < 0)
    {
        std::cerr << "bind failed: " << strerror(errno) << std::endl;
        return;
    }
    ret = listen(listenFd_, 10);
    if(ret < 0)
    {
        std::cerr << "listen failed: " << strerror(errno) << std::endl;
        return;
    }

    epollFd_ = epoll_create1(0);
    if (epollFd_ == -1) {
        std::cerr << "Epoll creation failed: " << strerror(errno) << std::endl;
        close(listenFd_);
        return ;
    }

    addFdToEpoll(listenFd_, true);
}

TcpServer::~TcpServer()
{
    close(listenFd_);
    close(epollFd_);
}

void TcpServer::handleConnection(bool enableET) {
    sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    int clientFd = accept(listenFd_, (sockaddr*)&clientAddr, &clientAddrLen);
    if (clientFd == -1) {
        std::cerr << "Accept failed: " << strerror(errno) << std::endl;
        return;
    }

    // std::cout << "New connection: " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << std::endl;
    setNonBlocking(clientFd);
    addFdToEpoll(clientFd, enableET);
    if(newConnectCallback_)
        newConnectCallback_(NULL, clientFd);
}

// 处理读事件
void TcpServer::handleRead(int fd) {
    char buffer[65535];
    while (true) {
        ssize_t bytesRead = read(fd, buffer, sizeof(buffer));
        if (bytesRead == 0) {
            // std::cout << "Client disconnected, fd: " << fd << std::endl;
            if(disconnectCallback_)
                disconnectCallback_(NULL, fd);
            close(fd);
            break;
        } else if (bytesRead < 0) {
            if (errno == EAGAIN) {
                break;
            }
            std::cerr << "Read error: " << strerror(errno) << std::endl;
            close(fd);
            break;
        } else {
            // std::cout << "Received: " << std::string(buffer, bytesRead) << std::endl;
            
            if(onMessgeCallback_)
                onMessgeCallback_(NULL, (const char *)&buffer, bytesRead, fd);
        }
    }
}

bool TcpServer::run(void)
{
    std::vector<epoll_event> events_(MAX_EVENTS);
    while(!isStop_)
    {
        std::cout << "epoll_wait..." <<std::endl;
        int eventCount = epoll_wait(epollFd_, events_.data(), MAX_EVENTS, -1);
        if (eventCount == -1) {
            std::cerr << "Epoll wait error: " << strerror(errno) << std::endl;
            break;
        }

        for (int i = 0; i < eventCount; ++i) {
            int fd = events_[i].data.fd;
            if (fd == listenFd_) {
                handleConnection(true);
            } else if (events_[i].events & EPOLLIN) {
                handleRead(fd);
            }
        }
    }
}