#ifndef CONFIG_HPP
# define CONFIG_HPP

# include "Server.hpp"

class Config
{
    public:
        Config(std::string, std::string, bool);
        ~Config();
		Config(const Config&);
    	Config  &operator=(const Config&);


		std::string					servername;
		std::string					errorpages;
		std::string                 configfile;
		bool						autoindex;
		std::vector<std::string>	methods;
		std::string                 root;
		std::string					directoryRequest;
		std::string					cgi;
		int							maxClientBodySize;
		std::string					extension;

		std::map<std::string, std::string>	pages; /* name - location */	
		std::map<std::string, std::string>	redirects; /* name - redirect_location */

    private:

		int		addFiles(std::string path, std::string location);

};

#endif
