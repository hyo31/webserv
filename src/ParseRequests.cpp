#include "../inc/Server.hpp"

static std::string readFileIntoString(const std::string & path)
{
    std::ifstream str(path);
    std::stringstream buff;
    buff << str.rdbuf();
    str.close();
    return buff.str();
}

void	Server::buildBodyForContentLength(std::string str, std::string::size_type ret, std::vector<Client*>::iterator it)
{
	std::string::size_type	start;
	std::string::size_type	end;
	std::string				substr;

	//find the number behind Content Length
	// ret = str.find(" ", ret) + 1;
	start = str.find(" ", ret) + 1;
	end = str.find("\r\n", start) - 1;
	substr = str.substr(start, (end - start + 1));
	(*it)->requestContentLength = std::stoi(substr);
	//check if the whole body is read
	ret = str.find("\r\n\r\n");
	start = ret + 4;
	for (end = start; str[end] != '\0'; ++end);
	// std::cout << "char read:" << (end - start)  << " cont len: " << (*it)->requestContentLength << std::endl;
	if ((int)(end - start) != (*it)->requestContentLength)
	{
		if ((int)(end - start) > (*it)->requestContentLength)
		{
			std::cout << "Content Length:" << (*it)->requestContentLength << " should be:" << end << std::endl;
			// set some value so we can return some error page ?
		}
		// else
		// 	std::cout << "didnt read full content len\nCont len:" << (*it)->requestContentLength << "\nCharacters read:" << end << std::endl;
		return ;
	}
	substr = str.substr(start, (*it)->requestContentLength);
	(*it)->requestBody.append(substr);
	(*it)->request_is_read = true;
    std::cout << "\n\033[33m\033[1m" << "RECEIVED:\n\033[0m\033[33m" << str << "\033[0m" << std::endl;
	// std::cout << "HEADER:\n" << (*it)->requestHeader << "Body:\n" << (*it)->requestBody;
	return ;
}


void	Server::unchunk(std::string str, std::string::size_type ret, std::vector<Client*>::iterator it)
{
	std::string::size_type	start;
	std::stringstream		ss;
	int						chunkSize;

	ret = str.find("\r\n\r\n");
	start = ret + 4;
	if (str.find("\r\n\r\n", start) == std::string::npos)
	{
		std::cout << "incomplete body\n" << str << std::endl;
		return ;
	}
	try
	{
		while (1)
		{
			std::string::size_type end = str.find("\r\n", start) - 1;
			std::string substr = str.substr(start, (end - start + 1));
			ss << std::hex << substr;
			ss >> chunkSize;
			ss.clear();
			if (chunkSize == 0)
				break ;
			start = end + 3;
			(*it)->requestBody.append(str, start, chunkSize);
			start = start + chunkSize + 2;
		}
		(*it)->request_is_read = true;
		std::cout << "\n\033[33m\033[1m" << "RECEIVED:\n\033[0m\033[33m" << str << "\033[0m" << std::endl;
		// std::cout << "HEADER:\n" << (*it)->requestHeader << "Body:\n" << (*it)->requestBody;
		return ;
	}
	catch(const std::exception& e)
	{
		std::cerr << "error:" << e.what() << ": Request chunks not properly written!!\n";
		// set some value so we can return some error page ?
		return ;
	}
}

void    Server::chunkedRequest(std::string path, std::vector<Client*>::iterator it)
{
    std::string 			str = readFileIntoString(path);
	std::string 			substr;
    size_t					ret, start, end;

    (*it)->request_is_read = false;
    if (str.find("\r\n\r\n") == std::string::npos)
    {
        std::cout << "Incomplete header\n" << std::endl;
        return ;
    }
	/* store full header */
	if ((*it)->headerSet == false)
	{
		end = str.find("\r\n\r\n") + 3;
		(*it)->requestHeader = str.substr(0, end);
		(*it)->requestHeader[end + 1] = '\0';
		(*it)->headerSet = true;
	}
    //check header for either Content Length: or Transfer-Encoding: chunked
    if ((ret = str.find("Content-Length:")) != std::string::npos)
		return buildBodyForContentLength(str, ret, it);
    else if ((ret = str.find("Transfer-Encoding:")) != std::string::npos)
    {
        start = ret;
        end = str.find("\n", start);
        substr = str.substr(start, (end - start));
        if ((ret = substr.find("chunked")) != std::string::npos)
			return unchunk(str, ret, it);
	}
    (*it)->request_is_read = true;
    std::cout << "\n\033[33m\033[1m" << "RECEIVED:\n\033[0m\033[33m" << str << "\033[0m" << std::endl;
	// std::cout << "HEADER:\n" << (*it)->requestHeader << "Body:\n" << (*it)->requestBody;
    return ;
}

int	Server::checkMaxClientBodySize(std::vector<Client*>::iterator client)
{
	size_t		start;
	size_t		end;
	std::string	boundary;

	if ((*client)->requestHeader.find("Content-Length:") == std::string::npos)
		return 2;
	start = (*client)->requestHeader.find("Content-Type: multipart/form-data;");
	if (start == std::string::npos)
		return 0;
	start = (*client)->requestHeader.find("=", start) + 1;
	end = (*client)->requestHeader.find("\r\n", start);
	boundary = (*client)->requestHeader.substr(start, (end - start));
	start = (*client)->requestBody.find("Content-Type: application/octet-stream") + 42;
	if ((end = (*client)->requestBody.find(boundary, start)) == std::string::npos)
		std::cout << "couldnt find a boundary :(" << std::endl;
	// std::cout << "found:" << (int)(end - 4 - start) << " max:" << this->_sockets[(*client)->port]->serverConfig->maxClientBodySize << std::endl;
	if ((int)(end - 4 - start) > this->_sockets[(*client)->port]->serverConfig->maxClientBodySize)
		return 1;
	return 0;
}
