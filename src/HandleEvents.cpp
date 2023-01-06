#include "../inc/Server.hpp"

Client *Server::acceptRequest( int sock_num )
{
    int conn_fd = accept( this->_sockets[sock_num]->fd,
                        ( struct sockaddr* )&this->_sockets[sock_num]->socketAddr,
                        ( socklen_t * )&this->_sockets[sock_num]->socketAddrLen );
    if ( conn_fd == -1 )
	{
        ft_return( "error: accept\n" );
		return nullptr ;
	}
    Client *newclient = new Client( conn_fd, sock_num, this->_sockets[sock_num]->serverConfig );
    this->_clients.push_back( newclient );
    int status = fcntl( conn_fd, F_SETFL, O_NONBLOCK );
    if ( status == -1 )
        ft_return( "fcntl failed:" );
    return this->_clients.back();
}

int Server::receiveClientRequest( Client *client, std::string & request )
{
    ssize_t				bytesRead = 1;
	int					c_fd = client->getConnectionFD();
	int					port = client->getPort();
    std::ofstream		ofs;
	std::vector<char>	buff( 1024 * 1024 );

	while ( ( bytesRead = recv( c_fd, &buff[0], buff.size(), 0 ) ) > 0 )
	{
    	for ( int i = 0; i < bytesRead; ++i ) {
			request.push_back( buff[i] );
		}
	}
	client->update_client_timestamp();
    if ( client->requestIsRead() == true )
    {
		// std::cout << "clearing content..\n";
        ofs.open( this->_sockets[port]->logFile, std::ofstream::out | std::ofstream::trunc );
        ofs.close();
		client->setHeaderIsSet( false );
		client->setBody( "" );
		client->setHeader( "", 0 );
    }
    ofs.open( this->_sockets[port]->logFile, std::fstream::out | std::fstream::app );
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

// create a correct response to the request of the client and send it back
int Server::sendResponseToClient( Client *client )
{
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
        return ft_return( "could not open response file " );
    htmlFileName = this->getHtmlFile( client );
    if ( !htmlFileName.size() )
        htmlFileName = client->getConfig()->errorpages + "500.html";
    htmlFile.open( htmlFileName, std::ios::in | std::ios::binary );
    if ( !htmlFile.is_open() )
    {
    	std::cout << "html file:" << htmlFileName << "  doesn't exist!" << std::endl;
    	this->_responseHeader = "HTTP/1.1 403 Forbidden";
    	htmlFile.open( client->getConfig()->errorpages + "403.html", std::ios::in | std::ios::binary );
		if ( !htmlFile.is_open() )
			htmlFile.open( "htmlFiles/pages/errorPages/500.html", std::ios::in | std::ios::binary );
    }

	//get length of htmlFile
	htmlFile.seekg( 0, std::ios::end );
	fileSize = htmlFile.tellg();
	htmlFile.clear();
	htmlFile.seekg( 0, std::ios::beg );

	//read correct headers (first one set in 'findHtmlFile') into responseFile
	responseFile << this->_responseHeader << std::endl;
	responseFile << "Content-Type: text/html" << std::endl;
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
		close( c_fd );
		return ft_return( "error: send\n" );
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
	if ( client->bodyTooLarge() == true )
		closeConnection( client );
    return (0);
}
