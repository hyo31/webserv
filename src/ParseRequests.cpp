#include "../inc/Server.hpp"

// static std::string readFileIntoString( const std::string & path )
// {
//     std::ifstream str( path );
//     std::stringstream buff;
//     buff << str.rdbuf();
//     str.close();
//     return buff.str();
// }

static size_t	getUploadBodySize( std::string body, std::string header )
{
	size_t	start, end;

	if ( header.find( "Content-Type: multipart/form-data" ) != std::string::npos )
	{
		start = body.find( "\r\n\r\n" ) + 4;
		end = body.find( "\r\n", start );
		// std::cout << "start:" << start << "   end:" << end << std::endl;
		return end - start;
	}
	return body.size();
}

static bool checkIfWholeBodyRead( std::string request )
{
	size_t		start, end;
	std::string	last_boundary;

	start = request.find( "boundary=" ) + 9;
	end = request.find( "\r\n", start );
	last_boundary = "--" + request.substr( start, end - start ) + "--";
	if ( request.find( last_boundary ) == std::string::npos )
		return false ;
	return true ;
}

static void	buildBodyBinaryContent( std::string request, size_t start, Client *client )
{
	std::string	body, header = client->getHeader();
	
	//add new body to existing body
	start = request.find( "\r\n\r\n" ) + 4;
	body = client->getBody();
	body.append( request.substr( start ) );
	client->setBody( body );
	// start = body.find( "\r\n\r\n" ) + 4;
	// start = body.find( "\r\n", start );
	// if ( start == std::string::npos )
	// 	return ;
	// std::cout << " body:\n" << body << std::endl;
	client->setRequestIsRead( true );
	if ( getUploadBodySize( body, header ) > client->getConfig()->maxClientBodySize )
		client->setBodyTooLarge( true );
    std::cout << "\n\033[33m\033[1m" << "RECEIVED:\n\033[0m\033[33m" << request << "\033[0m" << std::endl;
	// std::cout << "HEADER:\n" << (*it)->requestHeader << "Body:\n" << body;
	return ;
}

static void	buildBodyForContentLength( std::string request, size_t start, Client *client )
{
	size_t		end, content_len;
	std::string	body, header = client->getHeader();

	//find the number behind Content Length and check if it is over the maximum
	start = request.find( " ", start ) + 1;
	end = request.find( "\r\n", start );
	content_len = std::stoi( request.substr( start, end - start ) );
	client->setContentLength( content_len );
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
	// std::cout << "body:" << body << std::endl << getUploadBodySize( body, header ) << "  " << client->getConfig()->maxClientBodySize  << std::endl;
	client->setRequestIsRead( true );
	if ( getUploadBodySize( body, header ) > client->getConfig()->maxClientBodySize )
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
			if ( body.size() > client->getConfig()->maxClientBodySize )
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

void    Server::parseRequest( std::string request, Client *client )
{
    // std::string	request = readFileIntoString( path );
	std::string	substr, header;
    size_t		start, end;

    client->setRequestIsRead( false );
	// std::cout << "request:" << request << std::endl;
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

	// std::cout << "request:\n" << request << std::endl;
	//check if it is multipart/form-data -> if so, check if whole body is received
	if ( ( start = header.find( "Content-Type: multipart/form-data" ) ) != std::string::npos )
	{
		if ( checkIfWholeBodyRead( request ) == false )
			return ;
	}

    //check header for either Content Length: or Transfer-Encoding: chunked
    if ( ( start = header.find( "Content-Length:" ) ) != std::string::npos )
	{
		if ( request.find( "Content-Type: application/octet-stream" ) != std::string::npos || request.find( "Content-Type: image/png" ) != std::string::npos )
			return buildBodyBinaryContent( request, start, client );
		return buildBodyForContentLength( request, start, client );
	}
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
