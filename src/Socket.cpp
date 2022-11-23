#include "../inc/Socket.hpp"

Socket::Socket(std::string config)
{
    std::size_t pos, pos2;
    std::string page, location;
    pos = config.find("listen");
    if (pos == std::string::npos)
        exit (ft_return("No port set: "));
    try
    {
        port = std::stoi(config.substr(pos + 7, 4));
        logFile = "logs/port" + config.substr(pos + 7, 4) + ".log";
        ipAddr = "localhost";
    }
    catch(std::invalid_argument const& ex)
    {
        std::cout << "std::invalid_argument::what(): " << ex.what() << '\n';
        exit (ft_return("Error reading config file"));
    }
    pos = config.find("page");
    if (pos == std::string::npos)
        exit (ft_return("No pages set: "));
    while (pos != std::string::npos)
    {
        pos2 = config.find("location", config.find("{", pos));
        if (pos2 == std::string::npos || pos2 > config.find("}", pos))
            exit (ft_return("No location set: "));
        page = config.substr(pos + 5, config.find_first_of(' ', pos + 5) - (pos + 5));
        location = config.substr(pos2 + 9, config.find_first_of(';', pos2 + 9) - (pos2 + 9));
        this->_pages.insert(std::make_pair(page, location));
        pos = config.find("page", pos + 1);
    }
    this->_pages.insert(std::make_pair("/upload.php", "htmlFiles/upload.php"));
    this->_pages.insert(std::make_pair("/404", "htmlFiles/404.html"));
    this->setupSockets();
}

Socket::Socket(std::string ipAddr, int port)
: ipAddr(ipAddr), port(port), logFile("logs/port" + std::to_string(port) + ".log")
{
    this->_pages.insert(std::make_pair("/home", "htmlFiles/home.html"));
    this->_pages.insert(std::make_pair("/", "htmlFiles/home.html"));
    this->_pages.insert(std::make_pair("/form", "htmlFiles/form.html"));
    this->_pages.insert(std::make_pair("/uploadfile", "htmlFiles/uploadfile.html"));
    this->_pages.insert(std::make_pair("/upload.php", "htmlFiles/upload.php"));
    this->_pages.insert(std::make_pair("/404", "errorPages/htmlFiles/404.html"));
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
        std::cerr << "bind failed: " << strerror(errno) << std::endl;
        close (this->fd);
        errno = -4;
        return 0;
        // std::cerr << "bind failed: " << strerror(errno) << std::endl;
        // exit(1);
    }
    if (listen(this->fd, 100))
        return (ft_return("error: listen\n"));
    return (0);
}

std::string Socket::getLocationPage(std::string page)
{
    std::map<std::string, std::string>::iterator  it;
    it = this->_pages.find(page);
    // std::cout << "page: " << it->second << std::endl;
    if (it == this->_pages.end())
        return ("");
    return (it->second);
    
}
