#include "../inc/Server.hpp"

Client *Server::acceptRequest( int sock_num )
{
    int conn_fd = accept( this->_sockets[sock_num]->fd,
                        ( struct sockaddr* )&this->_sockets[sock_num]->socketAddr,
                        ( socklen_t * )&this->_sockets[sock_num]->socketAddrLen );
    if ( conn_fd == -1 )
	{
        printerror( "error: accept\n" );
		return nullptr ;
	}
    Client *newclient = new Client( conn_fd, sock_num );
    this->_clients.push_back( newclient );
    int status = fcntl( conn_fd, F_SETFL, O_NONBLOCK );
    if ( status == -1 )
        printerror( "fcntl failed:" );
    return this->_clients.back();
}

int Server::receiveClientRequest( Client *client, std::string & request )
{
    ssize_t				bytesRead = 1;
	int					c_fd = client->getConnectionFD();
	int					sock_num = client->getSockNum();
    std::ofstream		ofs;
	std::vector<char>	buff( 1024 * 1024 );

	bytesRead = recv( c_fd, &buff[0], buff.size(), 0 );
	client->update_client_timestamp();
	if ( bytesRead == 0 )
		return STOP_READ;
	if ( bytesRead == -1 )
		return CONT_READ;
	for ( int i = 0; i < bytesRead; ++i ) {
		request.push_back( buff[i] );
	}
    if ( client->requestIsRead() == true ) //clear content from previous request
    {
        ofs.open( this->_sockets[sock_num]->logFile, std::ofstream::out | std::ofstream::trunc );
        ofs.close();
		client->setHeaderIsSet( false );
		client->setBody( "" );
		client->setHeader( "", 0 );
    }
	//save request in logfile
    ofs.open( this->_sockets[sock_num]->logFile, std::fstream::out | std::fstream::app );
    ofs << request;
    ofs.close();
    parseRequest( request, client );
	if ( client->requestIsRead() == false && client->bodyTooLarge() == false )
		return CONT_READ;
	request.clear();
	if ( client->bodyTooLarge() == true || client->illegalRequest() == true )
		return STOP_READ;
    return 0;
}

// remove the temporary response files
void    removeResponseFiles( void )
{
    DIR	*directory;

    directory = opendir( "response" );
    if ( !directory )
        std::cout << "can not open directory(removeResonseFiles)\n";
    for ( struct dirent *dirEntry = readdir( directory ); dirEntry; dirEntry = readdir( directory ) )
    {
        std::string link = std::string( dirEntry->d_name );
        if ( link != "." && link != ".." )
        {
            link = "response/" + link;
            std::remove( link.c_str() );
        }
    }
	closedir( directory );
}

void	Server::resetPages( Client *client )
{
	std::map< std::string, Config* >::iterator	it;
	std::string	path = this->_path + "/" + this->_sockets[client->getSockNum()]->serverConfig->root;
	
	this->_sockets[client->getSockNum()]->serverConfig->pages.clear();
	this->_sockets[client->getSockNum()]->serverConfig->setPages(path, "");
	for (it = this->_sockets[client->getSockNum()]->routes.begin(); it != this->_sockets[client->getSockNum()]->routes.end(); ++it)
	{
		path = this->_path + "/" + (*it).second->root;
		(*it).second->pages.clear();
		(*it).second->setPages(path, "");
	}
}

// create a correct response to the request of the client and send it back
int	Server::sendResponseToClient( Client *client )
{
	Config			*config = this->_sockets[client->getSockNum()]->getConfig( client->getLocation() );
	int				fileSize, c_fd = client->getConnectionFD();
    std::string     htmlFileName;
    std::ifstream   htmlFile;
    std::fstream    responseFile;
    std::ofstream 	ofs;
	ssize_t			bytesSent;

    // clear response file
    ofs.open("response/response.txt", std::ofstream::out | std::ofstream::trunc);
    ofs.close();
    
    // open streamfiles
    responseFile.open( "response/response.txt", std::ios::in | std::ios::out | std::ios::binary );
    if ( !responseFile.is_open() )
        return printerror( "could not open response file " );
    htmlFileName = this->getHtmlFile( client );
	if (htmlFileName == "DO NOTHING")
		return 0;
    if ( !htmlFileName.size() )
        htmlFileName = config->errorPageDir + "500.html";
	htmlFileName = this->getErrorPage( htmlFileName, config );
    htmlFile.open( htmlFileName, std::ios::in | std::ios::binary );
    if ( !htmlFile.is_open() )
    {
		std::cout << "cant open filename:" << htmlFileName << std::endl;
    	this->_responseHeader = "HTTP/1.1 500 Error";
    	htmlFile.open( config->errorPageDir + "500.html", std::ios::in | std::ios::binary );
		if ( !htmlFile.is_open() )
		{
			htmlFile.open( createResponseHtml() );
			if ( !htmlFile.is_open() )
			{
				std::cout << "couldnt create a response.." << std::endl;
				closeConnection( client );
				return 0;
			}
		}
    }

	//get length of htmlFile
	htmlFile.seekg( 0, std::ios::end );
	fileSize = htmlFile.tellg();
	htmlFile.clear();
	htmlFile.seekg( 0, std::ios::beg );

	//read correct headers (first one set in 'findHtmlFile') into responseFile
	responseFile << this->_responseHeader << std::endl;
	if ( htmlFileName.substr( htmlFileName.size() - 5, 5 ) == ".html" )
		responseFile << "Content-Type: text/html" << std::endl;
	else if ( htmlFileName.substr( htmlFileName.size() - 4, 4 ) == ".ico" )
	{
		responseFile << "Content-Type: image/x-icon" << std::endl;
	}
	else if ( htmlFileName.substr( htmlFileName.size() - 4, 4 ) == ".png" )
		responseFile << "Content-Type: image/png" << std::endl;
	else
		responseFile << "Content-Type: text/plain" << std::endl;
	if ( htmlFileName.substr( htmlFileName.size() - 4, 4 ) != ".ico" )
	responseFile << "Content-Length: " << fileSize << "\r\n\r\n"; //std::endl << std::endl;

	//create char string to read html into, which is then read into responseFile      
	char    html[fileSize + 1];
	htmlFile.read( html, fileSize );
	html[fileSize] = '\0';
	responseFile << html << std::endl;

	//get length of full responseFile
	responseFile.seekg( 0, std::ios::end );
	fileSize = responseFile.tellg();
	responseFile.clear();
	responseFile.seekg( 0, std::ios::beg );

	//create response which is sent back to client
	char	response[fileSize + 1];
	responseFile.read( response, fileSize );
	response[fileSize] = '\0';
	bytesSent = send( c_fd, response, fileSize, 0 );
	if ( bytesSent == -1 )
	{
		htmlFile.close();
		responseFile.close();
		closeConnection( client );
		std::cout << "error sending data to client.." << std::endl;
		return 0;
	}
	client->update_client_timestamp();
	std::cout << "\n\033[32m\033[1m" << "RESPONDED:\n\033[0m\033[32m" << std::endl << response << "\033[0m" << std::endl;
	this->_responseHeader.clear();
	htmlFile.close();
	responseFile.close();
	std::ifstream ifs( "response/responseCGI.html" );
	if ( ifs.good() )
		ifs.close();
	removeResponseFiles();
	resetPages( client );
	if ( client->bodyTooLarge() == true )
		closeConnection( client );
    return 0;
}
