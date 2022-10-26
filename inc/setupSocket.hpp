#ifndef SETUPSOCKET_HPP
# define SETUPSOCKET_HPP

# include <iostream>
# include <sys/socket.h>
# include <netinet/in.h>
# include <unistd.h>
# include <vector>
# include <iomanip>
# include <fstream>
# include <errno.h>

class setupSocket
{
    private:
        std::string     _ipAddr;
        int             _port;
        int             _socket;
        int             _accept;
        sockaddr_in     _socketAddr;
        unsigned int    _socketAddrLen;
        std::ofstream   _logFile;
        
        int             startServer();
        int             ft_return(std::string str);

    public:
        setupSocket(std::string ipAddr, int port, std::string logFile);
        ~setupSocket();
};

#endif
