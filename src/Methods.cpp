#include "../inc/Server.hpp"

std::string	Server::methodGET( Client *client, Config *config )
{
	int			port = client->getPort();
	std::string index, location = client->getLocation();
    std::string page = this->_sockets[port]->getLocationPage( location );
	std::string redirect_page = this->_sockets[port]->getRedirectPage( location );

    // respond to a GET request that requests a directory
	if ( page == "Directory" )
	{
        // responds the (if set) directoryrequest
		_responseHeader = "HTTP/1.1 200 OK";
		if ( config->directoryRequest != "" )
			return ( config->root + config->directoryRequest );
		
        // else search for an index
        index = location + "index.html";
		page = this->_sockets[port]->getLocationPage( index );
		if ( page != "" )
		    return ( page );

        // if there was no index -> create an autoindex (if enabled)
        if ( config->autoindex )
            return ( this->createAutoIndex( config->root, location ) );
        _responseHeader = "HTTP/1.1 403 Forbidden";
        return ( config->errorpages + "403.html" );
	}
    // check if requested page is a redirection
	if ( redirect_page != "" )
	{
		_responseHeader = "HTTP/1.1 301 Moved Permanently\r\nLocation: ";
		_responseHeader.append( redirect_page );
		return ( config->errorpages + "301.html" );
	}
    if ( page != "" )
    {
        _responseHeader = "HTTP/1.1 200 OK";
        return ( page );
    }
    _responseHeader = "HTTP/1.1 404 Not Found";
    return ( config->errorpages + "404.html" );
}

std::string	Server::methodPOST( Client *client, Config *config )
{
	std::string	location = client->getLocation();

	if ( client->bodyTooLarge() == true )
	{
		_responseHeader = "HTTP/1.1 413 Request Entity Too Large";
		return ( config->errorpages + "413.html" );
	}
	if ( BinaryFile( client->getBody() ) == true )
	{
		SaveBinaryFile( this->_path, client );
		_responseHeader = "HTTP/1.1 200 OK";
		return ( "response/responseCGI.html" );
	}
	// execute the CGI on the requested file if it has the right extension
	if ( location.size() > config->extension.size() && location.substr( location.size() - 3, location.size() - 1) == config->extension )
	{
		if ( !executeCGI("/" + config->cgi + location, client->getPort(), this->_path, config->root, client->getBody(), client->getHeader(), config->uploadDir ) )
		{
			_responseHeader = "HTTP/1.1 200 OK";
			return ( "response/responseCGI.html" );
		}
	}
	return ( config->errorpages + "403.html" );
}

std::string	Server::methodDELETE( Client *client, Config *config )
{
	std::string		page = this->_sockets[client->getPort()]->getLocationPage( client->getLocation() );
    std::ifstream	file;

	// if DELETE method is not allowed, return 405
	if ( page != "" && std::find( config->methods.begin(), config->methods.end(), client->getMethod() ) == config->methods.end() )
	{
		_responseHeader = "HTTP/1.1 405 Method Not Allowed";
		return (config->errorpages + "405.html");
	}
	// check if the requested file exists and delete it
	file.open( config->root + client->getLocation() );
	if ( file )
		remove( ( config->root + client->getLocation() ).c_str() );
	_responseHeader = "HTTP/1.1 200 OK";
	return ( "htmlFiles/index.html" );

}