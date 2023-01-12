#ifndef SERVER_HPP
# define SERVER_HPP

# include <iostream>
# include <vector>
# include <map>
# include <fstream>
# include <sstream>
# include <fcntl.h>
# include <netinet/in.h>
# include <unistd.h>
# include <sys/event.h>
# include <sys/stat.h>
# include "Client.hpp"
# include "Config.hpp"
# include "Socket.hpp"

# ifndef TIMEOUT
#  define TIMEOUT 7
# endif

# ifndef MAX_ARGLEN_EXECVE
#  define MAX_ARGLEN_EXECVE 262144
# endif

# ifndef MAX_BODY
#  define MAX_BODY MAX_ARGLEN_EXECVE
# endif

enum {
	ERROR = -1,
	STOP_READ = 1,
	CONT_READ = 2
};

class Config;
class Client;
class Socket;
class Server
{
    private:
        Server( const Server & );
        Server &	operator=( const Server & );

		std::string createAutoIndex( std::string, std::string );
		Client 		*findClient( int);
        Client		*acceptRequest( int );
        int			monitor_ports();
        int			receiveClientRequest( Client*, std::string& );
        int			sendResponseToClient( Client* );
        void		closeConnection( Client* );
        int			openSockets( std::string );
        int			findSocket( int );
        void		set_chlist( std::vector< struct kevent >&, uintptr_t, int16_t, uint16_t, uint32_t, intptr_t, void * );
        void		bounceTimedOutClients();
        void		parseRequest( std::string, Client * );
		void		closeSockets();
		std::string	methodDELETE( Client*, Config* );
		std::string	methodGET( Client*, Config* );
		std::string	methodPOST( Client*, Config* );
		std::string	getHtmlFile( Client* );

        std::vector<Socket*>	_sockets;
        std::vector<Client*>	_clients;
        std::string				_responseHeader;
        std::string				_path;
		struct timespec			_timeout;

    public:
			Server();
			~Server();
	int		startServer( std::string configFilePath, std::string path );

};

int							printerror( std::string );
int							executeCGI( std::string, int, std::string, std::string, std::string, std::string, std::string, std::string );
bool						BinaryFile( std::string );
void						SaveBinaryFile( std::string, Client *, Config * );
std::vector<std::string>	readFile( std::string header, std::string body );
std::string					createResponseHtml( void );
#endif
