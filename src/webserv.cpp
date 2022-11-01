#include "../inc/Server.hpp"

//to-do
//catch / throw for errors ?
//spamming refresh on one port leads to the second request from a second port to hang

int main()
{
    Server webserv;
    webserv.startServer();
    return (0);
}
