#ifndef CONFIG_HPP
# define CONFIG_HPP

# include "Server.hpp"
# include "dirent.h"

class Config
{
    public:
        Config();
        ~Config();

    private:
                	Config(const Config &);
        Config &	operator=(const Config &);

};

#endif
