#include "../inc/Server.hpp"


Server::Server() {}
Server::~Server() { std::cout << "Closing server...\n";	}

//making the server listen to three sockets, bound do three different ports, writing requests to logfiles
int	Server::startServer()
{
	int	status = 0;

	// this->_sockets.push_back(new Socket("localhost", 8093, "logs/port8093.log"));
	// this->_sockets.push_back(new Socket("localhost", 8094, "logs/port8094.log"));
	// this->_sockets.push_back(new Socket("localhost", 8095, "logs/port8095.log"));
	this->_sockets.push_back(new Socket("localhost", 8096, "logs/port8096.log"));
	this->_sockets.push_back(new Socket("localhost", 8097, "logs/port8097.log"));
	this->_sockets.push_back(new Socket("localhost", 8098, "logs/port8098.log"));

    std::cout << "opened sockets:" << this->_sockets[0]->fd << " " << this->_sockets[1]->fd << " " << this->_sockets[2]->fd << std::endl;
    std::cout << "listening to ports:" << this->_sockets[0]->port << " " << this->_sockets[1]->port << " " << this->_sockets[2]->port << std::endl;
	status = this->monitor_fd();
	if (status == -1)
		return ft_return("monitor failed:\n");
	close(this->_sockets[0]->fd);
	close(this->_sockets[1]->fd);
	close(this->_sockets[2]->fd);
    delete this->_sockets[0];
    delete this->_sockets[1];
    delete this->_sockets[2];

	return 0;
}

//using kqueue() in order to monitor the 3 fds
//then accepts this with socket::acceptSocket() when a client sends a request
int	Server::monitor_fd()
{
    int             i, sock_num, new_event, kq, conn_fd;
	struct  kevent  chevent;        /* Events to monitor */
	struct  kevent  tevents[40];	/* Triggered event*/

    /* create the queue */
    kq = kqueue();  
    if (kq == -1)
        return ft_return("kqueue failed");

    /* initialize kevent events structs - uses EVFILT_READ so it returns when there is data available to read */
    for (i = 0; i < 3; ++i)
    {
        EV_SET(&chevent, this->_sockets[i]->fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
        if (kevent(kq, &chevent, 1, NULL, 0, NULL) < 0)
            ft_return("kevent failed:");
    }
    while(true) 
    {
        /* use kevent to wait for an event */
        std::cout << "--waiting on connection...--\n";
        new_event = kevent(kq, NULL, 0, tevents, 40, NULL);
        if (new_event < 0)
            ft_return("kevent failed:\n");
        /* kevent returned with n new events */
        else if (new_event > 0)
        {
            for (i = 0; i < new_event; i++)
            {
                for (int j = 0; j < new_event; ++j)
                    std::cout << "events to handle:" << new_event << " fd for event " << j << ":" << tevents[j].ident << std::endl;
                int fd = (int)tevents[i].ident;
                /* EV_EOF is set if the reader on the conn_fd has disconnected */
                if (tevents[i].flags & EV_EOF)
                {
                    std::cout << "Disconnecting.." << fd << std::endl;
                    close(fd);
                    new_event = 0;
                }
                else if (fd == this->_sockets[0]->fd || fd == this->_sockets[1]->fd || fd == this->_sockets[2]->fd)
                {
                    // std::cout << "accepting for: " << fd << std::endl;
                    sock_num = 0;
                    while (fd != this->_sockets[sock_num]->fd && sock_num < 3)
                        sock_num++;
                    conn_fd = this->acceptRequest(sock_num);
                    // std::cout << "opened:" << ret << std::endl;
                    if (conn_fd == -1)
                        return -1;
                    /* waiting for connection to be read/writable */
                    EV_SET(&chevent, conn_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
                    kevent(kq, &chevent, 1, NULL, 0, NULL);
                    std::cout << "have connection:\n";
                    /* coninueing loop -> new events will be in kq and enter below */
                }
                else if (tevents[i].filter == EVFILT_READ)
                {
                    std::cout << "READING\n";
                    if (this->receiveClientRequest(fd) == -1)
                        return -1;
                    EV_SET(&chevent, tevents[i].ident, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, NULL);
                    kevent(kq, &chevent, 1, NULL, 0, NULL);
                    // std::cout << "received:\n";
                }
                else if (tevents[i].filter == EVFILT_WRITE)
                {
                    if (this->respondToClient(fd) == -1)
                        return -1;
                    // std::cout << "responded:\n";
                    break ;
                }
            }
        }
    }
	return -1;
}

//accepts client requests to server, returns the fd
int Server::acceptRequest(int sock_num)
{
    //ACCEPT
    int newfd = accept(this->_sockets[sock_num]->fd,
                        (struct sockaddr*)&this->_sockets[sock_num]->socketAddr,
                        (socklen_t *)&this->_sockets[sock_num]->socketAddrLen);
    if (newfd == -1)
        return (ft_return("error: accept\n"));
    this->_conn_fd.insert(std::make_pair(newfd, sock_num));
    int status = fcntl(newfd, F_SETFL, O_NONBLOCK);	
    if (status == -1)
        ft_return("fcntl failed");
    return newfd;
}

int Server::receiveClientRequest(int c_fd)
{
    ssize_t bytesRead = -1;
    char    buf[50000];

    bytesRead = recv(c_fd, buf, 50000, MSG_DONTWAIT);
    // std::cout << "read: " << bytesRead << " bytes\n";
    if (bytesRead == -1)
    {
        close(c_fd);
        return ft_return("recv failed:\n");
    }
    if (bytesRead == 0)
    {
        close(c_fd);
        return ft_return("recv read 0:\n");
    }
    buf[bytesRead] = '\0';

    std::map<int, int>::iterator it;
    it = this->_conn_fd.find(c_fd);
    if (it == this->_conn_fd.end())
        return ft_return("didn't find connection pair: ");
    
    this->_sockets[it->second]->logfile_ostream.open(this->_sockets[it->second]->logFile);
    this->_sockets[it->second]->logfile_ostream << buf;
    this->_sockets[it->second]->logfile_ostream.close();

    std::cout << "received:\n" << buf << std::endl;
    return 0;
}

std::string Server::writeResponse()
{
    return ("htmlFiles/home.html");
    return ("htmlFiles/button.html");
    return ("htmlFiles/404.html");
}

int Server::respondToClient(int c_fd)
{
    int             fileSize;
    std::fstream    responseFile("response.txt");
    if (responseFile.is_open())
    {
        std::ifstream htmlFile(this->writeResponse());
        if (htmlFile.is_open())
        {
            htmlFile.seekg(0, std::ios::end);
            fileSize = htmlFile.tellg(); fileSize--;
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
        fileSize = responseFile.tellg(); fileSize--;
        responseFile.clear();
        responseFile.seekg(0);
        char    response[fileSize];
        responseFile.read(response, fileSize);
        ssize_t bytesSent = send(c_fd, response, fileSize, MSG_DONTWAIT);
        if (bytesSent == -1)
        {
            close(c_fd);
            return ft_return("error: send\n");
        }
        std::cout << "responded:\n" << response << std::endl;
        responseFile.close();
    }
    else
        return ft_return("could not open response file ");
    close(c_fd);
    return (0);
}

int ft_return(std::string str)
{
    std::cerr << str << strerror(errno) << std::endl;
    return (-1);
}
