#pragma once
#include "TcpServer.h"

namespace Tinyhttpd
{
    class HttpServer
    {
        public:
            HttpServer(std::string _ip, int _port);
            ~HttpServer();

            void response(int connfd, char *message, int status);
            void response(int connfd, int size, int status);
            void run(void);
            
        private:
            TcpServer *tcpServer = NULL;

    };
}
