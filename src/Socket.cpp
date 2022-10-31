#include "../inc/Server.hpp"

Server::Socket::Socket(std::string ipAddr, int port, std::string logFile) : _ipAddr(ipAddr), _port(port), _logFile(logFile)
{
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
    int status = fcntl(_socket, F_SETFL, O_NONBLOCK);	
    if (status == -1)
        ft_return("fcntl failed");
    memset(&_socketAddr, 0, sizeof(_socketAddr));
    _socketAddr.sin_family = AF_INET;
    _socketAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    _socketAddr.sin_port = htons(_port);
    if (bind(_socket, (struct sockaddr*)&_socketAddr, sizeof(_socketAddr)))
    {
        close (_socket);
        std::cerr << "bind failed: " << strerror(errno) << std::endl;
        exit(1);
    }
    if (listen(_socket, 100))
        return (ft_return("error: listen\n"));
    return (0);
}

//accepts client requests to server, takes the request and stored it in logfile, responds with response.txt
int Server::Socket::acceptSocket()
{
    //ACCEPT
    std::cout << this->_port << " " << this->_socket << std::endl;
    _accept = accept(_socket, (struct sockaddr*)&_socketAddr, (socklen_t *)&_socketAddrLen);
    if (_accept == -1)
        return (ft_return("error: accept\n"));
    // std::cout << "-- accepted --\n";
    return 0;
}

//prints error messages and returns to main
int ft_return(std::string str)
{
    std::cerr << str << strerror(errno) << std::endl;
    return (errno);
}

int Server::Socket::receive_from_client(int kq)
{
    struct  kevent  chevent;   /* Event to monitor */
    struct  kevent  tevent;   /* Event triggered */

    EV_SET(&chevent, _accept, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, 0);
    int new_event = kevent(kq, &chevent, 1, &tevent, 1, NULL);
    if (new_event == -1)
        ft_return("kevent 2 failed");
    if (tevent.flags & EV_EOF)
    {
        close(_accept);
        return -1;
    }
    else if (tevent.flags & EVFILT_READ)
    {
        ssize_t bytesRead;
        char    buf[50000];
    
        for (bytesRead = 0; bytesRead != -1;)
            bytesRead = recv(_accept, buf, 50000, MSG_DONTWAIT);
        if (bytesRead == 0)
        {
            close(_accept);
            return -1;
        }
        buf[bytesRead] = '\0';
        _logfile_ostream.open(this->_logFile);
        _logfile_ostream << buf;
        _logfile_ostream.close();
        // std::cout << "received:\n" << buf << std::endl;
        return 0;
    }
    return ft_return("didnt flag a read event\n");
}

int Server::Socket::respond_to_client()
{
    //RESPOND
    std::ifstream responseFile("response.txt");
    if (responseFile.is_open())
    {
        char    response[50000];
        responseFile.read(response, 50000);
        ssize_t bytesSent = send(_accept, response, 50000, MSG_DONTWAIT);
        // std::cout << "response:\n" << response << std::endl;
        if (bytesSent == -1)
        {
            close(_accept);
            return ft_return("error: send\n");
        }
    }
    responseFile.close();
    close(_accept);
    return (0);
}
