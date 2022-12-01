#include "../inc/Socket.hpp"

Socket::Socket(std::string config, std::string path) : autoindex(false), config(config)
{
    std::size_t     pos, pos2;
    std::string     page, location, line;

    pos = this->config.find("root");
    if (pos == std::string::npos)
        exit (ft_return("No root set: "));
    pos = this->config.find("listen");
    if (pos == std::string::npos)
        exit (ft_return("No port set: "));
    try
    {
		pos2 = this->config.find(" ", pos + 7);
        port = std::stoi(this->config.substr(pos + 7, (pos2 - pos - 7)));
        logFile = "logs/port" + this->config.substr(pos + 7, (pos2 - pos - 7)) + ".log";
        ipAddr = "localhost";
        pos = this->config.find("root");
        pos2 = this->config.find(";", pos);
        _root = this->config.substr(pos + 5, (pos2 - pos - 5));
    }
    catch(std::invalid_argument const& ex)
    {
        std::cout << "std::invalid_argument::what(): " << ex.what() << '\n';
        exit (ft_return("Error reading config file"));
    }
	if ((pos = this->config.find("clientBodyMaxSize")) != std::string::npos)
	{
		pos2 = this->config.find("\n", pos);
		line = this->config.substr(pos, (pos2 - pos));
		if ((pos2 = line.find(";")) == std::string::npos | (pos = line.find(" ")) == std::string::npos)
			exit (ft_return("config error for limit client body size: "));
		this->maxClientBodySize = std::stoi(line.substr(pos + 1, (pos2 - pos + 1)));
	}
	else
		this->maxClientBodySize = -1;
	if ((pos = this->config.find("autoindex on;")) != std::string::npos)
		this->autoindex = true;
	if ((pos = this->config.find("directoryRequest")) != std::string::npos)
	{
		pos = this->config.find(" ", pos) + 1;
		pos2 = this->config.find(";", pos);
		location = this->config.substr(pos, (pos2 - pos));
		this->_pages.insert(std::make_pair("directoryRequest", location));
	}
	pos = 0;
	while ((pos = this->config.find("redirect", pos)) != std::string::npos)
	{
		pos = this->config.find(" ", pos) + 1;
		pos2 = this->config.find(" ", pos);
		page = this->config.substr(pos, (pos2 - pos));
		pos = pos2 + 1;
		pos2 = this->config.find(";", pos);
		location = this->config.substr(pos, (pos2 - pos));
		this->_redirects.insert(std::make_pair(page, location));
	}
    this->addFiles(path + "/" + _root, "");
    for(std::map<std::string, std::string>::iterator it = _pages.begin(); it != _pages.end(); ++it)
    {
        std::cout << it->first << "     " << it->second << "\n";
    }
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

    it = this->_pages.find(page);
    if (it == this->_pages.end())
        return ("");
    return (it->second);
}

std::string Socket::getRedirectPage(std::string page)
{
    std::map<std::string, std::string>::iterator	it;
	std::string	root;
	size_t		end = page.find("/", 1);
	size_t		start = 0;

	while (end != std::string::npos)
	{
		root = page.substr(start, (end - start));
		it = this->_redirects.find(root);
    	if (it != this->_redirects.end())
			page.replace(start, (end - start), it->second);
		end = page.find("/", end + 1);
	}
	it = this->_pages.find(page);
	if (it != this->_pages.end())
		return (page);
    it = this->_redirects.find(page);
    if (it == this->_redirects.end())
        return ("");
    return (it->second);
}

int Socket::addFiles(std::string path, std::string root)
{
    std::string     pathDir, page, location;
    DIR             *directory;
    struct dirent   *x;

    pathDir = path + root;
    this->_pages.insert(std::make_pair(root + "/", "Directory"));
    directory = opendir(pathDir.c_str());
    if (!directory)
        return ft_return("can not open directory: ");
    while ((x = readdir(directory)))
    {
        page = x->d_name;
        page = "/" + page;
        if (page.length() > 5 && page.substr(page.length() - 5, page.length()) == ".html")
        {
            location = pathDir + page;
            this->_pages.insert(std::make_pair(root + page, location));
            this->_pages.insert(std::make_pair(root + page.substr(0, page.length() - 5), location));
        }
        else if (page != "/." && page != "/..")
        {
            this->addFiles(path, root + page);
        }
    }
    closedir (directory);
    return (0);
}
