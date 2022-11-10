#include "../inc/Server.hpp"


Server::Server() {}
Server::~Server() { std::cout << "Closing server...\n";	}

//making the server listen to three sockets, bound do three different ports, writing requests to logfiles
int	Server::startServer()
{
	int	status = 0;

	// this->_sockets.push_back(new Socket("localhost", 8093));
	// this->_sockets.push_back(new Socket("localhost", 8094));
	// this->_sockets.push_back(new Socket("localhost", 8095));
	this->_sockets.push_back(new Socket("localhost", 8096));
	this->_sockets.push_back(new Socket("localhost", 8097));
	this->_sockets.push_back(new Socket("localhost", 8098));

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
    int             i, sock_num, new_event, kq, conn_fd, ret;
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
        /* use kevent to wait for an event (when a client tries to connect or when a connection has data to read/is open to receive data) */
        std::cout << "--waiting for events...--\n";
        new_event = kevent(kq, NULL, 0, tevents, 40, NULL);
        if (new_event < 0)
            ft_return("kevent failed:\n");
        /* kevent returned with n new events */
        else if (new_event > 0)
        {
            for (i = 0; i < new_event; i++)
            {
                // for (int j = 0; j < new_event; ++j)
                //     std::cout << "events to handle:" << new_event << " fd for event " << j << ":" << tevents[j].ident << std::endl;
                int fd = (int)tevents[i].ident;
                std::cout << "handling event:" << i+1 << "/" << new_event <<  " on fd:" << fd << std::endl;
                /* EV_EOF is set if the reader on the conn_fd has disconnected */
                if (tevents[i].flags & EV_EOF)
                {
                    std::cout << "client disconnected..\n";
                    if (closeConnection(fd) == -1)
                        return -1;
                    // new_event = 0;
                }
                else if (fd == this->_sockets[0]->fd || fd == this->_sockets[1]->fd || fd == this->_sockets[2]->fd)
                {
                    // std::cout << "accepting for: " << fd << std::endl;
                    sock_num = 0;
                    while (fd != this->_sockets[sock_num]->fd && sock_num < 3)
                        sock_num++;
                    if (open_connection(fd) == true)
                    {
                        std::cout << "READING from:" << fd << std::endl;
                        if ((ret = this->receiveClientRequest(fd)) == -1)
                            return -1;
                        else if (ret == 0) /* now that we read the request, we can respond, so now we add an event to monitor that triggers if we can send to client */
                        {
                            EV_SET(&chevent, tevents[i].ident, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, NULL);
                            kevent(kq, &chevent, 1, NULL, 0, NULL);
                        }
                        continue;
                    }
                    conn_fd = this->acceptRequest(sock_num);
                    std::cout << "OPENED:" << conn_fd << std::endl;
                    if (conn_fd == -1)
                        return -1;
                    /* add event to monitor, triggers when server can read request from client */
                    EV_SET(&chevent, conn_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
                    kevent(kq, &chevent, 1, NULL, 0, NULL);
                    // std::cout << "have connection:\n";
                    /* coninueing loop -> new events will be in kq and enter below */
                }
                else if (tevents[i].filter == EVFILT_READ)
                {
                    std::cout << "READING from:" << fd << std::endl;
                    if ((ret = this->receiveClientRequest(fd)) == -1)
                        return -1;
                    else if (ret == 0) /* now that we read the request, we can respond, so now we add an event to monitor that triggers if we can send to client */
                    {
                        EV_SET(&chevent, tevents[i].ident, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, NULL);
                        kevent(kq, &chevent, 1, NULL, 0, NULL);
                    }  
                    // std::cout << "received:\n";
                }
                else if (tevents[i].filter == EVFILT_WRITE)
                {
                    std::cout << "WRITING to:" << fd << std::endl;
                    if (this->respondToClient(fd) == -1)
                        return -1;
                    // std::cout << "responded:\n";
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

    bytesRead = recv(c_fd, buf, 50000, 0);
    // std::cout << "read: " << bytesRead << " bytes\n";
    if (bytesRead == -1)
    {
        closeConnection(c_fd);
        return ft_return("recv failed:\n");
    }
    if (bytesRead == 0)
    {
        if (closeConnection(c_fd) == -1)
            return -1;
        return 1; 
    }
    buf[bytesRead] = '\0';
    std::map<int, int>::iterator it;
    it = this->_conn_fd.find(c_fd);
    if (it == this->_conn_fd.end())
        return ft_return("didn't find connection pair: ");
    
    this->_sockets[it->second]->logfile_fstream.open(this->_sockets[it->second]->logFile);
    this->_sockets[it->second]->logfile_fstream.clear();
    this->_sockets[it->second]->logfile_fstream.seekg(0);
    this->_sockets[it->second]->logfile_fstream << buf;
    this->_sockets[it->second]->logfile_fstream.close();

    std::cout << "\n\033[33m\033[1m" << "RECEIVED:\n\033[0m\033[33m" << buf << "\033[0m" << std::endl;
    return 0;
}

std::string Server::writeResponse(int c_fd)
{
    std::map<int, int>::iterator it;
    it = this->_conn_fd.find(c_fd);
    if (it == this->_conn_fd.end())
    {
        ft_return("didn't find connection pair: ");
        return (NULL);
    }
    this->_sockets[it->second]->logfile_fstream.open(this->_sockets[it->second]->logFile);
    if (!this->_sockets[it->second]->logfile_fstream.is_open())
    {
        ft_return("could not open logfile: ");
        return (NULL);
    }
    std::vector<std::string> head;
    std::string line;
    for (int i = 0; i < 3 && this->_sockets[it->second]->logfile_fstream.peek() != '\n' && this->_sockets[it->second]->logfile_fstream >> line; i++)
        head.push_back(line);
    if (head[0] == "GET")
    {
        std::cout << head[1] << std::endl;
        if (head[1] == "/home")
            return ("htmlFiles/home.html");
        else
            return ("htmlFiles/404.html");
    }
    head.clear();
    return ("htmlFiles/button.html");
}

int Server::respondToClient(int c_fd)
{
    int             fileSize;
    std::fstream    responseFile("response.txt");
    if (responseFile.is_open())
    {
        std::ifstream htmlFile(this->writeResponse(c_fd));
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
        ssize_t bytesSent = send(c_fd, response, fileSize, 0);
        if (bytesSent == -1)
        {
            htmlFile.close();
            responseFile.close();
            close(c_fd);
            return ft_return("error: send\n");
        }
        std::cout << "\n\033[32m\033[1m" << "RESPONDED:\n\033[0m\033[32m" << response << "\033[0m" << std::endl;
        htmlFile.close();
        responseFile.close();
    }
    else
        return ft_return("could not open response file ");
    // closeConnection(c_fd);
    return (0);
}

int Server::closeConnection(int fd)
{
    std::map<int, int>::iterator it;
    it = this->_conn_fd.find(fd);
    if (it == this->_conn_fd.end())
        ft_return("attempted close on unknown socket_pair: ");
    this->_conn_fd.erase(it);
    close(fd);
    std::cout << "disconnected from socket:" << fd << std::endl;
    return 0;
}

int ft_return(std::string str)
{
    std::cerr << str << strerror(errno) << std::endl;
    return (-1);
}

bool    Server::open_connection(int c_fd)
{
    std::map<int,int>::iterator it;
    it = this->_conn_fd.find(c_fd);
    if (it == this->_conn_fd.end())
        return false;
    return true;
}
