#include "../inc/Client.hpp"


Client::Client(int fd, int port) : conn_fd(fd), port(port)
{ 
    this->timestamp = std::time(nullptr);
    this->request_is_read = true;
    this->request_content_length = -1;
    this->chunk_number = 0;
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
    return *this;
}

void    Client::update_client_timestamp()
{
    this->timestamp = std::time(nullptr);
}
