#include "../inc/Server.hpp"

int Server::acceptRequest(int sock_num)
{
    int conn_fd = accept(this->_sockets[sock_num]->fd,
                        (struct sockaddr*)&this->_sockets[sock_num]->socketAddr,
                        (socklen_t *)&this->_sockets[sock_num]->socketAddrLen);
    if (conn_fd == -1)
        return (ft_return("error: accept\n"));
    Client *newclient = new Client(conn_fd, sock_num, this->_sockets[sock_num]->serverConfig);
    this->_clients.push_back(newclient);
    int status = fcntl(conn_fd, F_SETFL, O_NONBLOCK);
    if (status == -1)
        ft_return("fcntl failed:");
    return conn_fd;
}

int Server::receiveClientRequest(int c_fd)
{
    std::vector<Client*>::iterator	it = this->_clients.begin();
    std::vector<Client*>::iterator	end = this->_clients.end();
    ssize_t							bytesRead = -1;
	char							buf[5000];

    for(; it != end; ++it)
        if (c_fd == (*it)->conn_fd)
            break ;
    bytesRead = recv(c_fd, buf, 5000, 0);
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
	else if (bytesRead == 5000)
		std::cout << "request is too big, require another read\n";
    if ((*it)->request_is_read == true)
    {
		std::cout << "clearing content..\n";
        std::ofstream ofs;
        ofs.open(this->_sockets[(*it)->port]->logFile, std::ofstream::out | std::ofstream::trunc);
        ofs.close();
		(*it)->headerSet = false;
		(*it)->requestBody.clear();
		(*it)->requestHeader.clear();
    }
    std::ofstream ofs;
    ofs.open(this->_sockets[(*it)->port]->logFile, std::fstream::out | std::fstream::app);
    ofs << buf;
    ofs.close();
    std::ofstream ofs2;
    ofs2.open("logs/check", std::fstream::out | std::fstream::app);
    ofs2 << buf;
	ofs2.close();
    /* check if request is full*/
    parseRequest(this->_sockets[(*it)->port]->logFile, it);
	if ((*it)->request_is_read == false && (*it)->client_body_too_large == false)
		return 2;
	if ((*it)->client_body_too_large == true)
		return 1;
    return 0;
}

void    removeResponseFiles()
{
    DIR             *directory;
    directory = opendir("response");
    if (!directory)
        ft_return("can not open directory (removeResonseFiles): ");
    for (struct dirent *dirEntry = readdir(directory); dirEntry; dirEntry = readdir(directory))
    {
        std::string link = std::string(dirEntry->d_name);
        if (link != "." && link != "..")
        {
            link = "response/" + link;
            std::remove(link.c_str());
        }
    }
	closedir(directory);
}

int Server::sendResponseToClient(int c_fd)
{
    int             fileSize;
    std::string     htmlFileName;
    std::ifstream   htmlFile;
    std::fstream    responseFile;
    std::ofstream 	ofs;
    std::vector<Client*>::iterator	it = this->_clients.begin();
    std::vector<Client*>::iterator	end = this->_clients.end();

    for(; it != end; ++it)
        if (c_fd == (*it)->conn_fd)
            break ;
	if (it == end)
		return 0;
    ofs.open("response/response.txt", std::ofstream::out | std::ofstream::trunc);
    ofs.close();
    //open streamfiles
    responseFile.open("response/response.txt", std::ios::in | std::ios::out | std::ios::binary);
    if (!responseFile.is_open())
        return ft_return("could not open response file ");
    htmlFileName = this->findHtmlFile(c_fd);
    if (!htmlFileName.size())
        htmlFileName = this->_sockets[(*it)->port]->serverConfig->errorpages + "500.html";
    htmlFile.open(htmlFileName, std::ios::in | std::ios::binary);
    if (!htmlFile.is_open())
    {
        std::cout << "html file:" << htmlFileName << "  doesn't exist!" << std::endl;
        this->_responseHeader = "HTTP/1.1 403 Forbidden";
        htmlFile.open( this->_sockets[(*it)->port]->serverConfig->errorpages + "403.html", std::ios::in | std::ios::binary);
		if (!htmlFile.is_open())
			htmlFile.open( "htmlFiles/pages/errorPages/403.html", std::ios::in | std::ios::binary);
    }

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
	this->_responseHeader.clear();
	htmlFile.close();
	responseFile.close();
	std::ifstream ifs("response/responseCGI.html");
	if (ifs.good())
		ifs.close();
	removeResponseFiles();
	if ((*it)->client_body_too_large == true)
		closeConnection(c_fd);
    return (0);
}
