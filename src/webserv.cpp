#include "../inc/setupSocket.hpp"

int ft_return(std::string str, int ret)
{
    std::cerr << str;
    return (ret);
}

int main()
{
    setupSocket server("localhost", 8080, "port8080.log");
    setupSocket server1("localhost", 4242, "port8080.log");
    setupSocket server2("localhost", 9999, "port8080.log");

    server.acceptSocket();
    return (0);
}
