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
    if ((*it)->request_is_read == false)
        return 1;
    
    return 0;
}

void    removeResponseFiles()
{
    DIR             *directory;
    directory = opendir("response");
    if (!directory)
        ft_return("can not open directory: ");
    for (struct dirent *dirEntry = readdir(directory); dirEntry; dirEntry = readdir(directory))
    {
        std::string link = std::string(dirEntry->d_name);
        if (link != "." && link != "..")
        {
            link = "response/" + link;
            std::remove(link.c_str());
        }
    }
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
    ofs.open("response/response.txt", std::ofstream::out | std::ofstream::trunc);
    ofs.close();
    //open streamfiles
    responseFile.open("response/response.txt", std::ios::in | std::ios::out | std::ios::binary);
    if (!responseFile.is_open())
        return ft_return("could not open response file ");
    htmlFileName = this->findHtmlFile(c_fd);
    if (!htmlFileName.size())
        htmlFileName = "htmlFiles/Pages/errorPages/500.html";
    htmlFile.open(htmlFileName, std::ios::in | std::ios::binary);
    if (!htmlFile.is_open())
    {
        ft_return("html file doesn't exist: ");
        htmlFile.open("htmlFiles/Pages/errorPages/403.html", std::ios::in | std::ios::binary);
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
	this->_responseHeader.erase();
	htmlFile.close();
	responseFile.close();
	std::ifstream ifs("response/responseCGI.html");
	if (ifs.good())
		ifs.close();
	removeResponseFiles();
    return (0);
}
