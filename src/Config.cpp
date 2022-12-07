#include "../inc/Config.hpp"


Config::Config(std::string configfile, std::string path, bool ai) : configfile(configfile), autoindex(ai), root(""), directoryRequest(""), cgi(""), maxClientBodySize(-1)
{
	std::size_t     pos, pos2;
    std::string     page, location, line, default_part;

	default_part = configfile;
	pos = configfile.find("location");
	if (pos != std::string::npos)
		default_part = configfile.substr(0, (pos - 1));
    pos = default_part.find("root");
    if (pos == std::string::npos)
        root = "htmlFiles";
	else
	{
		pos2 = default_part.find(";", pos);
		root = default_part.substr(pos + 5, (pos2 - pos - 5));
	}
	pos = default_part.find("clientBodyMaxSize");
	if (pos != std::string::npos)
	{
		pos2 = default_part.find("\n", pos);
		line = default_part.substr(pos, (pos2 - pos));
		pos2 = line.find(";");
		pos = line.find(" ");
		if (pos2 == std::string::npos | pos == std::string::npos)
			exit (ft_return("config error for limit client body size: "));
		this->maxClientBodySize = std::stoi(line.substr(pos + 1, (pos2 - pos + 1)));
	}
	else
		this->maxClientBodySize = -1;
	pos = default_part.find("autoindex on;");
	if (pos != std::string::npos)
		this->autoindex = true;
	pos = default_part.find("cgi ");
	this->cgi = default_part.substr(pos + 4, default_part.find(";", pos) - (pos + 4));
	pos = default_part.find("errorPages ");
	if (pos != std::string::npos)
	{
		pos = pos + 11;
		pos2 = default_part.find(";", pos);
		this->errorpages = this->root + default_part.substr(pos, (pos2 - pos));
	}
	else
		this->errorpages = "htmlFiles/pages/errorPages";
	pos = default_part.find("extension ");
	if (pos != std::string::npos)
	{
		pos = pos + 10;
		pos2 = default_part.find(";", pos);
		this->extension = default_part.substr(pos, (pos2 - pos));
	}
	else
		this->extension = ".pl";

    this->addFiles(path + "/" + root, "");
	methods.push_back("GET");
}
Config::Config(const Config& src)
{
    *this = src;
}

Config & Config::operator=(const Config& src)
{
		this->servername = src.servername;
		this->errorpages = src.errorpages;
		this->configfile = src.configfile;
		this->autoindex = src.autoindex;
		this->methods = src.methods;
		this->root = src.root;
		this->directoryRequest = src.directoryRequest;
		this->cgi = src.cgi;
		this->maxClientBodySize = src.maxClientBodySize;
		this->extension = src.extension;
		this->pages = src.pages;
		this->redirects = src.redirects;
		return *this;
}

Config::~Config() { std::cout << "Config removed\n"; }

int Config::addFiles(std::string path, std::string root)
{
    std::string     pathDir, page, location;
    DIR             *directory;
    struct dirent   *x;

    pathDir = path + root;
    this->pages.insert(std::make_pair(root + "/", "Directory"));
	this->pages.insert(std::make_pair(root, "Directory"));
    directory = opendir(pathDir.c_str());
    if (!directory)
        return ft_return("can not open directory(addFiles): ");
    while ((x = readdir(directory)))
    {
        page = x->d_name;
        page = "/" + page;
        location = pathDir + page;
        if (page.length() > 5 && page.substr(page.length() - 5, page.length()) == ".html")
        {
            this->pages.insert(std::make_pair(root + page, location));
            this->pages.insert(std::make_pair(root + page.substr(0, page.length() - 5), location));
        }
        else if (page != "/." && page != "/..")
        {
			std::string	temp = path + root + page;
			DIR* 		tempDir = opendir(temp.c_str());
			if (!tempDir)
				this->pages.insert(std::make_pair(root + page, location));
			else
			{
            	this->addFiles(path, root + page);
				closedir(tempDir);
			}
		}
    }
    closedir(directory);
    return (0);
}
