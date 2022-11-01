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
    close(_socket);
}

//Creates socket, fills in sockaddr_in struct in order to bind socket to address, and makes server listen to this socket
int Server::Socket::setupSockets()
{
    _socket = socket(AF_INET, SOCK_STREAM, 0);
    if (_socket == -1)
        return (ft_return("error: socket\n"));
    //fcntl(_socket, F_SETFL, O_NONBLOCK);
    _socketAddr.sin_family = AF_INET;
    _socketAddr.sin_addr.s_addr = INADDR_ANY;
    _socketAddr.sin_port = htons(_port);
    if (bind(_socket, (struct sockaddr*)&_socketAddr, _socketAddrLen))
    {
        close (_socket);
        return (ft_return("error: bind\n"));
    }
    if (listen(_socket, 100))
        return (ft_return("error: listen\n"));
    return (0);
}

//accepts client requests to server, takes the request and stored it in logfile, responds with response.txt
int Server::Socket::acceptSocket()
{
    std::cout << this->_port << " " << this->_socket << std::endl;
    char *buf = new char[5000];
    _accept = accept(_socket, (struct sockaddr*)&_socketAddr, (socklen_t *)&_socketAddrLen);
    std::cout << "-- accepted --\n";
    if (_accept == -1)
        return (ft_return("error: accept\n"));
    //fcntl(_accept, F_SETFL, O_NONBLOCK);
    ssize_t bytesRead = recv(_accept, buf, 5000, 0);
    std::cout << "=============RECEIVED=============\n";
    if (bytesRead == -1)
        return ft_return("error: recv\n");
    buf[bytesRead] = '\0';
    bytesRead = 0;
    _logfileStream.open(this->_logFile);
    _logfileStream << buf;
    std::cout << buf << std::endl;
    std::cout << "==================================\n";
    std::ifstream responseFile("404.txt");
    if (responseFile.is_open())
    {
        std::string response = std::string((std::istreambuf_iterator<char>(responseFile)), std::istreambuf_iterator<char>());
        send(_accept, response.c_str(), response.size(), 0);
    }
    close(_accept);
    responseFile.close();
    return (0);
}

int Server::Socket::get()
{
    return (0);
}

//prints error messages and returns to main
int ft_return(std::string str)
{
    std::cerr << str << strerror(errno) << std::endl;
    return (errno);
}
