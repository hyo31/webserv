#include "../inc/Server.hpp"

//to-do
//catch / throw for errors ?
//spamming refresh on one port leads to the second request from a second port to hang

int main(int argc, char **argv)
{
    if (argc < 1 || argc > 2)
        return (ft_return("Wrong number of arguments: "));
    Server webserv;
    if (argc == 3)
        webserv.startServer(argv[2]);
    else
        webserv.startServer("config/default.conf");
    return (0);
}
