#pragma once
#include "TcpServer.h"

namespace Tinyhttpd
{
    class HttpServer
    {
        public:
        enum Ecode{
            OK = 200,
            MovedPermanently = 301,
            MoveTemporarily = 302,
            Forbidden = 403,
            NotFound = 404,
            MethodNotAllowed = 405,
            InternalServerError = 500,
            GatewayTimeout = 504,
            ECODE_MAX
        };

        struct http_req
        {
            std::string cmd;
            std::string url;
            std::string dynamic;
            std::string version;
            std::string header;
            std::string body;
        };

        public:
            HttpServer(std::string _ip, int _port);
            ~HttpServer();

            bool getReq(const char *dat, size_t size, http_req &req);
            void response(int connfd, char *message, int status);
            void response(int connfd, int size, int status);
            void responseFile(int connfd, std::string &file);
            void run(void);
            
        private:
            TcpServer *tcpServer = NULL;

    };
}
