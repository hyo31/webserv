#include "../inc/Server.hpp"

std::string Server::createAutoIndex(std::string page, std::string folder)
{
    std::ofstream   autoindexFile;
    DIR             *directory;

    autoindexFile.open("response/autoindex.html", std::ofstream::out | std::ofstream::trunc);
    if (!autoindexFile.is_open())
    {
        ft_return("failed to create autoindex: ");
        return ("htmlFiles/Pages/errorPages/404.html");
    }
    directory = opendir(page.c_str());
    if (!directory)
    {
        ft_return("can not open directory: ");
        return ("htmlFiles/Pages/errorPages/404.html");
    }
    autoindexFile <<
    "<!DOCTYPE html>\n\
    <html lang=\"en\">\n\
    <head>\n\
    <title>Webserv</title>\n\
    </head>\n\
    <body\">\n\
    <h1>" + folder + "</h1><br>" << std::endl;
    for (struct dirent *dirEntry = readdir(directory); dirEntry; dirEntry = readdir(directory))
    {
        std::string link = std::string(dirEntry->d_name);
        if (link != "." && link != "..")
        {
            if (link.length() > 5 && link.substr(link.length() - 5, link.length()) == ".html")
                autoindexFile << "<a href=\"" << folder + link << "\">/" << link << "</a><br>" << std::endl;
            else
                autoindexFile << "<a href=\"" << folder + link << "/\">/" << link << "/</a><br>" << std::endl;
        }
    }
    autoindexFile <<\
    "</body>\n\
    </html>" << std::endl;
    return ("response/autoindex.html");
}
