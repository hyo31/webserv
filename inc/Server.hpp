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

class Server
{
    private:
        Server(const Server &);
        Server &    operator=(const Server &);
        class       Socket {
            private:
                Socket(const Socket &);
                Socket &            operator=(const Socket &);
            public:
                std::string         _ipAddr;
                int                 _port;
                int                 _socket;
                int                 _accept;
                struct sockaddr_in  _socketAddr;
                unsigned int         _socketAddrLen;
                std::string         _logFile;
                std::ofstream       _logfile_ostream;
                                    Socket(std::string ipAddr, int port, std::string logFile);
                                    ~Socket();
                int                 acceptSocket();
                int                 setupSockets();
                int                 receive_from_client(int);
                int                 respond_to_client();


        };
        std::vector<Socket*>        _sockets;
    public:
        Server();
        ~Server();
        int startServer();
        int monitor_fd();

};
int ft_return(std::string str);

#endif