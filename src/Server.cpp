#include "../inc/Server.hpp"


Server::Server() {}
Server::~Server() { std::cout << "Closing server...\n";	}

//making the server listen to three sockets, bound do three different ports, writing requests to logfiles
int	Server::startServer()
{
	int	status = 0;

	// this->_sockets.push_back(new Socket("localhost", 8093, "port8093.log"));
	// this->_sockets.push_back(new Socket("localhost", 8094, "port8094.log"));
	// this->_sockets.push_back(new Socket("localhost", 8095, "port8095.log"));
	this->_sockets.push_back(new Socket("localhost", 8096, "port8093.log"));
	this->_sockets.push_back(new Socket("localhost", 8097, "port8094.log"));
	this->_sockets.push_back(new Socket("localhost", 8098, "port8095.log"));

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
    int             i, sock_num, new_event, kq, ret;
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
                //-----//
                // for (int j = 0; j < new_event; ++j)
                    // std::cout << "events to handle:" << new_event << " fd for event " << j << ":" << tevents[j].ident << std::endl;
                //-----//
                int fd = (int)tevents[i].ident;
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
                    ret = this->accept_Request(sock_num);
                    // std::cout << "opened:" << ret << std::endl;
                    if (ret == -1)
                        return -1;
                    /* waiting for connection to be read/writable */
                    EV_SET(&chevent, ret, EVFILT_READ, EV_ADD, 0, 0, NULL);
                    kevent(kq, &chevent, 1, NULL, 0, NULL);
                    std::cout << "have connection:\n";
                    EV_SET(&chevent, ret, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, NULL);
                    kevent(kq, &chevent, 1, NULL, 0, NULL);
                    /* coninueing loop -> new events will be in kq and enter below */
                }
                else if (tevents[i].filter == EVFILT_READ)
                {
                    // std::cout << "READING\n";
                    if (this->receive_ClientRequest(fd) == -1)
                        return -1;
                    // std::cout << "received:\n";
                }
                else if (tevents[i].filter == EVFILT_WRITE)
                {
                    if (this->respond_to_Client(fd) == -1)
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
int Server::accept_Request(int sock_num)
{
    //ACCEPT
    int newfd = accept(this->_sockets[sock_num]->fd,
                        (struct sockaddr*)&this->_sockets[sock_num]->socketAddr,
                        (socklen_t *)&this->_sockets[sock_num]->socketAddrLen);
    if (newfd == -1)
        return (ft_return("error: accept\n"));
    Connection *newcon = new Connection(newfd);
    this->_connections.push_back(newcon);
    int status = fcntl(newfd, F_SETFL, O_NONBLOCK);	
    if (status == -1)
        ft_return("fcntl failed");
    return newcon->fd;
}

int Server::receive_ClientRequest(int c_fd)
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
    // this->_sockets[sock_num]->logfile_ostream.open(this->_sockets[sock_num]->logFile);
    // this->_sockets[sock_num]->logfile_ostream << buf;
    // this->_sockets[sock_num]->logfile_ostream.close();
    // std::cout << "received:\n" << buf << std::endl;
    return 0;
}

int Server::respond_to_Client(int c_fd)
{
    std::ifstream responseFile("response.txt");
    if (responseFile.is_open())
    {
        responseFile.seekg(0, std::ios::end);
        int file_size = responseFile.tellg();
        responseFile.clear();
        responseFile.seekg(0);
        char    response[file_size - 1];
        responseFile.read(response, file_size);
        // std::cout << "response:\n" << response << std::endl;
        ssize_t bytesSent = send(c_fd, response, file_size, MSG_DONTWAIT);
        // std::cout << "bytes sent:" << bytesSent << std::endl;
        if (bytesSent == -1)
        {
            close(c_fd);
            std::cout << "tried send to:" << c_fd << std::endl;
            return ft_return("error: send\n");
        }
        responseFile.close();
        close(c_fd);
        // std::cout << "closed " << c_fd << std::endl;
        return (0);

    }
    std::cout << "couldnt open responsefile " << c_fd << std::endl;
    return (-1);
}

int ft_return(std::string str)
{
    std::cerr << str << strerror(errno) << std::endl;
    return (-1);
}
