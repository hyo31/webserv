#ifndef SERVER_HPP
# define SERVER_HPP

# include <iostream>
# include <unistd.h>
# include <vector>
# include <iomanip>
# include <fstream>
# include <limits>
# include <map>
# include <sstream>
# include <filesystem>
# include <netinet/in.h>
# include <iterator>
# include <sys/socket.h>
# include <sys/event.h>
# include <errno.h>
# include <sys/types.h>
# include <sys/time.h>
# include <fcntl.h>
# include <unistd.h>
# include <sys/wait.h>
# include "Client.hpp"
# include "Config.hpp"
# include "Socket.hpp"

# ifndef TIMEOUT
#  define TIMEOUT 60
# endif

class Config;
class Client;
class Socket;
class Server
{
    private:
        Server(const Server &);
        Server &	operator=(const Server &);
        
        std::string	findHtmlFile(int);
		std::string	getDirectoryListedPage(std::string);
        int			monitor_ports();
        int			acceptRequest(int);
        int			receiveClientRequest(int);
        int			sendResponseToClient(int);
        int			closeConnection(int);
        int			is_connection_open(int);
        void		set_chlist(std::vector<struct kevent>&, uintptr_t, int16_t, uint16_t, uint32_t, intptr_t, void *);
        void		update_client_timestamp(int);
        void		bounceTimedOutClients();
        void		chunkedRequest(std::string, std::vector<Client*>::iterator);
        int			Configuration(std::string);
		void		unchunk(std::string, std::string::size_type, std::vector<Client*>::iterator);
		void		buildBodyForContentLength(std::string, std::string::size_type, std::vector<Client*>::iterator);
        int			configuration(std::string);
        int			findSocket(int);
		int			checkMaxClientBodySize(std::vector<Client*>::iterator);
		std::string createAutoIndex(std::string, std::string);

        std::vector<Socket*>		_sockets;
        std::vector<Client*>		_clients;
        std::string					_responseHeader;
        std::string            	 	_path;
		struct timespec				_timeout;

    public:
			Server();
			~Server();
	int		startServer(std::string configFilePath, std::string path);

};

int	ft_return(std::string str);
#endif