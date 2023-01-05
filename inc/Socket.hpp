#ifndef SOCKET_HPP
# define SOCKET_HPP

# include <map>
# include "Server.hpp"
# include "dirent.h"

class Config;
class Socket
{
    public:
        Socket( std::string config, std::string path );
        ~Socket();

		Config			*serverConfig;
        std::string		ipAddr;
        int				port;
        int				fd;
        sockaddr_in		socketAddr;
        unsigned int	socketAddrLen;
        std::string		logFile;
        std::string		currentFile;
		bool			bound;
        int             setupSockets();
        std::string     getLocationPage( std::string ) const;
		std::string		getRedirectPage( std::string ) const;
		Config			*getConfig( std::string ) const;
		void			setRouteConfigs( std::string );
		
		std::map< std::string, Config* >	routes; /* location - config */

    private:
                	Socket( const Socket & );
        Socket &	operator=( const Socket & );

};

#endif
