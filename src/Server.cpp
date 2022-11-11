#include "../inc/Server.hpp"

Server::Server() {}
Server::~Server() { std::cout << "Closing server...\n";	}

//using kqueue() in order to monitor the 3 fds
//then accepts this with socket::acceptSocket() when a client sends a request
int	Server::monitor_fd()
{
    int i, sock_num, new_event, kq, conn_fd, ret;
	std::vector<struct  kevent> chlist;         /* list of events to monitor */
	struct  kevent              tevents[40];	/* list of triggered events */

    /* create the queue */
    /* initialize kevent events structs - uses EVFILT_READ so it returns when there is data available to read */
    kq = kqueue();  
    if (kq == -1)
        return ft_return("kqueue failed");
    for (i = 0; i < 3; ++i)
        set_chlist(chlist, this->_sockets[i]->fd, EVFILT_READ, EV_ADD, 0, 0, NULL);

    /* enter run loop */
    while(true) 
    {
        /* use kevent to wait for an event (when a client tries to connect or when a connection has data to read/is open to receive data) */
        std::cout << "--waiting for events...--\n";
        new_event = kevent(kq, &chlist[0], chlist.size(), tevents, 40, NULL);
        chlist.clear();
        if (new_event < 0)
            ft_return("kevent failed:\n");
        /* kevent returned with n new events */
        else if (new_event > 0)
        {
            for (i = 0; i < new_event; i++)
            {
                int fd = (int)tevents[i].ident;
                std::cout << "handling event:" << i+1 << "/" << new_event <<  " on fd:" << fd << std::endl;
                /* EV_EOF is set if the reader on the conn_fd has disconnected */
                if (tevents[i].flags & EV_EOF)
                {
                    std::cout << "client disconnected..\n";
                    if (closeConnection(fd) == -1)
                        return -1;
                }
                else if (fd == this->_sockets[0]->fd || fd == this->_sockets[1]->fd || fd == this->_sockets[2]->fd)
                {
                    std::cout << "accepting for: " << fd << std::endl;
                    sock_num = 0;
                    while (fd != this->_sockets[sock_num]->fd && sock_num < 3)
                        sock_num++;
                    conn_fd = this->acceptRequest(sock_num);
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
                    /* now that we read the request, we can respond, so now we add an event to monitor that triggers if we can send to client */
                    set_chlist(chlist, fd, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, NULL);
                }
                else if (tevents[i].filter == EVFILT_WRITE)
                {
                    std::cout << "WRITING to:" << fd << std::endl;
                    // char msg[74] = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!";
                    // send(fd, msg, 74, 0);
                    if (this->sendResponseToClient(fd) == -1)
                        return -1;
                }
            }
        }
    }
	return -1;
}

//making the server listen to three sockets, bound do three different ports, writing requests to logfiles
int	Server::startServer()
{
	int	status = 0;

	this->_sockets.push_back(new Socket("localhost", 8093));
	this->_sockets.push_back(new Socket("localhost", 8094));
	this->_sockets.push_back(new Socket("localhost", 8095));
    if (errno == -4)
    {
        close(this->_sockets[0]->fd);
        close(this->_sockets[1]->fd);
        close(this->_sockets[2]->fd);
        this->_sockets.pop_back();
        this->_sockets.pop_back();
        this->_sockets.pop_back();
        delete this->_sockets[0];
        delete this->_sockets[1];
        delete this->_sockets[2];
        this->_sockets.push_back(new Socket("localhost", 8096));
	    this->_sockets.push_back(new Socket("localhost", 8097));
	    this->_sockets.push_back(new Socket("localhost", 8098));
    }
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
