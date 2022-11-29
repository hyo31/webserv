#include "../inc/Server.hpp"

int Server::acceptRequest(int sock_num)
{
    int newfd = accept(this->_sockets[sock_num]->fd,
                        (struct sockaddr*)&this->_sockets[sock_num]->socketAddr,
                        (socklen_t *)&this->_sockets[sock_num]->socketAddrLen);
    if (newfd == -1)
        return (ft_return("error: accept\n"));
    Client *newclient = new Client(newfd, sock_num);
    this->_clients.push_back(newclient);
    int status = fcntl(newfd, F_SETFL, O_NONBLOCK);	
    if (status == -1)
        ft_return("fcntl failed");
    return newfd;
}

int Server::receiveClientRequest(int c_fd)
{
	char	buf[50000];
    ssize_t bytesRead = -1;
    std::vector<Client*>::iterator it = this->_clients.begin();
    std::vector<Client*>::iterator end = this->_clients.end();
    for(; it != end; ++it)
        if (c_fd == (*it)->conn_fd)
            break ;
    if (it == end)
        return ft_return("didn't find connection pair: ");
    bytesRead = recv(c_fd, buf, 50000, 0);
    buf[bytesRead] = '\0';
    update_client_timestamp(c_fd);
    if (bytesRead == -1)
    {
        closeConnection(c_fd);
        return ft_return("recv failed:\n");
    }
    else if (bytesRead == 0)
    {
        std::cout << "0 bytes read/stream socket peer shutdown (eof)\n";
        if (closeConnection(c_fd) == -1)
            return -1;
        return 1; 
    }
	else if (bytesRead == 50000)
		std::cout << "request is too big, didn't read it all\n";
    if ((*it)->request_is_read == true)
    {
        std::cout << "clearing content\n";
        std::ofstream ofs;
        ofs.open(this->_sockets[(*it)->port]->logFile, std::ofstream::out | std::ofstream::trunc);
        ofs.close();
		(*it)->headerSet = false;
		(*it)->requestBody = "";
		(*it)->requestHeader = "";
    }
    std::ofstream ofs;
    ofs.open(this->_sockets[(*it)->port]->logFile, std::fstream::out | std::fstream::app);
    ofs << buf;
    ofs.close();
    std::ofstream asd;
    asd.open("logs/check", std::fstream::out | std::fstream::app);
    asd << buf;
    /* check if request is full*/
    chunkedRequest(this->_sockets[(*it)->port]->logFile, it);
    if ((*it)->request_is_read == true)
        return 0;
    return 1;
}

std::string Server::findHtmlFile(int c_fd)
{
    std::vector<Client*>::iterator it = this->_clients.begin();
    std::vector<Client*>::iterator end = this->_clients.end();
    for(; it != end; ++it)
        if (c_fd == (*it)->conn_fd)
            break ;
    if (it == end)
    {
        ft_return("didn't find connection pair: ");
        return (NULL);
    }
    std::fstream fstr;
    fstr.open(this->_sockets[(*it)->port]->logFile);
    if (!fstr.is_open())
    {
        ft_return("could not open logfile: ");
        return (NULL);
    }
    std::vector<std::string> head;
    std::string line;
    for (int i = 0; i < 3 && fstr.peek() != '\n' && fstr >> line; i++)
        head.push_back(line);
    if (head[0] == "GET" || head[0] == "POST")
    {
		if (head[0] == "POST" && checkMaxClientBodySize(it) == false)
		{
			_responseHeader = "HTTP/1.1 413 Request Entity Too Large";
            return ("htmlFiles/errorPages/404.html");
		}
        // std::cout << head[1] << std::endl;
        std::string ret = this->_sockets[(*it)->port]->getLocationPage(head[1]);
        if (ret != "")
        {
            _responseHeader = "HTTP/1.1 200 OK";
            return (ret);
        }
		ret = this->_sockets[(*it)->port]->getRedirectPage(head[1]);
		if (ret != "")
		{
			_responseHeader = "HTTP/1.1 301 Moved Permanently\r\nLocation: ";
			_responseHeader.append(ret);
			return ("htmlFiles/errorPages/404.html");
		}
        _responseHeader = "HTTP/1.1 404 Not Found";
        return ("htmlFiles/errorPages/404.html");
    }
    head.clear();
    return (NULL);
}

int Server::sendResponseToClient(int c_fd)
{
    int             fileSize;
    std::ifstream   htmlFile;
    std::fstream    responseFile;

    std::ofstream ofs;
    ofs.open("response.txt", std::ofstream::out | std::ofstream::trunc);
    ofs.close();
    //open streamfiles
    responseFile.open("response.txt", std::ios::in | std::ios::out | std::ios::binary);
    if (!responseFile.is_open())
        return ft_return("could not open response file ");
    htmlFile.open(this->findHtmlFile(c_fd), std::ios::in | std::ios::binary);
    if (!htmlFile.is_open())
        return (ft_return("html file doesn't exist: "));

	//get length of htmlFile
    htmlFile.seekg(0, std::ios::end);
    fileSize = htmlFile.tellg();
    htmlFile.clear();
    htmlFile.seekg(0, std::ios::beg);

    //read correct headers (first one set in 'findHtmlFile') into responseFile
    responseFile << this->_responseHeader << std::endl;
    responseFile << "Content-Type: text/html" << std::endl;
    responseFile << "Content-Length: " << fileSize << "\r\n\r\n"; //std::endl << std::endl;
    
    //create char string to read html into, which is then read into responseFile         
    char    html[fileSize];
    htmlFile.read(html, fileSize);
    responseFile << html << std::endl;

    //get length of full responseFile
    responseFile.seekg(0, std::ios::end);
    fileSize = responseFile.tellg();
    responseFile.clear();
    responseFile.seekg(0, std::ios::beg);

    //create response which is sent back to client
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
    update_client_timestamp(c_fd);
    std::cout << "\n\033[32m\033[1m" << "RESPONDED:\n\033[0m\033[32m" << std::endl << response << "\033[0m" << std::endl;
    this->_responseHeader.erase();
    htmlFile.close();
    responseFile.close();
    return (0);
}
