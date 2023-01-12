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
    std::vector<Client*>::iterator client = this->_clients.begin();
    std::vector<Client*>::iterator end = this->_clients.end();
    time_t current_time = std::time(nullptr);

    while ( client != end )
    {
        if ( ( *client )->getTimeStamp() + TIMEOUT <=  current_time )
        {
            std::cout << "Bouncing client from:" << ( *client )->getConnectionFD() << std::endl;
			closeConnection( *client );
			client = _clients.begin();
			if ( client == _clients.end() )
				return ;
        }
		else
			client++;
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
		return config->errorPageDir + "405.html";
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


std::string	createResponseHtml( void )
{
	std::ofstream	ofs("public_html/temp.html");
	ofs << "<!DOCTYPE html>" << std::endl
		<< "<html lang=\"en\">" << std::endl
		<< "<head>" << std::endl
		<< "	<title>500 Error</title>" << std::endl
		<< "	<link rel=\"icon\" type=\"image/png\" sizes=\"16x16\" href=\"data:image/png;base64," << std::endl
		<< "	iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAIAAAAlC+aJAAAACXBIWXMAAAsTAAALEwEAmpwYAAAAGXRFWHRTb2Z0d2FyZQBBZG9iZSBJbWFn" << std::endl
		<< "	ZVJlYWR5ccllPAAABORJREFUeNrsWltIY1cUNbljKtGMqKXVzOhIRRttg6kKqcVKfQVHHNCPBKpYfKCoUCTG+KHEnyi0xRcK+iODocRnPgQ/" << std::endl
		<< "	6gs0qOD7hSKNSDVSEpWI0mhjNKYbLYMMc09uzE1yB7K+9Obuc/bae5+z99nnenl54IEHHzVopI8YExOTkpLC4/EiIyMjIiKe3wOe39zcXF5e6" << std::endl
		<< "	vV6jUazt7e3sLCgVqtPT0+pYomEhISOjg6dTme1B+vr6xKJJDg42G160+l0kUi0trZmdQC3t7cDAwNcLtfV2mdkZOzs7FjJA9AIDQ11herg9O" << std::endl
		<< "	HhYasTAOukurqaRqM5UXuBQABr0epMTE5OhoSEOEX7mpqau7s7q/Oh1WpjY2NJ3XFptLa2NqsLsbq6SiaB9vZ2V2pvMBiioqJsavWMoPZ1dXVV" << std::endl
		<< "	VVV2Ed7Y2FhZWYGcBdnKYrH4+Piw2WxIc0lJSTbjG7JeTk4OpDxybJ+VlUXccvv7++jcBKEYHx/f1dVlNBrxBsnLyyMa2H/yv7fho+DPX/3+Fm" << std::endl
		<< "	OxbI4FlgZH9fb2QmIiuBfLZLLy8nLIho+fNzQ0yOVy0gi8aPvNN/FbmwOpVCpQBQLXXvfy+XylUglV08O/CoWiqKgInEC0FED/zEr9gYj2UqlUK" << std::endl
		<< "	BQ+QXvA4uIi1FHj4+Pw98zMTFlZGXHtAdjPL1/hegfD2K2/Yn5+6CEKCwu7u7sdWWMmk2loaIjFYlVWVkImtksWtQuxMgXetupEyGvgdMf3CbPZ" << std::endl
		<< "	LBaLn1JNIvJWUNFPaGEwW0tLi3vLeFwCzG943i9fICRPTk4gXt1+DsEl8Dz7NVqytrb24uKCokdKDMNgS/H398cTg2wFeR6qOpsTHHUmu8EDcX" << std::endl
		<< "	FxCO0BUNUR0d5tIZSamoreMfr7+ylyFqfjdRYQMnNzc2dnZ5QmwOFwEDJqtZo6faEPEwgPD0fIbG9vU52AH7J8ODw8pDoBJpOJkDk/P6cOgWc" << std::endl
		<< "	/vrU7Ht788sfxP2aCL4cWf+0GD1zfovZ4JoNO9RBCE/jUj0F1AqfGG4QM2/8TqhPQXVwjZL5i+1KdwN/nKAJffsZkMjBKE9jRoc513hjtuy/8" << std::endl
		<< "	KU3gL4Pp3xvUOn4dE+jU/jEJUCqV6AZWQUEBpQmkp6ejCeh0uofLL4qCTqcfHBygOYCX3K4n7mYC+plMpuzsbIQwl8uFg8HS0hJFncBgMLRaLd" << std::endl
		<< "	oJFoslPz+fBENiWFNTU0BAAMkcRCKRzXY0cCgtLXVkFl9f35GRERhqenoarEYyh7GxMSJd9Z6eHj9bTUi84+vW1ta7cRQKBck3fGFhYRDoRDjA" << std::endl
		<< "	os/NzSU+PYvFksvl19fX740jk8kc7Qu9B1jKo6OjBEfc3Nzs7OwcHBw0Go1478CRtaSkpKKiIigo6IMvwLrq6+t7/ATv3ELUWvX19Y2NjcQNYza" << std::endl
		<< "	b5+fnl5eXNRqNwWAAM0Ogs9ns6Ojo5ORkdNfD6/6KKS0tbXZ2ljQCXveXfPZekzkCiNvExMR312R4BOw4W4nFYuDgMgKBgYFE2md2EIDlBRykU" << std::endl
		<< "	qldNyhPxtHRUXFxMZkEHtDc3JyZmXl8fOxU7aempvh8PuwH5BMATExM8Hg8lUrlDNWvrq4kEolAIIBi8ennAZvQ6/VCoRCm2d3dJVF72Hw5HE5" << std::endl
		<< "	ra6trovT/ovUj/uDpMdz1yZnnoz8PPPDAvfhPgAEAeYUvyGcO9EMAAAAASUVORK5CYII=\" />" << std::endl
		<< "</head>" << std::endl
		<< "<style>" << std::endl
		<< ".borderexample {" << std::endl
		<< "border-bottom:solid thin white;" << std::endl
		<< "}" << std::endl
		<< "</style>" << std::endl
		<< "<body style=\"background-color:rgb(67, 67, 67);\">" << std::endl
		<< "	<h1 class=\"borderexample\"; style=\"text-align:center;color:white\">500 Error</h1>" << std::endl
		<< "	<p style=\"text-align:center;color:white\">Something Went Wrong</p>" << std::endl
		<< "</body>" << std::endl
		<< "</html>" << std::endl;
	ofs.close();
	return "public_html/error500.html";
}

std::string	Server::getErrorPage( std::string response, Config *config )
{
	std::map<std::string, std::string>::iterator	it;
	std::string	responseCode;
	size_t		start, end;

	start = this->_responseHeader.find ( " " ) + 1;
	end = this->_responseHeader.find ( " ", start );
	if ( end == std::string::npos )
		responseCode = this->_responseHeader.substr( start );
	else
		responseCode = this->_responseHeader.substr( start, end - start );
	if ( ( responseCode[0] == '4' || responseCode[0] == '5' ) && responseCode.size() == 3 )
		if ( ( it = config->errorPages.find( responseCode ) ) != config->errorPages.end() )
			return config->root + it->second;
	return response;
}

int	Server::uniqueSocket( std::string config, std::string & host )
{
	size_t		start, end;
	std::string	line;
	int			port;

	start = config.find( "listen" ) + 7;
	end = config.find( '\n', start );
	line = config.substr( start, end - start );
	port = std::stoi( line );
	start = line.find( " " );
	if ( start != std::string::npos )
		host = line.substr( start );
	else
		host = "localhost";
	for ( std::vector<Socket *>::iterator it = this->_sockets.begin(); it != this->_sockets.end(); it++ ) {
		if ( port == (*it)->port )
			return port;
	}
	return 0;
}

void	Server::addHost( int port, std::string host )
{
	std::vector<std::string>::iterator	it;

	for ( it = this->_sockets[port]->hosts.begin(); it != this->_sockets[port]->hosts.end(); it++ ) {
		if ( *it == host )
			return ;
	}
	this->_sockets[port]->hosts.push_back( host );
}
