#ifndef SOCKET_HPP
# define SOCKET_HPP

# include "Server.hpp"

class Socket
{
    public:
                        Socket(std::string ipAddr, int port, std::string logFile);
                        ~Socket();

        std::string     ipAddr;
        int             port;
        int             fd;
        sockaddr_in     socketAddr;
        unsigned int    socketAddrLen;
        std::string     logFile;
        std::ofstream   logfile_ostream;
                        Socket(const Socket &);
        Socket &        operator=(const Socket &);
        int             startServer();
        int             setupSockets();
};

#endif
