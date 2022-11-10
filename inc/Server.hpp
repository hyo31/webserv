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
# include <map>

class Connection;
class Socket;
class Server
{
    private:
        Server(const Server &);
        Server &                operator=(const Server &);
        std::vector<Socket*>    _sockets;
        std::map<int,int>       _conn_fd; /* key=fd, value=socket_num*/
        std::string             writeResponse(int c_fd);
        std::string             responseHeader;

    public:
        Server();
        ~Server();
        int     startServer();
        int     monitor_fd();
        int     acceptRequest(int);
        int     receiveClientRequest(int);
        int     respondToClient(int);
        int     closeConnection(int);
        bool    open_connection(int);
};

int ft_return(std::string str);

#endif