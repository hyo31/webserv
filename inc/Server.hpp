#ifndef SERVER_HPP
# define SERVER_HPP

# include <iostream>
# include <sys/socket.h>
# include <netinet/in.h>
# include <unistd.h>
# include <vector>
# include <iomanip>
# include <fstream>
# include <errno.h>
# include <sys/types.h>
# include <sys/event.h>
# include <sys/time.h>
# include <fcntl.h>
# include "Socket.hpp"

class Socket;
class Server
{
    private:
        Server(const Server &);
        Server &    operator=(const Server &);
        std::vector<Socket*>    _sockets;
   
    public:
        Server();
        ~Server();
        int startServer();
        int monitor_fd();

};

int ft_return(std::string str);

#endif