#include "../inc/Config.hpp"

//Constructor that creates a config object for each serverblock
Config::Config( std::string config, std::string path ):	servername("default"), root("htmlFiles"), errorpages(root + "/pages/errorPages/"),
														autoindex(false), directoryRequest(""), cgi("cgi-bin"), maxClientBodySize(MAX_BODY),
														extension(".pl")
{
	size_t	pos;

	pos = config.find( "location" );
	if ( pos != std::string::npos )
		config = config.substr( 0, pos - 1 );
	methods.push_back( "GET" );
	setConfig( config );
	setPages( path + "/" + this->root, "" );
}

//When parsing specific routes in the serverblock, a new config object is created by copying the original
//then route-info is added/overwrites the original
Config::Config( const Config& src )
{
    *this = src;
}

Config & Config::operator=( const Config& src )
{
		this->servername = src.servername;
		this->errorpages = src.errorpages;
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

//All our html files are added to a map.
//The name is the key, the location on the system the value.
int Config::setPages( std::string path, std::string root )
{
    std::string     pathDir, page, location;
    DIR             *directory;
    struct dirent   *x;

    pathDir = path + root;
    this->pages.insert( std::make_pair( root + "/", "Directory" ) );
	this->pages.insert( std::make_pair( root, "Directory" ) );
    directory = opendir( pathDir.c_str() );
    if ( !directory )
	{
		std::cout << "failedpath:" << pathDir << std::endl;
        return ft_return( "can not open directory(setPages): " );
	}
    while ( ( x = readdir( directory ) ) )
    {
        page = x->d_name;
        page = "/" + page;
        location = pathDir + page;
        if ( page.length() > 5 && page.substr( page.length() - 5, page.length() ) == ".html" )
        {
            this->pages.insert( std::make_pair( root + page, location ) );
            this->pages.insert( std::make_pair( root + page.substr( 0, page.length() - 5 ), location ) );
        }
        else if ( page != "/." && page != "/.." )
        {
			std::string	temp = path + root + page;
			DIR* 		tempDir = opendir( temp.c_str() );
			if ( !tempDir )
				this->pages.insert( std::make_pair( root + page, location ) );
			else
			{
            	this->setPages( path, root + page );
				closedir( tempDir );
			}
		}
    }
    closedir( directory );
    return 0;
}

//check which options are present in the config and sets them to their values
//only redirects are done seperately (see setRouteConfigs in Socket.cpp)
void	Config::setConfig( std::string config )
{
	int						n = 9;
	std::string				line;
	std::string::iterator 	start = config.begin(), new_pos = config.begin();
	std::string 			members[9] = { "listen", "root", "errorPages", "autoindex", "directoryRequest", "cgi", "maxClientBodySize", "extension", "methods" };
	ConfigMemFn				fs[] = {	&Config::setServerName, &Config::setRoot, &Config::setErrorPages, &Config::setAutoIndex, &Config::setDirectoryRequest,
										&Config::setCGI, &Config::setMaxBodySize, &Config::setExtension, &Config::setMethods };

	for ( std::string::iterator it = config.begin(); it != config.end(); ++it ) {
		if ( *it == '\n' )
		{
			line = config.substr( std::distance( start, new_pos ), std::distance( new_pos, it ) );
			for ( int k = 0; k < n; ++k ) {
				if ( line.find( members[k] ) != std::string::npos )
					CALL_MEMBER_FN( fs[k] )( line );
			}
			new_pos = it + 1;
		}
	}
}

// servername in config is declared on first line behind the port (seperated by one space)
void	Config::setServerName( std::string line )
{
	size_t	pos = line.find( " " ) + 1;

	if ( line.find( " ", pos ) != std::string::npos )
		this->servername = line.substr( line.find( " ", pos ) + 1 );
}

// Location of default errorpages in config is declared as <errorPages location>
void	Config::setErrorPages( std::string line )
{
	this->errorpages = this->root + line.substr( line.find( " " ) + 1 );
}

// autoindex in config is declared as <autoindex on/off>
void	Config::setAutoIndex( std::string line )
{
	if ( ( line.substr( line.find( " " ) + 1 ) ) == "on" )
		this->autoindex = true;
	else
		this->autoindex = false;
}

// root in config is declared as <root location>
void	Config::setRoot( std::string line )
{
	this->root = line.substr( line.find( " " ) + 1 );
}

// default response page for a directory request in config is declared as <directoryRequest page>
void	Config::setDirectoryRequest( std::string line )
{
	this->directoryRequest = line.substr( line.find( " " ) + 1 );
}

// cgi in config is declared as <cgi location_of_cgi_scripts>
void	Config::setCGI( std::string line )
{
	this->cgi = line.substr( line.find( " " ) + 1 );
}

// MaxBodySize in config is declared as <maxClientBodySize xxx>
void	Config::setMaxBodySize( std::string line )
{
	this->maxClientBodySize = std::stoi( line.substr( line.find( " " ) + 1 ) );
	if ( this->maxClientBodySize > MAX_BODY )
		this->maxClientBodySize = MAX_BODY;
}

//extensions in config are declared as <extension .x>
void	Config::setExtension( std::string line )
{
	this->extension = line.substr( line.find( " " ) + 1 );
}

//methods in config are declared as <methods X+Y+Z>
void	Config::setMethods( std::string line )
{
	size_t		pos = 0;
	std::string	allowed_methods = line.substr( line.find( " " ) + 1 );

	this->methods.clear();
	while ( 1 )
	{
		this->methods.push_back( allowed_methods.substr( pos, allowed_methods.find( "+", pos ) - pos ) );
		pos = allowed_methods.find( "+", pos ) + 1;
		if ( pos == 0 )
			break;
	}
}

//redirects in config are declared as <redirect to_this_location>
void	Config::setRedirects(std::string route , std::string location)
{
	size_t	pos = route.find( "redirect" );
	size_t	newline;

	if ( pos != std::string::npos )
	{
		pos = route.find( " ", pos ) + 1;
		newline = route.find( "\n", pos );
		std::string redirect_to = route.substr( pos, newline - pos );
		this->redirects.insert( std::make_pair( location, redirect_to ) );
		// std::cout << "red to:" << redirect_to << std::endl;
	}
}
