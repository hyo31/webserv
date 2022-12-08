#include "../inc/Socket.hpp"

Socket::Socket(std::string config, std::string path) : bound(false)
{
    size_t	pos, pos2;

    try
    {
		this->serverConfig = new Config(config, path);
		pos = config.find("listen") + 7;
		pos2 = config.find(" ", pos);
        port = std::stoi(config.substr(pos, (pos2 - pos)));
        logFile = "logs/port" + config.substr(pos, (pos2 - pos)) + ".log";
        ipAddr = "localhost";
		this->setRouteConfigs(config);
    	this->setupSockets();
    }
    catch(std::invalid_argument const& ex)
    {
        std::cout << "what():" << ex.what() << std::endl;
        exit (ft_return("Error reading config file"));
    }
}
Socket::Socket(const Socket &) { std::cout << "cant copy sockets!" << std::endl; }
Socket &	Socket::operator=(const Socket &) {std::cout << "no assignment allowed for socket object!" << std::endl; return *this; }

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
		return (0);
    }
	this->bound = true;
    if (listen(this->fd, 100))
        return (ft_return("error: listen\n"));
    return (0);
}

void	Socket::setRouteConfigs(std::string & configfile)
{
	std::string	location, route;
	size_t		start = 0, end = 0;

	while ((start = configfile.find("location", end)) != std::string::npos)
	{
		Config	*routeConfig = new Config(*this->serverConfig);
		end = configfile.find("{", start) - 1;
		location = configfile.substr(start + 9, end - (start + 9));
		end = configfile.find("}", start);
		route = configfile.substr(start, (end - start));
		routeConfig->setConfig(route);
		routeConfig->setRedirects(route, location);
		this->routes.insert(std::make_pair(location, routeConfig));
	}
}

Config	*Socket::getConfig(std::string &location)
{
	std::map<std::string, Config*>::iterator it;

	it = routes.find(location);
	if (it != routes.end())
		return it->second;
	else
		return this->serverConfig;
}

std::string Socket::getLocationPage(std::string page)
{
    std::map<std::string, std::string>::iterator    it;
	Config	*config = this->getConfig(page);

    it = config->pages.find(page);
    if (it == config->pages.end())
        return ("");
    return (it->second);
}

std::string Socket::getRedirectPage(std::string page)
{
    std::map<std::string, std::string>::iterator	it;
	Config	*config = this->getConfig(page);

    it = config->redirects.find(page);
	if (it == config->redirects.end())
        return ("");
    return (it->second);
}
