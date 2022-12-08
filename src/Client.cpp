#include "../inc/Client.hpp"


Client::Client(int fd, int port, Config  *server_config) : conn_fd(fd), port(port), server_config(server_config)
{ 
    this->timestamp = std::time(nullptr);
    this->request_is_read = true;
    this->requestContentLength = -1;
	this->requestHeader = "";
	this->requestBody = "";
	this->headerSet = false;
	this->client_body_too_large = false;
	this->headerSize = 0;
}

Client::~Client() { std::cout << "Client removed\n"; }

Client::Client(const Client& src)
{
    *this = src;
}

Client & Client::operator=(const Client& src)
{
    this->conn_fd = src.conn_fd;
    this->timestamp = src.timestamp;
    this->port = src.port;
	this->request_is_read = src.request_is_read;
    this->requestContentLength = src.requestContentLength;;
	this->requestHeader = src.requestHeader;
	this->requestBody = src.requestBody;
	this->headerSet = src.headerSet;
	this->current_route = src.current_route;
    return *this;
}

void    Client::update_client_timestamp()
{
    this->timestamp = std::time(nullptr);
}
