#include "../inc/Socket.hpp"

Socket::Socket(std::string ipAddr, int port, std::string logFile) : ipAddr(ipAddr), port(port), logFile(logFile)
{
    this->setupSockets();
}

Socket::~Socket()
{
    std::cout << "Socket:" << this->socket_fd << " - bound to port:" << this->port << " closed\n";
    close(socket_fd);
}

//Creates socket, fills in sockaddr_in struct in order to bind socket to address, and makes server listen to this socket
int Socket::setupSockets()
{
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1)
        return (ft_return("error: socket\n"));
    int status = fcntl(socket_fd, F_SETFL, O_NONBLOCK);	
    if (status == -1)
        ft_return("fcntl failed");
    memset(&socketAddr, 0, sizeof(socketAddr));
    socketAddr.sin_family = AF_INET;
    socketAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    socketAddr.sin_port = htons(port);
    if (bind(socket_fd, (struct sockaddr*)&socketAddr, sizeof(socketAddr)))
    {
        close (socket_fd);
        std::cerr << "bind failed: " << strerror(errno) << std::endl;
        exit(1);
    }
    if (listen(socket_fd, 100))
        return (ft_return("error: listen\n"));
    return (0);
}

//accepts client requests to server, takes the request and stored it in logfile, responds with response.txt
int Socket::accept_Request()
{
    //ACCEPT
    std::cout << this->port << " " << this->socket_fd << std::endl;
    accept_fd = accept(socket_fd, (struct sockaddr*)&socketAddr, (socklen_t *)&socketAddrLen);
    if (accept_fd == -1)
        return (ft_return("error: accept\n"));
    // std::cout << "-- accepted --\n";
    return 0;
}

int Socket::receive_ClientRequest(int kq)
{
    struct  kevent  chevent;   /* Event to monitor */
    struct  kevent  tevent;   /* Event triggered */

    EV_SET(&chevent, accept_fd, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, 0);
    int new_event = kevent(kq, &chevent, 1, &tevent, 1, NULL);
    if (new_event == -1)
        ft_return("kevent 2 failed");
    if (tevent.flags & EV_EOF)
    {
        close(accept_fd);
        return -1;
    }
    else if (tevent.flags & EVFILT_WRITE && tevent.flags & EVFILT_READ)
    {
        ssize_t bytesRead;
        char    buf[50000];
    
        std::cout << "---receiving---\n";
        for (bytesRead = 0; bytesRead != -1;)
            bytesRead = recv(accept_fd, buf, 50000, MSG_DONTWAIT);
        // bytesRead = recv(accept_fd, buf, 50000, MSG_DONTWAIT);
        // if (bytesRead == -1)
        // {
        //     close(accept_fd);
        //     ft_return("recv failed:\n");
        // }
        if (bytesRead == 0)
        {
            close(accept_fd);
            return -1;
        }
        buf[bytesRead] = '\0';
        logfile_ostream.open(this->logFile);
        logfile_ostream << buf;
        logfile_ostream.close();
        std::cout << "received:\n" << buf << std::endl;
        return 0;
    }
    return ft_return("didnt flag a read event\n");
}

std::string Socket::writeResponse()
{
    return ("html_files/home.html");
    return ("html_files/button.html");
    return ("html_files/404.html");
}

int Socket::respond_to_Client()
{
    //RESPOND
    int             fileSize;
    std::fstream    responseFile("response.txt");
    if (responseFile.is_open())
    {
        std::ifstream htmlFile(this->writeResponse());
        if (htmlFile.is_open())
        {
            htmlFile.seekg(0, std::ios::end);
            fileSize = htmlFile.tellg();
            htmlFile.clear();
            htmlFile.seekg(0);
        }
        else
            return (ft_return("html file doesn't exist"));
        responseFile << "HTTP/1.1 200 OK" << std::endl;
        responseFile << "Content-Type: text/html" << std::endl;
        responseFile << "Connection: keep-alive" << std::endl;
        responseFile << "Content-Length " << fileSize << std::endl << std::endl;
        char    html[fileSize];
        htmlFile.read(html, fileSize);
        responseFile << html << std::endl;
        
        responseFile.seekg(0, std::ios::end);
        fileSize = responseFile.tellg();
        responseFile.clear();
        responseFile.seekg(0);
        char    response[fileSize];
        responseFile.read(response, fileSize);
        std::cout << "response:\n" << response << std::endl;
        ssize_t bytesSent = send(accept_fd, response, fileSize, MSG_DONTWAIT);
        if (bytesSent == -1)
        {
            close(accept_fd);
            return ft_return("error: send\n");
        }
    }
    responseFile.close();
    close(accept_fd);
    return (0);
}
