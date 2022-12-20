#ifndef SERVER_HPP
# define SERVER_HPP

# include <iostream>
# include <vector>
# include <map>
# include <iomanip>
# include <fstream>
# include <limits>
# include <sstream>
# include <filesystem>
# include <iterator>
# include <unistd.h>
# include <errno.h>
# include <fcntl.h>
# include <signal.h>
# include <netinet/in.h>
# include <unistd.h>
# include <sys/event.h>
# include <sys/socket.h>
# include <sys/types.h>
# include <sys/time.h>
# include <sys/wait.h>
# include "Client.hpp"
# include "Config.hpp"
# include "Socket.hpp"

# ifndef TIMEOUT
#  define TIMEOUT 5
# endif

# ifndef MAX_BODY
#  define MAX_BODY INT_MAX
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
        int			monitor_ports();
        int			acceptRequest(int);
        int			receiveClientRequest(int);
        int			sendResponseToClient(int);
        int			closeConnection(int);
        void		set_chlist(std::vector<struct kevent>&, uintptr_t, int16_t, uint16_t, uint32_t, intptr_t, void *);
        void		update_client_timestamp(int);
        void		bounceTimedOutClients();
        void		parseRequest(std::string, std::vector<Client*>::iterator);
		void		unchunk(std::string, std::string::size_type, std::vector<Client*>::iterator);
		void		buildBodyForContentLength(std::string, std::string::size_type, std::vector<Client*>::iterator);
        int			openSockets(std::string);
        int			findSocket(int);
		int			checkMaxClientBodySize(std::vector<Client*>::iterator);
		std::string createAutoIndex(std::string, std::string);
		void		closeSockets();

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

int		ft_return(std::string str);

#endif