#ifndef SOCKET_HPP
# define SOCKET_HPP

# include "Server.hpp"

class Socket
{
    public:
                        Socket(std::string ipAddr, int port);
                        ~Socket();

        std::string     ipAddr;
        int             port;
        int             fd;
        sockaddr_in     socketAddr;
        unsigned int    socketAddrLen;
        std::string     logFile;
        std::fstream    logfile_fstream;
                        Socket(const Socket &);
        Socket &        operator=(const Socket &);
        int             startServer();
        int             setupSockets();
};

#endif
