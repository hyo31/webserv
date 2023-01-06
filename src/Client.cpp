#include "../inc/Client.hpp"

//CONSTRUCTOR + DESTRUCTOR
Client::Client(int fd, int port, Config  *server_config) : conn_fd(fd), port(port), server_config(server_config)
{ 
    this->timestamp = std::time(nullptr);
    this->request_is_read = true;
	this->client_body_too_large = false;
	this->requestHeader = "";
	this->requestBody = "";
	this->headerSet = false;
}
Client::~Client() { std::cout << "Client removed\n"; }

//COPY CONSTRUCTOR
Client::Client(const Client& src) { *this = src; }

//ASSIGNMENT OPERATOR
Client & Client::operator=(const Client& src)
{
    this->conn_fd = src.conn_fd;
    this->timestamp = src.timestamp;
    this->port = src.port;
	this->request_is_read = src.request_is_read;
	this->requestHeader = src.requestHeader;
	this->requestBody = src.requestBody;
	this->headerSet = src.headerSet;
	this->current_route = src.current_route;
    return *this;
}

//CLIENT FUNCTIONS

//UTILS
void    Client::update_client_timestamp()
{
    this->timestamp = std::time(nullptr);
}

//GETTERS
std::string	Client::getLocation()		{ return this->requestLocation; }
std::string	Client::getBody()			{ return this->requestBody; }
std::string	Client::getHeader()			{ return this->requestHeader; }
std::string	Client::getMethod()			{ return this->requestMethod; }
int			Client::getConnectionFD()	{ return this->conn_fd; }
int			Client::getPort()			{ return this->port; }
bool		Client::requestIsRead()		{ return this->request_is_read; }
bool		Client::headerIsSet()		{ return this->headerSet; }
bool		Client::bodyTooLarge()		{ return this->client_body_too_large; }
bool		Client::illegalRequest()	{ return this->illegal_request; }
Config		*Client::getConfig()		{ return this->server_config; }
std::time_t	Client::getTimeStamp()		{ return this->timestamp; }

//SETTERS
void	Client::setBody( std::string body )					{ this->requestBody = body; }
void	Client::setHeader( std::string header, int end )	{ this->requestHeader = header; this->requestHeader[end] = '\0'; }
void	Client::setMethod( std::string method )				{ this->requestMethod = method; }
void	Client::setLocation( std::string location )			{ this->requestLocation = location; }
void	Client::setHeaderIsSet( bool status )				{ this->headerSet = status; }
void	Client::setRequestIsRead( bool status )				{ this->request_is_read = status; }
void	Client::setBodyTooLarge( bool status )				{ this->client_body_too_large = status; }
void	Client::setIllegalRequest( bool status )			{ this->illegal_request = status; }
