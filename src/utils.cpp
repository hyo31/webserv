#include "../inc/Server.hpp"

int printerror( std::string str )
{
    std::cerr << str << strerror( errno ) << std::endl;
    return (-1);
}

void Server::set_chlist( std::vector<struct kevent>& change_list, uintptr_t ident, int16_t filter,
        uint16_t flags, uint32_t fflags, intptr_t data, void *udata )
{
    struct kevent temp_evt;

    EV_SET( &temp_evt, ident, filter, flags, fflags, data, udata );
    change_list.push_back( temp_evt );
}

void	Server::closeConnection( Client *client )
{
	if ( client == nullptr )
		return ;
    std::vector<Client*>::iterator	it;
    std::vector<Client*>::iterator	end = this->_clients.end();
	int	fd = client->getConnectionFD();

    for( it = this->_clients.begin(); it != end; ++it )
    {
        if ( *it == client )
        {
            delete client;
            this->_clients.erase( it );
			break ;
        }
    }
    close( fd );
    std::cout << "disconnected client from socket:" << fd << std::endl;
    return ;
}

void    Server::bounceTimedOutClients()
{
    std::vector<Client*>::iterator client;
    std::vector<Client*>::iterator end = this->_clients.end();
    time_t current_time = std::time(nullptr);

    for( client = this->_clients.begin(); client != end; ++client )
    {
        if ( ( *client )->getTimeStamp() + TIMEOUT <=  current_time )
        {
            std::cout << "Bouncing client from:" << ( *client )->getConnectionFD() << std::endl;
			closeConnection( *client );
			break ;
        }
    }
}

int	Server::findSocket( int fd )
{
    for ( size_t i = 0; i < this->_sockets.size(); i++ )
        if ( fd == this->_sockets[i]->fd )
            return i;
    return -1;
}

Client *Server::findClient( int c_fd )
{
	std::vector<Client*>::iterator it;

    for( it = this->_clients.begin(); it != this->_clients.end(); ++it )
        if ( c_fd == (*it)->getConnectionFD() )
            break ;
	if ( it == this->_clients.end() )
		return nullptr;
	return	( *it );
}

// find the right file to answer to the request
std::string Server::getHtmlFile( Client* client )
{
	Config		*config = this->_sockets[client->getPort()]->getConfig( client->getLocation() );
	std::string	method = client->getMethod();

    if ( method == "DELETE" )
	    return methodDELETE( client, config );
	if ( std::find( config->methods.begin(), config->methods.end(), method ) == config->methods.end() )
	{
		_responseHeader = "HTTP/1.1 405 Method Not Allowed";
		return config->errorpages + "405.html";
	}
	if ( method == "POST" )
	    return methodPOST( client, config );
	return methodGET( client, config );
}

void	SaveBinaryFile( std::string path, Client *client, Config *config )
{
	std::vector<std::string>	to_upload;
	std::string					temp, dest;
	DIR							*directory;
	
	directory = opendir((path + "/" + config->root + config->uploadDir).c_str());
	if ( !directory )
		mkdir( (path + "/" + config->root + config->uploadDir).c_str(), 0777 );
	else
		closedir( directory );
	to_upload = readFile( client->getHeader(), client->getBody() );
	dest = path + "/" + config->root + config->uploadDir + to_upload[0];

	std::ofstream	outfile( config->root + config->uploadDir + to_upload[0], std::ofstream::out | std::ofstream::trunc );
	outfile << to_upload[1];
	outfile.close();

	std::ofstream outfile2( "response/responseCGI.html", std::ofstream::out | std::ofstream::trunc );
	std::ifstream ifs( config->root + "/uploadresponse.html" );
	ifs.seekg( 0, std::ios::end );   
	temp.reserve( ifs.tellg() );
	ifs.seekg( 0, std::ios::beg );
	temp.assign( ( std::istreambuf_iterator<char>( ifs ) ), std::istreambuf_iterator<char>() );
	temp.replace( temp.find( "$outfile" ), 8,  dest, 0, dest.size() );
	outfile2 << temp;
	outfile2.close();
}

bool BinaryFile( std::string body )
{
	const char *c_str = body.c_str();
	if ( strlen( c_str ) != body.size() )
		return true;
	return false;
}
