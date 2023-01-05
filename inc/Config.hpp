#ifndef CONFIG_HPP
# define CONFIG_HPP

# include "Server.hpp"

class Config;

typedef  void ( Config::*ConfigMemFn )( std::string );
#define CALL_MEMBER_FN( ptrToMember )( this->*(ptrToMember ) )

class Config
{
    public:
        Config( std::string, std::string );
        ~Config();
		Config( const Config& );
    	Config  &operator=( const Config& );


		std::string					servername;
		std::string                 root;
		std::string					errorpages;
		bool						autoindex;
		std::vector<std::string>	methods;
		std::string					directoryRequest;
		std::string					cgi;
		size_t						maxClientBodySize;
		std::string					extension;

		std::map< std::string, std::string >	pages; /* name - location */	
		std::map< std::string, std::string >	redirects; /* name - redirect_location */
		void		setConfig( std::string );
		void		setRedirects( std::string, std::string );

    private:

		int			setPages( std::string , std::string );
		void		setServerName( std::string );
		void		setRoot( std::string );
		void		setErrorPages( std::string );
		void		setAutoIndex( std::string );
		void		setDirectoryRequest( std::string );
		void		setCGI( std::string );
		void		setMaxBodySize( std::string );
		void		setExtension( std::string );
		void		setMethods( std::string );
};

#endif
