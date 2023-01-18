#include "../inc/Server.hpp"

std::string	Server::methodGET( Client *client, Config *config )
{
	int			sock_num = client->getSockNum();
	int			port = this->_sockets[sock_num]->port;
	std::string	page, query, index, location;

	location = client->getLocation();
	if (location.find("?") != std::string::npos)
	{
		query = location.substr(location.find("?") + 1, location.size() - (location.find("?") + 1));
		location = location.substr(0, location.find("?"));
	}
    page = this->_sockets[sock_num]->getLocationPage( location, client->getHost(), client );

    // respond to a GET request that requests a directory
	if ( page == "Directory" )
	{
        // responds the (if set) directoryrequest
		_responseHeader = "HTTP/1.1 200 OK";
		if ( config->directoryRequest != "" )
			return ( config->root + config->directoryRequest );
		
        // else search for an index
        index = location + "index.html";
		if ( location.back() != '/' )
			index = location + "/index.html";
		page = this->_sockets[sock_num]->getLocationPage( index, client->getHost(), client );
		if ( page != "" )
		    return ( page );
        // if there was no index -> create an autoindex (if enabled)
        if ( config->autoindex )
            return ( this->createAutoIndex( config->root, location ) );
        _responseHeader = "HTTP/1.1 403 Forbidden";
        return ( config->errorPageDir + "403.html" );
	}
	if ( page != "" )
    {
		// execute the CGI on the requested file if it has the right extension
		if ( location.size() > config->extension.size() && location.substr( location.size() - config->extension.size(), location.size() - 1) == config->extension )
		{
			switch ( executeCGI( "/" + config->root + location, port, this->_path, config->root, query, client->getHeader(), config->uploadDir, "GET" ) )
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
	std::string		location = client->getLocation(), page = this->_sockets[sock_num]->getLocationPage( location, client->getHost(), client );
	std::string		header = client->getHeader(), body = client->getBody(), newFileName, newFileContent, index;
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
		return ( "response/responseBinaryUpload" );
	}
	if ( page != "" )
    {
		// execute the CGI on the requested file if it has the right extension
		if ( location.size() > config->extension.size() && location.substr( location.size() - config->extension.size(), location.size() - 1) == config->extension )
		{
			switch ( executeCGI( "/" + config->root + location, port, this->_path, config->root, client->getBody(), client->getHeader(), config->uploadDir, "POST" ) )
			{
				case 0:
					_responseHeader = "HTTP/1.1 200 OK";
					return ( "response/responseCGI" );
				case 1:
					_responseHeader = "HTTP/1.1 500 Error";
					return ( config->errorPageDir + "500.html" );
				case NO_FILE:
    				_responseHeader = "HTTP/1.1 404 Not Found";
					return ( config->errorPageDir + "404.html" );
				case -1:
					return ( "DO NOTHING" );
			}
		}
		// POST request is a form
		if (client->getHeader().substr(client->getHeader().find("Content-Type: ") + 14, client->getHeader().find("\r\n", client->getHeader().find("Content-Type: ") + 14) - (client->getHeader().find("Content-Type: ") + 14)) == "application/x-www-form-urlencoded")
		{
			int	posMid = body.find("="), posStart = 0;
			time_t rawtime;
  			struct tm *timeinfo;

  			time(&rawtime);
  			timeinfo = localtime(&rawtime);
			newFile.open("forms/form.log", std::ios_base::app);
  			newFile << "Local date and time " <<  asctime(timeinfo);
			newFile << body.substr(posStart, posMid) << " = " << body.substr(posMid + 1, body.find("&") - (posMid + 1)) << "\n";
			posStart = body.find("&");
			posMid = body.find("=", posStart);
			newFile << body.substr(posStart + 1, posMid - (posStart + 1)) << " = " << body.substr(posMid + 1, body.find("&", posMid) - (posMid + 1)) << "\n";
			posStart = body.find("&", posMid);
			posMid = body.find("=", posStart);
			newFile << body.substr(posStart + 1, posMid - (posStart + 1)) << " = " << body.substr(posMid + 1, body.size() - (posMid + 1)) << "\n\n";
			free(timeinfo);
			_responseHeader = "HTTP/1.1 201 Created";
			return ( config->errorPageDir + "201.html" );
		}
		newFileName = location + "/" + body.substr(0, 6);
		if (location.back() == '/')
			newFileName = location + body.substr(0, 6);
		// std::cout << newFileName << std::endl;
		newFileContent = body;
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
		_responseHeader = "HTTP/1.1 201 Created";
		return ( config->errorPageDir + "201.html" );
	}
	_responseHeader = "HTTP/1.1 404 Not Found";
    return ( config->errorPageDir + "404.html" );
}

std::string	Server::methodDELETE( Client *client, Config *config )
{
	std::string		page = this->_sockets[client->getSockNum()]->getLocationPage( client->getLocation(), client->getHost(), client );
    std::ifstream	file;

	// check if the requested file exists and delete it
	file.open( config->root + client->getLocation() );
	if ( file )
		remove( ( config->root + client->getLocation() ).c_str() );
	_responseHeader = "HTTP/1.1 200 OK";
	return ( config->root + "/index.html" );
}
