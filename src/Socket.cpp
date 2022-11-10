#include "../inc/Socket.hpp"

Socket::Socket(std::string ipAddr, int port)
: ipAddr(ipAddr), port(port), logFile("logs/port" + std::to_string(port) + ".log")
{
    this->_pages.insert(std::make_pair("/home", "htmlFiles/home.html"));
    this->_pages.insert(std::make_pair("/", "htmlFiles/home.html"));
    this->_pages.insert(std::make_pair("/form", "htmlFiles/form.html"));
    this->_pages.insert(std::make_pair("/uploadfile", "htmlFiles/uploadfile.html"));
    this->_pages.insert(std::make_pair("/upload.php", "htmlFiles/upload.php"));
    this->_pages.insert(std::make_pair("/404", "htmlFiles/404.html"));
    this->setupSockets();
}

Socket::~Socket()
{
    std::cout << "Socket:" << this->fd << " - bound to port:" << this->port << " closed\n";
    close(fd);
}

//Creates socket, fills in sockaddr_in struct in order to bind socket to address, and makes server listen to this socket
int Socket::setupSockets()
{
    this->fd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->fd == -1)
        return (ft_return("error: socket\n"));
    int status = fcntl(this->fd, F_SETFL, O_NONBLOCK);	
    if (status == -1)
        ft_return("fcntl failed");
    memset(&socketAddr, 0, sizeof(socketAddr));
    socketAddr.sin_family = AF_INET;
    socketAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    socketAddr.sin_port = htons(port);
    if (bind(this->fd, (struct sockaddr*)&socketAddr, sizeof(socketAddr)))
    {
        close (this->fd);
        std::cerr << "bind failed: " << strerror(errno) << std::endl;
        exit(1);
    }
    if (listen(this->fd, 100))
        return (ft_return("error: listen\n"));
    return (0);
}

std::string Socket::getLocationPage(std::string page)
{
    std::map<std::string, std::string>::iterator  it;
    it = this->_pages.find(page);
    std::cout << "page: " << it->second << std::endl;
    if (it == this->_pages.end())
        return ("");
    return (it->second);
    
}
