#include "../inc/Server.hpp"

Server::Socket::Socket(std::string ipAddr, int port, std::string logFile) : _ipAddr(ipAddr), _port(port), _socket(),
                                                                            _socketAddr(), _socketAddrLen(sizeof(_socketAddr)),
                                                                            _logFile(logFile)
{
    std::cout << "Socket started\n";
    this->setupSockets();
}

Server::Socket::~Socket()
{
    std::cout << "Socket:" << this->_socket << " - bound to port:" << this->_port << " closed\n";
    _logFile.close();
    close(_socket);
}

//Creates socket, fills in sockaddr_in struct in order to bind socket to address, and makes server listen to this socket
int Server::Socket::setupSockets()
{
    _socket = socket(AF_INET, SOCK_STREAM, 0);
    if (_socket == -1)
        return (ft_return("error: socket\n"));
    _socketAddr.sin_family = AF_INET;
    _socketAddr.sin_addr.s_addr = INADDR_ANY;
    _socketAddr.sin_port = htons(_port);
    if (bind(_socket, (struct sockaddr*)&_socketAddr, _socketAddrLen))
        return (ft_return("error: bind\n"));
    if (listen(_socket, 4))
        return (ft_return("error: listen\n"));
    return (0);
}

//accepts client requests to server, takes the request and stored it in logfile, responds with response.txt
int Server::Socket::acceptSocket()
{
    std::cout << this->_port << " " << this->_socket << std::endl;
    std::vector<char> buf(5000);
    _accept = accept(_socket, (struct sockaddr*)&_socketAddr, (socklen_t *)&_socketAddrLen);
    if (_accept == -1)
        return (ft_return("error: accept\n"));
    ssize_t bytesRead = recv(_accept, buf.data(), buf.size(), 0);
    if (bytesRead == -1)
        return (this->ft_return("error: recv\n"));
    buf[bytesRead] = '\0';
    _logFile <<  buf.data();
    std::ifstream responseFile("response.txt");
    if (responseFile.is_open())
    {
        char *response = new char[1024];
        responseFile.read(response, 1024);
        send(_accept, response, 1024, 0);
    }
    close(_accept);
    responseFile.close();
    return (0);
}

//prints error messages and returns to main
int Server::Socket::ft_return(std::string str)
{
    std::cerr << str << strerror(errno) << std::endl;
    return (errno);
}
