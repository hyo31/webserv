#include "../inc/Server.hpp"

std::string	Server::methodGET( Client *client, Config *config )
{
	int			sock_num = client->getSockNum();
	int			port = this->_sockets[sock_num]->port;
	std::string query, index, location = client->getLocation();

	for ( std::vector<std::string>::iterator it = this->_sockets[sock_num]->hosts.begin(); it != this->_sockets[sock_num]->hosts.end(); it++ ) {
		std::cout << *it << std::endl;
	}

	if (location.find("?") != std::string::npos)
	{
		query = location.substr(location.find("?") + 1, location.size() - (location.find("?") + 1));
		location = location.substr(0, location.find("?"));
	}
    std::string page = this->_sockets[sock_num]->getLocationPage( location, client->getHost() );
	std::string redirect_page = this->_sockets[sock_num]->getRedirectPage( location, client->getHost() );
    
	// check if requested page is a redirection
	if ( redirect_page != "" )
	{
		_responseHeader = "HTTP/1.1 301 Moved Permanently\r\nLocation: ";
		_responseHeader.append( redirect_page );
		return ( config->errorPageDir + "301.html" );
	}

    // respond to a GET request that requests a directory
	if ( page == "Directory" )
	{
        // responds the (if set) directoryrequest
		_responseHeader = "HTTP/1.1 200 OK";
		if ( config->directoryRequest != "" )
			return ( config->root + config->directoryRequest );
		
        // else search for an index
        index = location + "index.html";
		page = this->_sockets[sock_num]->getLocationPage( index, client->getHost() );
		if ( page != "" )
		    return ( page );

        // if there was no index -> create an autoindex (if enabled)
        if ( config->autoindex )
            return ( this->createAutoIndex( config->root, location ) );
        _responseHeader = "HTTP/1.1 403 Forbidden";
        return ( config->errorPageDir + "403.html" );
	}

	// execute the CGI on the requested file if it has the right extension
	if ( location.size() > config->extension.size() && location.substr( location.size() - 3, location.size() - 1) == config->extension )
	{
		switch ( executeCGI( "/" + config->root + config->cgi + location, port, this->_path, config->root, query, client->getHeader(), config->uploadDir, "GET" ) )
		{
			case 0:
				_responseHeader = "HTTP/1.1 200 OK";
				return ( "response/responseCGI" );
			case 1:
				_responseHeader = "HTTP/1.1 500 Error";
				return ( config->errorPageDir + "500" );
			case -1:
				return ( "DO NOTHING" );
		}
	}
	if ( page != "" )
    {
        _responseHeader = "HTTP/1.1 200 OK";
        return ( page );
    }
    _responseHeader = "HTTP/1.1 404 Not Found";
    return ( config->errorPageDir + "404.html" );
}

std::string	Server::methodPOST( Client *client, Config *config )
{
	int				sock_num = client->getSockNum(), fileExtension = 0;
	int				port = this->_sockets[sock_num]->port;
	std::string		location = client->getLocation(), page = this->_sockets[sock_num]->getLocationPage( location, client->getHost() ), body, newFileName, newFileContent, index;
	std::ofstream	newFile;
	std::ifstream	checkIfOpen;

	if ( client->bodyTooLarge() == true )
	{
		_responseHeader = "HTTP/1.1 413 Request Entity Too Large";
		return ( config->errorPageDir + "413.html" );
	}
	if ( BinaryFile( client->getBody() ) == true )
	{
		SaveBinaryFile( this->_path, client, config );
		_responseHeader = "HTTP/1.1 200 OK";
		return ( "response/responseCGI" );
	}

	// execute the CGI on the requested file if it has the right extension
	if ( location.size() > config->extension.size() && location.substr( location.size() - 3, location.size() - 1) == config->extension )
	{
		switch ( executeCGI( "/" + config->root + config->cgi + location, port, this->_path, config->root, client->getBody(), client->getHeader(), config->uploadDir, "GET" ) )
		{
			case 0:
				_responseHeader = "HTTP/1.1 200 OK";
				return ( "response/responseCGI" );
			case 1:
				_responseHeader = "HTTP/1.1 500 Error";
				return ( config->errorPageDir + "500.html" );
			case -1:
				return ( "DO NOTHING" );
		}
	}
	if ( page != "" )
    {
        _responseHeader = "HTTP/1.1 200 OK";
		body = client->getBody();
		newFileName = location + "/" + body.substr(0, body.find("="));
		newFileContent = body.substr(body.find("=") + 1, body.size() - (body.find("=") + 1));
		checkIfOpen.open(config->root + newFileName);
		while ( checkIfOpen.is_open() )
		{
			fileExtension++;
			checkIfOpen.close();
			checkIfOpen.open(config->root + newFileName + std::to_string(fileExtension));
		}
		if (fileExtension)
			newFileName += std::to_string(fileExtension);
		newFile.open(config->root + newFileName);
		newFile << newFileContent;
		newFile.close();

        // responds the (if set) directoryrequest
		_responseHeader = "HTTP/1.1 200 OK";
		if ( config->directoryRequest != "" )
			return ( config->root + config->directoryRequest );
		
        // else search for an index
        index = location + "index.html";
		page = this->_sockets[sock_num]->getLocationPage( index, client->getHost() );
		if ( page != "" )
		    return ( page );

        // if there was no index -> create an autoindex (if enabled)
        if ( config->autoindex )
            return ( this->createAutoIndex( config->root, location ) );
        _responseHeader = "HTTP/1.1 403 Forbidden";
        return ( config->errorPageDir + "403.html" );
	}
	_responseHeader = "HTTP/1.1 404 Not Found";
    return ( config->errorPageDir + "404.html" );
}

std::string	Server::methodDELETE( Client *client, Config *config )
{
	std::string		page = this->_sockets[client->getSockNum()]->getLocationPage( client->getLocation(), client->getHost() );
    std::ifstream	file;

	// check if the requested file exists and delete it
	file.open( config->root + client->getLocation() );
	if ( file )
		remove( ( config->root + client->getLocation() ).c_str() );
	_responseHeader = "HTTP/1.1 200 OK";
	return ( config->root + "/index.html" );
}
