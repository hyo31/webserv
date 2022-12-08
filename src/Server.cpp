#include "../inc/Server.hpp"

Server::Server()
{
	this->_timeout.tv_sec = TIMEOUT;
	this->_timeout.tv_nsec = 0;
}
Server::Server(const Server &) { std::cout << "cant copy server!" << std::endl; }
Server &	Server::operator=(const Server &) {std::cout << "no assignment allowed for server object!" << std::endl; return *this; }

Server::~Server()
{
    std::cout << "Closing server...\n";
}

//using kqueue() in order to monitor the 3 ports
//then accepts this with socket::acceptSocket() when a client sends a request
int	Server::monitor_ports()
{
    int i, sock_num, new_event, kq, conn_fd, ret;
	std::vector<struct  kevent> chlist;         /* list of events to monitor */
	struct  kevent              tevents[42];	/* list of triggered events */

    /* create the queue */
    kq = kqueue();  
    if (kq == -1)
        return ft_return("kqueue failed");
    /* initialize kevent chlist structs - uses EVFILT_READ so it returns when there is data available to read */
    for (size_t j = 0; j < this->_sockets.size(); ++j)
        set_chlist(chlist, this->_sockets[j]->fd, EVFILT_READ, EV_ADD, 0, 0, NULL);

    /* enter run loop */
    std::cout << "\033[1m--waiting for events...--\n\033[0m";
    while(1) 
    {
		std::cout << "waiting on events...\n";
        /* use kevent to wait for an event (when a client tries to connect or when a connection has data to read/is open to receive data) */
        new_event = kevent(kq, &chlist[0], chlist.size(), tevents, 40, &this->_timeout);
        chlist.clear();
        if (new_event < 0)
            return ft_return("kevent failed: \n");
        /* kevent returned with new events */
        else if (new_event > 0)
        {
            for (i = 0; i < new_event; i++)
            {
                int fd = (int)tevents[i].ident;
                std::cout << "handling event:" << i+1 << "/" << new_event <<  " on fd:" << fd << std::endl;
                /* EV_EOF is set if the reader on the conn_fd has disconnected */
                if (tevents[i].flags & EV_EOF)
                {
                    std::cout << "Client disconnected..\n";
                    if (closeConnection(fd) == -1)
                        return -1;
                }
                else if ((sock_num = findSocket(fd))!= -1)
                {
                    std::cout << "accepting for: " << fd << std::endl;
                    conn_fd = acceptRequest(sock_num);
                    std::cout << "OPENED:" << conn_fd << std::endl;
                    if (conn_fd == -1)
                        return -1;
                    /* add event to monitor, triggers when server can read request from client through conn_fd */
                    set_chlist(chlist, conn_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
                }
                else if (tevents[i].filter == EVFILT_READ)
                {
                    std::cout << "READING from:" << fd << std::endl;
                    if ((ret = this->receiveClientRequest(fd)) == -1)
                        return -1;
                    /* request has been read, now event is added to monitor if response can be sent */
                    if (ret == 1)
						set_chlist(chlist, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
					if (ret < 2)
                        set_chlist(chlist, fd, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, NULL);
                }
                else if (tevents[i].filter == EVFILT_WRITE)
                {
                    std::cout << "WRITING to:" << fd << std::endl;
                    if (this->sendResponseToClient(fd) == -1)
                        return -1;
                }
            }
        }
		bounceTimedOutClients();
    }
	return -1;
}

int Server::openSockets(std::string configFilePath)
{
    std::ifstream               configFile;
    std::string                 line;
    std::string                 socketConfig;

    configFile.open(configFilePath);
    if (!configFile.is_open())
        return (ft_return("Error opening config file: "));
    while (std::getline(configFile, line))
    {
        if (!line.compare("{"))
        {
            while (std::getline(configFile, line))
            {
                if (!line.compare("}"))
                    break;
                socketConfig = socketConfig + line + "\n";
            }
			Socket *new_sock = new Socket(socketConfig, this->_path);
			if (new_sock->bound == true)
            	this->_sockets.push_back(new_sock);
			else
				delete new_sock;
			socketConfig.clear();
        }
    }
    configFile.close(); 
    return (0);
}

//making the server listen to three sockets, bound do three different ports, writing requests to logfiles
int	Server::startServer(std::string configFilePath, std::string path)
{
	int	status = 0;
    this->_path = path;
    if (openSockets(configFilePath) == -1)
        return (ft_return(""));
    std::cout << "\033[1mOpened sockets: \033[0m";
    for (size_t i = 0; i < this->_sockets.size(); i++)
        std::cout << this->_sockets[i]->fd << " ";
    std::cout << "\n\033[1mListening to ports: \033[0m";
    for (size_t i = 0; i < this->_sockets.size(); i++)
        std::cout << this->_sockets[i]->port << " ";
    std::cout << std::endl << std::endl;;
	status = this->monitor_ports();
	if (status == -1)
		return ft_return("monitor failed: ");
    delete this->_sockets[0];
    delete this->_sockets[1];
    delete this->_sockets[2];
	return 0;
}

