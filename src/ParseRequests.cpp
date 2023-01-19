#include "../inc/Server.hpp"

static size_t	getUploadBodySize( std::string body, std::string header )
{
	size_t	start, end;

	if ( header.find( "Content-Type: multipart/form-data" ) != std::string::npos )
	{
		start = body.find( "\r\n\r\n" ) + 4;
		end = body.find( "\r\n", start );
		return end - start;
	}
	return body.size();
}

static void	buildBodyForContentLength( std::string request, size_t start, Client *client, size_t maxBodySize )
{
	size_t		end, content_len;
	std::string	body, header = client->getHeader();

	std::ofstream output;

	output.open("outputofzo");
	output << request;
	output.close();
	//find the number behind Content Length (start is at this line)
	start = request.find( " ", start ) + 1;
	end = request.find( "\r\n", start );
	content_len = std::stoi( request.substr( start, end - start ) );
	
	//add new body to existing body
	start = request.find( "\r\n\r\n" ) + 4;
	body = client->getBody();
	body.append( request.substr( start, content_len ) );
	client->setBody( body );

	//check if the whole body is read
	for ( end = start; end != request.size(); ++end );
	if ( end - start != content_len )
	{
		if ( end - start > content_len )
			client->setBadRequest( true );
		return ;
	}
	client->setRequestIsRead( true );
	if ( getUploadBodySize( body, header ) > maxBodySize )
		client->setBodyTooLarge( true );
    //std::cout << "\n\033[33m\033[1m" << "RECEIVED:\n\033[0m\033[33m" << request << "\033[0m" << std::endl;
	return ;
}


static void	unchunk( std::string request, size_t start, Client *client, size_t maxBodySize )
{
	std::stringstream	ss;
	size_t 				end, chunkSize;
	std::string			substr, body;

	start = request.find( "\r\n\r\n", start ) + 4;
	if ( request.find( "\r\n\r\n", start ) == std::string::npos )
		return ;
	while ( 1 )
	{
		end = request.find( "\r\n", start );
		substr = request.substr( start, ( end - start ) );
		ss << std::hex << substr;
		ss >> chunkSize;
		ss.clear();
		if (chunkSize == 0)
			break ;
		start = end + 2;
		body = client->getBody();
		body.append( request, start, chunkSize );
		client->setBody( body );
		if ( body.size() > maxBodySize )
		{
			client->setBodyTooLarge( true );
			return ;
		}
		start = start + chunkSize + 2;
	}
	client->setRequestIsRead( true );
	std::cout << "\n\033[33m\033[1m" << "RECEIVED:\n\033[0m\033[33m" << request << "\033[0m" << std::endl;
	return ;
}

void    Server::parseRequest( std::string request, Client *client )
{
	std::string	substr, header;
    size_t		start, end, MaxBody;

    client->setRequestIsRead( false );
	/* check if full header is read and store it */
    if ( request.find( "\r\n\r\n" ) != std::string::npos )
    {
		if ( client->headerIsSet() == false )
		{
			end = request.find( "\r\n\r\n" );
			client->setHeader( request.substr( 0, end ), end );
			client->setHeaderIsSet( true );
		}
		// setHeaderInfo( client );
    }
	else
		return ;
	header = client->getHeader();
	client->setHost( header );
	if ( client->getHost() == "" )
		client->setBadRequest( true );
	MaxBody = this->_sockets[client->getSockNum()]->getConfig( client->getLocation(), client->getHost(), client )->maxClientBodySize;
	end = header.find( " ", 0 );
	client->setMethod( header.substr( 0, end ) );
	start = end + 1;
	end = header.find( " ", start );
	client->setLocation( header.substr( start, end - start ) );
    if ( ( start = header.find( "Content-Length: " ) ) != std::string::npos )
		return buildBodyForContentLength( request, start, client, MaxBody );
    else if ( ( start = header.find( "Transfer-Encoding:" ) ) != std::string::npos )
    {
        end = request.find( "\n", start );
        substr = request.substr( start, end - start );
        if ( ( start = substr.find( "chunked" ) ) != std::string::npos )
			return unchunk( request, start, client, MaxBody );
	}
    client->setRequestIsRead( true );
    std::cout << "\n\033[33m\033[1m" << "RECEIVED:\n\033[0m\033[33m" << request << "\033[0m" << std::endl;
    return ;
}
