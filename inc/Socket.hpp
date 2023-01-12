#ifndef SOCKET_HPP
# define SOCKET_HPP

# include "Server.hpp"
# include "dirent.h"

class Config;
class Socket
{
    public:
        Socket( std::string config, std::string path );
        ~Socket();

		// Config			*serverConfig;
        std::string		ipAddr;
        int				fd;
		int				port;
        sockaddr_in		socketAddr;
        unsigned int	socketAddrLen;
        std::string		logFile;
        std::string		currentFile;
		bool			bound;
        int             setupSockets();
        std::string     getLocationPage( std::string, std::string ) const;
		std::string		getRedirectPage( std::string, std::string ) const;
		Config			*getConfig( std::string, std::string ) const;
		void			setRouteConfigs( std::string );
		void			setPortLogHost( std::string );

		std::map< std::string, std::vector< Config* > >	hostConfigs;/* host - configs */
		std::vector< std::string >						hosts;
		// std::map< std::string, Config* >				routes; 	/* location - config */

    private:
                    Socket() {};
                	Socket( const Socket & );
        Socket &	operator=( const Socket & );

};

#endif
