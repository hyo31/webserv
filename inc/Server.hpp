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

struct Client {
    int conn_fd;
    int socket;


};

class Connection;
class Socket;
class Server
{
    private:
        Server(const Server &);
        Server &                operator=(const Server &);
        std::vector<Socket*>    _sockets;
        std::map<int,int>       _conn_fd; /* key=con_fd, value=paired socket_num*/
        std::string             buildResponse(int c_fd);
        std::string             responseHeader;

    public:
        Server();
        ~Server();
        int     startServer();
        int     monitor_fd();
        int	    monitor_fd2();
        int     acceptRequest(int);
        int     receiveClientRequest(int);
        int     sendResponseToClient(int);
        int     closeConnection(int);
        int     is_connection_open(int);
        void    set_chlist(std::vector<struct kevent>&, uintptr_t, int16_t, uint16_t, uint32_t, intptr_t, void *);
};

int ft_return(std::string str);

#endif