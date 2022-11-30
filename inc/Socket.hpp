#ifndef SOCKET_HPP
# define SOCKET_HPP

#include "map"
# include "Server.hpp"
# include "dirent.h"

class Socket
{
    public:
        Socket(std::string config, std::string path);
        Socket(std::string ipAddr, int port);
        ~Socket();

		bool    					autoindex;
		std::string					config;
		std::vector<std::string>	methods;
		int							maxClientBodySize;
        std::string					ipAddr;
        std::string                 root;
        int							port;
        int							fd;
        sockaddr_in					socketAddr;
        unsigned int				socketAddrLen;
        std::string					logFile;
                        			Socket(const Socket &);
        Socket &        			operator=(const Socket &);
        int             			startServer();
        int             			setupSockets();
        std::string     			getLocationPage(std::string page);
		std::string					getRedirectPage(std::string page);

    private:
        std::map<std::string, std::string>	_pages; /* name - location */
		std::map<std::string, std::string>	_redirects; /* name - redirect_location */
};

#endif
