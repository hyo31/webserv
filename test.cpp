#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>


int main()
{

    int ret = open("x.txt", O_RDWR);
    if (ret != -1)
    {
        char *str = (char*)malloc(sizeof(char)* 1000);
        read(ret, str, 1000);
        printf("str:%s\n", str); 
        int i = 0;
        while (str[i] != '\0')
            i++;
        printf("i:%i\n", i);     
    }
    return (0);
}
