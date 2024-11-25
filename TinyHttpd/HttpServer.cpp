#include "HttpServer.h"
#include <sys/stat.h>
#include <sys/sendfile.h>

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

    HttpServer::http_req req;
    
    httpServer->getReq(dat, size, req);
    cout << "req.cmd: " << req.cmd << endl;
    if(req.cmd == "GET")
    {
        if(req.url.empty())
        {
            //httpServer->response(connfd, (char *)"Hello this is a tiny http server!", HttpServer::OK);
            req.url.append("submit.html");
            httpServer->responseFile(connfd, req.url);
        }
        else
        {
            cout << "file: " << req.url << endl;
            httpServer->responseFile(connfd, req.url);
        }
    }
    else if(req.cmd == "POST")
    {
        if(req.url == "login")
        {
            //httpServer->response(connfd, (char *)"Hello this is a tiny http server!", HttpServer::OK);
            cout << "login" << endl;
            cout << "req.body" << req.body << endl;
            req.url.clear();
            req.url.append("submit.html");
            httpServer->responseFile(connfd, req.url);
        }
        else
        {
            cout << "file: " << req.url << endl;
            httpServer->response(connfd, (char *)"NotFound", HttpServer::NotFound);
        }
    }

    
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

    // HttpServer::http_req req;
    // char *dat = "GET /1234?test=123&pwd=aaaa HTTP/1.1 ";
    // int size = strlen(dat);
    // getReq(dat, size, req);
};


HttpServer::~HttpServer()
{
    delete tcpServer;
};

bool HttpServer::getReq(const char *dat, size_t size, http_req &req)
{
    std::string msg(dat);
    std::string::size_type start = 0, index = 0;

    for(int i=0; i<3; i++)
    {
        index = msg.find(' ', start);
        //cout << "index: " << index << " start:" << start << "msg: " << msg.substr(start, index - start )<< endl;
        if(index == std::string::npos)
        {
            cout << "find end! "
                << "start: " << start
                << "index: " << index
                << endl;
            return false;
        }
            
        if(i == 0)      req.cmd = msg.substr(start, index - start );
        else if(i == 1) 
        {
            std::string tmp = msg.substr(start + 1, index - start - 1);
            //cout << "tmp: " << tmp << endl;
            if(tmp.empty())
            {
                req.url.clear();
            }
            else
            {
                index = tmp.find('?');
                if(index != std::string::npos)
                {
                    req.url = tmp.substr(0, index);
                    req.dynamic = tmp.substr(index + 1, tmp.length() - index - 1);
                    index += tmp.length() - 1;
                }
                else
                {
                    req.url = tmp;
                }
            }
            

            
        }
        else            req.version = msg.substr(start, index - start);
        start = index + 1;
    }

    // cout << "req.cmd: " << req.cmd 
    //     << "\nreq.url: " << req.url
    //     << "\nreq.dynamic: " << req.dynamic
    //     << "\nreq.version: " << req.version 
    //     << "\nreq.body" << req.body 
    //     << endl;

    return true;
}

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

void HttpServer::responseFile(int connfd, std::string &file)
{
    struct stat filestat;
    int ret = stat(file.c_str(), &filestat);
    if(ret < 0 || S_ISDIR(filestat.st_mode)) //file doesn't exits
	{
		char message[512];
		sprintf(message, "<html><title>Tinyhttpd Error</title>");
		sprintf(message, "%s<body>\r\n", message);
		sprintf(message, "%s 404\r\n", message);
		sprintf(message, "%s <p>GET: Can't find the file", message);
		sprintf(message, "%s<hr><h3>The Tiny Web Server<h3></body>", 
			message);
		response(connfd, message, 404);
		return;
	}

    char buf[128];
    sprintf(buf, "HTTP/1.1 %d OK\r\n"
    "content-length:%ld\r\n\r\n", HttpServer::Ecode::OK, filestat.st_size);
    write(connfd, buf, strlen(buf));
    int filefd = open(file.c_str(), O_RDONLY);
    sendfile(connfd, filefd, 0, filestat.st_size);
}

void HttpServer::run(void)
{
    if(tcpServer)
        tcpServer->run();
}
