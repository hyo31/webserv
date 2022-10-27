#include "../inc/setupSocket.hpp"
#include "../inc/Server.hpp"

//to-do
//catch / throw for errors ?

int main()
{
    Server webserv;
    webserv.startServer();
    // setupSocket socket_0("localhost", 8080, "port8080.log");
    // setupSocket socket_1("localhost", 4242, "port4242.log");
    // setupSocket socket_2("localhost", 9999, "port9999.log");
    // //select
    // socket_0.acceptSocket();
    return (0);
}
