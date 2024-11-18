#include "TcpServer.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <iostream>

using namespace std;
using namespace Tinyhttpd::net;

int main(int argc, char **argv)
{

    TcpServer server("0.0.0.0", 6666);
    server.run();
}
