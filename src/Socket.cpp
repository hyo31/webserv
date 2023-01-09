#include "../inc/Socket.hpp"

//each serverblock listens to a socket.
//The socket objects bind with the server and listen to the port
//Each object creates its own configuration file
Socket::Socket( std::string config, std::string path ) : bound( false )
{
    size_t	start, end;

    try
    {
		this->serverConfig = new Config( config, path );
		start = config.find( "listen" ) + 7;
		end = config.find( " ", start );
        port = std::stoi( config.substr( start, end - start ) );
        logFile = "logs/port" + config.substr( start, end - start ) + ".log";
        ipAddr = "localhost";
		currentFile = "";
    	this->setupSockets();
		if ( this->bound == false )
			return ;
		this->setRouteConfigs( config );
    }
    catch( std::invalid_argument const& e )
    {
        std::cout << "what():" << e.what() << std::endl;
        exit ( ft_return( "Error reading config file" ) );
    }
}
Socket::Socket( const Socket & ) { std::cout << "cant copy sockets!" << std::endl; }
Socket &	Socket::operator=( const Socket & ) { std::cout << "no assignment allowed for socket object!" << std::endl; return *this; }

Socket::~Socket()
{
	//delete configs
	std::map< std::string, Config* >::iterator	it;
	for ( it = this->routes.begin(); it != this->routes.end(); ++it ) {
		delete (*it).second;
	}
	delete this->serverConfig;
    std::cout << "Socket:" << this->fd << " - bound to port:" << this->port << " closed\n";
    close( fd );
}

//Creates socket, fills in sockaddr_in struct in order to bind socket to address, and makes server listen to this socket
int Socket::setupSockets()
{
    this->fd = socket( AF_INET, SOCK_STREAM, 0 );
    if ( this->fd == -1 )
        return ft_return("error: socket\n" );
    int status = fcntl( this->fd, F_SETFL, O_NONBLOCK );	
    if ( status == -1 )
        ft_return( "fcntl failed" );
    memset( &socketAddr, 0, sizeof( socketAddr ) );
    socketAddr.sin_family = AF_INET;
    socketAddr.sin_addr.s_addr = htonl( INADDR_ANY );
    socketAddr.sin_port = htons(port);
    if ( bind( this->fd, ( struct sockaddr* )&socketAddr, sizeof( socketAddr ) ) )
    {
        std::cerr << "bind failed: " << strerror( errno ) << std::endl;
        close ( this->fd );
		return 0;
    }
	this->bound = true;
    if ( listen( this->fd, 100 ) )
        return ft_return( "error: listen\n" );
    return 0;
}

//sets up a map with all the info for all the routes
//key = location, value = pointer to the config
void	Socket::setRouteConfigs( std::string configfile )
{
	std::string	location, route;
	size_t		start = 0, end = 0;

	while ( ( start = configfile.find( "location", end ) ) != std::string::npos )
	{
		Config	*routeConfig = new Config( *this->serverConfig );
		end = configfile.find( "{", start ) - 1;
		location = configfile.substr( start + 9, end - ( start + 9 ) );
		end = configfile.find( "}", start );
		route = configfile.substr( start, ( end - start ) );
		routeConfig->setConfig( route );
		routeConfig->setRedirects( route, location );
		this->routes.insert( std::make_pair( location, routeConfig ) );
	}
}

//finds the correct config corresponding to the (clients) requested location
//if the client tries to upload, the request will be routed to our .pl script
//and the location needs to be the root directory , not the name itself
Config	*Socket::getConfig( std::string location ) const
{
	std::map<std::string, Config*>::const_iterator	it;
	std::string	new_location = location;
	size_t		pos;

	pos = location.find( ".pl" );
	if ( pos != std::string::npos )
	{
		pos = location.find_last_of( "/" ) + 1;
		new_location = location.substr( 0, pos );
	}
	it = routes.find( new_location );
	if ( it != routes.end() )
		return it->second;
	pos = location.find( ".html" );
	if ( pos != std::string::npos )
	{
		pos = location.find_last_of( "." );
		new_location = location.substr( 0, pos );
	}
	it = routes.find( new_location );
	if ( it != routes.end() )
		return it->second;
	return this->serverConfig;
}

//find the correct page for the requested location and if root is set in a route it will be replaced
std::string Socket::getLocationPage( std::string location ) const
{
    std::map<std::string, std::string>::iterator    it;
	Config	*config = this->getConfig( location );
	size_t	pos;

    it = config->pages.find( location );
    if ( it == config->pages.end() )
        return "";
	if ( config->root != "" )
	{
		pos = it->second.find( "htmlFiles" );
		if ( pos != std::string::npos )
			it->second.replace( pos, 9, config->root );
	}
    return it->second;
}

//find the correct page to redirect to
std::string Socket::getRedirectPage( std::string location ) const
{
    std::map<std::string, std::string>::iterator	it;
	Config	*config = this->getConfig( location );

    it = config->redirects.find( location );
	if ( it == config->redirects.end() )
        return "";
    return it->second;
}
