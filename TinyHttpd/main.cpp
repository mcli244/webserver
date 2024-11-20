#include "HttpServer.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <iostream>

using namespace std;
using namespace Tinyhttpd;


int main(int argc, char **argv)
{
    if(argc < 2)
    {
        cout << "Usage:" << argv[0] << " [port]" << endl;
        return -1;
    }
    HttpServer server("0.0.0.0", atoi(argv[1]));
    server.run();
}
