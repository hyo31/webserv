#include "../inc/setupSocket.hpp"

int ft_return(std::string str, int ret)
{
    std::cerr << str;
    return (ret);
}

int main()
{
    setupSocket server("localhost", 8080, "port8080.log");
    
    return (0);
}
