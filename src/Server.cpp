#include "../inc/Server.hpp"

Server::Server()
{
	this->_timeout.tv_sec = TIMEOUT;
	this->_timeout.tv_nsec = 0;
}
Server::Server(const Server &) {  }
Server &	Server::operator=(const Server &) { return *this; }
Server::~Server() { std::cout << "Closing server...\n"; }

//using kqueue() in order to monitor the ports
//then accepts this with socket::acceptSocket() when a client sends a request
int	Server::monitor_ports()
{
    int							i, sock_num, new_event, kq, fd, conn_fd, ret;
	Client						*client;
	std::vector<struct  kevent> chlist;         /* list of events to monitor */
	struct  kevent              tevents[42];	/* list of triggered events */
	std::string					request = "";
	intptr_t					data = 0;

    /* create the queue */
    kq = kqueue();  
    if ( kq == -1 )
        return printerror( "kqueue failed" );

    /* initialize kevent chlist structs - uses EVFILT_READ so it returns when there is data available to read */
    for ( size_t j = 0; j < this->_sockets.size(); ++j )
        set_chlist( chlist, this->_sockets[j]->fd, EVFILT_READ, EV_ADD, data, 0, NULL );

    /* enter run loop */
    while( 1 ) 
    {
    	std::cout << "\033[1m--waiting for events...--\n\033[0m";
        /* use kevent to wait for an event (when a client tries to connect or when a connection has data to read/is open to receive data) */
        new_event = kevent( kq, &chlist[0], chlist.size(), tevents, 42, &this->_timeout );
        chlist.clear();
        if ( new_event < 0 )
            return printerror( "kevent failed: \n" );
        /* kevent returned with new events */
        if ( new_event > 0 )
        {
            for ( i = 0; i < new_event; i++ )
            {
                fd = ( int )tevents[i].ident;
				client = findClient( fd );
                std::cout << "handling event:" << i+1 << "/" << new_event <<  " on fd:" << fd << std::endl;
				/* EV_ERROR is set if kevent occured an error processing chlist */
				if ( tevents[i].flags & EV_ERROR )
				{
                	closeConnection( client );
					std::cout << "system error:" << data << std::endl;
                    return printerror( "kevent failed: \n" );
				}
                /* EV_EOF is set if the client has disconnected */
				if ( tevents[i].flags & EV_EOF )
                    closeConnection( client );
                else if ( ( sock_num = findSocket( fd ) ) != -1 ) /* if fd is one of the listening sockes -> accept the connection */
                {
                    std::cout << "accepting for: " << fd << std::endl;
                    client = acceptRequest( sock_num );
                    if ( client == nullptr )
                        return ERROR;
					conn_fd = client->getConnectionFD();
                    std::cout << "OPENED:" << conn_fd << std::endl;
                    set_chlist( chlist, conn_fd, EVFILT_READ, EV_ADD, data, 0, nullptr );
                }
                else if ( tevents[i].filter == EVFILT_READ ) /* if fd is one of the established connections -> READ or WRITE */
                {
                    std::cout << "READING from:" << fd << std::endl;
					ret = this->receiveClientRequest( client, request );
                    if ( ret == ERROR )
                    	return ERROR ;
                    if ( ret == STOP_READ )
						set_chlist( chlist, fd, EVFILT_READ, EV_DELETE, data, 0, nullptr );
					if ( ret == CONT_READ )
						continue ;
                    set_chlist( chlist, fd, EVFILT_WRITE, EV_ADD | EV_ONESHOT, data, 0, nullptr );
                }
                else if ( tevents[i].filter == EVFILT_WRITE )
                {
                    std::cout << "WRITING to:" << fd << std::endl;
                    if ( this->sendResponseToClient( client ) == ERROR )
						return ERROR;
                }
            }
        }
		bounceTimedOutClients();
    }
	return -1;
}

// add the sockets according to the config file
int Server::openSockets( std::string configFilePath )
{
    std::ifstream	configFile;
    std::string		line;
    std::string		socketConfig;

    configFile.open( configFilePath );
    if ( !configFile.is_open() )
        return printerror( "Error opening config file: " );
    while ( std::getline( configFile, line ) )
    {
        if ( !line.compare( "{" ) )
        {
            while ( std::getline( configFile, line ) )
            {
                if ( !line.compare( "}" ) )
                    break;
                socketConfig = socketConfig + line + "\n";
            }
			Socket *new_sock = new Socket( socketConfig, this->_path );
			if ( new_sock->bound == true )
            	this->_sockets.push_back( new_sock );
			else
				delete new_sock;
			socketConfig.clear();
        }
    }
    configFile.close(); 
    return 0;
}

// start the server
int	Server::startServer( std::string configFilePath, std::string path )
{
	int	status = 0;
    this->_path = path;
    if ( openSockets( configFilePath ) )
        return printerror( "" );
    std::cout << "\033[1mOpened sockets: \033[0m";
    for ( size_t i = 0; i < this->_sockets.size(); i++ )
        std::cout << this->_sockets[i]->fd << " ";
    std::cout << "\n\033[1mListening to ports: \033[0m";
    for ( size_t i = 0; i < this->_sockets.size(); i++ )
        std::cout << this->_sockets[i]->port << " ";
    std::cout << std::endl << std::endl;;
	status = this->monitor_ports();
	closeSockets();
	if (status == -1)
		return printerror( "monitor failed: " );
	return 0;
}

void	Server::closeSockets( void )
{
	while ( !_sockets.empty() )
	{
		delete _sockets.back();
		_sockets.pop_back();
	}
}
