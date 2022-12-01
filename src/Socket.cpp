#include "../inc/Socket.hpp"

Socket::Socket(std::string config, std::string path)
{
    size_t	pos, pos2;

	this->serverConfig = new Config(config, path, false);
    try
    {
		pos = config.find("listen");
		pos2 = config.find(" ", pos + 7);
        port = std::stoi(config.substr(pos + 7, (pos2 - pos - 7)));
        logFile = "logs/port" + config.substr(pos + 7, (pos2 - pos - 7)) + ".log";
        ipAddr = "localhost";
    }
    catch(std::invalid_argument const& ex)
    {
        std::cout << "std::invalid_argument::what(): " << ex.what() << '\n';
        exit (ft_return("Error reading config file"));
    }
	this->setRouteConfigs(config);
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
		return (0);
        exit(1);
    }
    if (listen(this->fd, 100))
        return (ft_return("error: listen\n"));
    return (0);
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
	std::string	root;
	size_t		end = page.find("/", 1);
	size_t		start = 0;
	Config	*config = this->getConfig(page);

	while (end != std::string::npos)
	{
		root = page.substr(start, (end - start));
		it = config->redirects.find(root);
    	if (it != config->redirects.end())
			page.replace(start, (end - start), it->second);
		end = page.find("/", end + 1);
	}
	it = config->pages.find(page);
	if (it != config->pages.end())
		return (page);
    it = config->redirects.find(page);
    if (it == config->redirects.end())
        return ("");
    return (it->second);
}

void	Socket::setRouteConfigs(std::string & configfile)
{
	std::string	location, route, redirect_page, autoindex, method;
	size_t		start = 0;
	size_t		end, route_end;

	while ((start = configfile.find("location", start)) != std::string::npos)
	{
		Config	*routeConfig = new Config(*this->serverConfig);

		start = start + 9;
		end = configfile.find("{", start) - 1; 
		location = configfile.substr(start, (end - start));
		start = end + 1;
		end = configfile.find("}", start);
		route_end = end;
		route = configfile.substr(start, (end - start));
		start = 0;
		if ((start = route.find("redirect", start)) != std::string::npos)
		{
			start = route.find(" ", start) + 1;
			end = route.find(";", start);
			redirect_page = route.substr(start, (end - start));
			routeConfig->redirects.insert(std::make_pair(redirect_page, location));
		}
		start = 0;
		if ((start = route.find("autoindex", start)) != std::string::npos)
		{
			start = route.find(" ", start) + 1;
			end = route.find(";", start);
			autoindex = route.substr(start, (end - start));
			if (autoindex.compare("on") == 0)
				routeConfig->autoindex = true;
			else
				routeConfig->autoindex = false;
		}
		start = 0;
		if ((start = route.find("methods", start)) != std::string::npos)
		{
			start = route.find(" ", start) + 1;
			end = start;
			while (end != std::string::npos)
			{
				end = route.find("+", end);
				if (end != std::string::npos)
				{
					method = route.substr(start, (end - start));
					routeConfig->methods.push_back(method);
					start = end + 1;
					end = start;
				}
			}
		}
		start = 0;
		if ((start = route.find("root", start)) != std::string::npos)
		{
			start = route.find(" ", start) + 1;
			end = route.find(";", start);
			routeConfig->root = route.substr(start, (end - start));
		}
		start = 0;
		if ((start = route.find("directoryRequest", start)) != std::string::npos)
		{
			start = route.find(" ", start) + 1;
			end = route.find(";", start);
			routeConfig->directoryRequest = route.substr(start, (end - start));
		}
		start = 0;
		if ((start = route.find("cgi", start)) != std::string::npos)
		{
			start = route.find(" ", start) + 1;
			end = route.find(";", start);
			routeConfig->cgi = route.substr(start, (end - start));
		}
		this->routes.insert(std::make_pair(location, routeConfig));
		start = route_end;
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
