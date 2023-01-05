#include "../inc/Server.hpp"

static std::string readFileIntoString( const std::string & path )
{
    std::ifstream str( path );
    std::stringstream buff;
    buff << str.rdbuf();
    str.close();
    return buff.str();
}

static size_t	getUploadBodySize( std::string body )
{
	size_t	start, end;

	start = body.find( "\r\n\r\n" ) + 4;
	end = body.find( "\r\n", start );
	return end - start;
}

static void	buildBodyForContentLength( std::string request, size_t start, Client *client )
{
	size_t		end, content_len;
	std::string	body;

	//find the number behind Content Length and check if it is over the maximum
	start = request.find( " ", start ) + 1;
	end = request.find( "\r\n", start );
	content_len = std::stoi( request.substr( start, end - start ) );
	client->setContentLength( content_len );
	// std::cout << "max: " << (*it)->server_config->maxClientBodySize << "  CL:" << (*it)->requestContentLength << std::endl;
	if ( content_len > client->getConfig()->maxClientBodySize + 1000 )
		client->setBodyTooLarge( true );
	//add new body to existing body
	start = request.find( "\r\n\r\n" ) + 4;
	body = client->getBody();
	body.append( request.substr( start, content_len ) );
	client->setBody( body );

	//check if the whole body is read
	for ( end = start; request[end] != '\0'; ++end );
	if ( end - start != content_len )
	{
		if ( end - start > content_len )
			std::cout << "error: Content Length:" << content_len << " should be:" << end << std::endl;
		else
			std::cout << "didnt read full content len\nCont len:" << content_len << "\nCharacters read:" << end << std::endl;
		return ;
	}
	client->setRequestIsRead( true );
	if ( getUploadBodySize( body ) > client->getConfig()->maxClientBodySize )
		client->setBodyTooLarge( true );
    std::cout << "\n\033[33m\033[1m" << "RECEIVED:\n\033[0m\033[33m" << request << "\033[0m" << std::endl;
	// std::cout << "HEADER:\n" << (*it)->requestHeader << "Body:\n" << body;
	return ;
}


static void	unchunk( std::string request, size_t start, Client *client )
{
	std::stringstream	ss;
	size_t 				end, chunkSize;
	std::string			substr, body;

	start = request.find( "\r\n\r\n", start ) + 4;
	if ( request.find( "\r\n\r\n", start ) == std::string::npos )
	{
		std::cout << "incomplete body\n" << request << std::endl;
		return ;
	}
	try
	{
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
			if ( body.size() > ( size_t )client->getConfig()->maxClientBodySize )
			{
				client->setBodyTooLarge( true );
				return ;
			}
			start = start + chunkSize + 2;
		}
		client->setRequestIsRead( true );
		std::cout << "\n\033[33m\033[1m" << "RECEIVED:\n\033[0m\033[33m" << request << "\033[0m" << std::endl;
		// std::cout << "HEADER:\n" << (*it)->requestHeader << "Body:\n" << body;
		return ;
	}
	catch( const std::exception& e )
	{
		std::cerr << "error:" << e.what() << ": Request chunks not properly written!!\n";
		return ;
	}
}

void    Server::parseRequest( std::string path, Client *client )
{
    std::string 			request = readFileIntoString( path );
	std::string 			substr, header;
    size_t					start, end;

    client->setRequestIsRead( false );
	/* check if full healder is read */
    if ( request.find( "\r\n\r\n" ) == std::string::npos )
    {
        std::cout << "Incomplete header\n" << std::endl;
        return ;
    }

	/* store header */
	if ( client->headerIsSet() == false )
	{
		end = request.find( "\r\n\r\n" );
		client->setHeader( request.substr( 0, end ), end );
		client->setHeaderIsSet( true );
		client->setHeaderSize( client->getHeader().size() );
	}

	// set requested method and location
	header = client->getHeader();
	start = 0;
	end = header.find( " ", start );
	client->setMethod( header.substr( start, end - start ) );
	start = end + 1;
	end = header.find( " ", start );
	client->setLocation( header.substr( start, end - start) );

    //check header for either Content Length: or Transfer-Encoding: chunked
    if ( ( start = header.find( "Content-Length:" ) ) != std::string::npos )
		return buildBodyForContentLength( request, start, client );
    else if ( ( start = header.find( "Transfer-Encoding:" ) ) != std::string::npos )
    {
        end = request.find( "\n", start );
        substr = request.substr( start, end - start );
        if ( ( start = substr.find( "chunked" ) ) != std::string::npos )
			return unchunk( request, start, client );
	}
    client->setRequestIsRead( true );
    std::cout << "\n\033[33m\033[1m" << "RECEIVED:\n\033[0m\033[33m" << request << "\033[0m" << std::endl;
	// std::cout << "HEADER:\n" << (*it)->requestHeader << "Body:\n" << (*it)->requestBody;
    return ;
}
