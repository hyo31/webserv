#ifndef SOCKET_HPP
# define SOCKET_HPP

# include "map"
# include "Server.hpp"
# include "dirent.h"

class Config;
class Socket
{
    public:
        Socket(std::string config, std::string path);
        ~Socket();

		Config						*serverConfig;
        std::string					ipAddr;
        int							port;
        int							fd;
        sockaddr_in					socketAddr;
        unsigned int				socketAddrLen;
        std::string					logFile;
        int             			setupSockets();
        std::string     			getLocationPage(std::string page);
		std::string					getRedirectPage(std::string page);
		void						setRouteConfigs(std::string &);
		Config						*getConfig(std::string &);
		
		std::map<std::string, Config*>	routes; /* location - config */

    private:
                	Socket(const Socket &);
        Socket &	operator=(const Socket &);

};

#endif
