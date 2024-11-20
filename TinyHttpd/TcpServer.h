#pragma once
#include <iostream>
#include <functional>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <deque>
#include <vector>

using namespace std;

namespace Tinyhttpd
{
    using OnMessgeCallback = function<void(void *user, const char *dat, size_t size, int connfd)>;
    using NewConnectCallback = function<void(void *user, int connfd)>;
    using DisconnectCallback = function<void(void *user, int connfd)>;

    struct TcpClients
    {
        int connfd;
        struct sockaddr_in connaddr;
    };
    

    class TcpServer
    {
        

    public:
        TcpServer(const TcpServer&) = delete;
        TcpServer& operator=(const TcpServer&) = delete;

        TcpServer(const string ip, unsigned port);
        ~TcpServer();

        void setOnMessgeCallback(const OnMessgeCallback &cb) { onMessgeCallback_ = cb; };
        void setNewConnectCallback(const NewConnectCallback &cb) { newConnectCallback_ = cb; };
        void setDisconnectCallback(const DisconnectCallback &cb) { disconnectCallback_ = cb; };

        bool run(void);
        int setNonBlocking(int fd);

        void addFdToEpoll(int fd, bool enableET);
        void handleConnection(bool enableET);
        void handleRead(int fd);

    private:
        static void worker(void *arg);
        static void recver(void *arg);
        
        
    private:
        int listenFd_;
        bool isStop_;
        pthread_t work_pthread_;
        pthread_t recv_pthread_;
        string ip_;
        unsigned port_;
        OnMessgeCallback onMessgeCallback_;
        NewConnectCallback newConnectCallback_;
        DisconnectCallback  disconnectCallback_;
        std::deque<struct TcpClients *> clients_;

        int epollFd_;
        const int MAX_EVENTS = 100;
        // std::vector<epoll_event> events_(100);
    };

}


