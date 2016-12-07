#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int
main(int argc, char *argv[])
{  
    int sock;  
    char ipaddr[50];  
    struct sockaddr_in *sin;  
    struct ifreq ifr_ip;     
 
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {  
		perror("socket create failed");
		return -1;
    }  
 
    memset(&ifr_ip, 0, sizeof(ifr_ip));     
    strncpy(ifr_ip.ifr_name, "ens33", sizeof(ifr_ip.ifr_name) - 1);
 
    if(ioctl(sock, SIOCGIFADDR, &ifr_ip) < 0)
    {     
		perror("ioctl failed");
		return -1;
    }

    sin = (struct sockaddr_in *)&ifr_ip.ifr_addr;
    strcpy(ipaddr, inet_ntoa(sin->sin_addr));         
 
    printf("local ip:%s \n", ipaddr);

    close(sock);  
 
    return 0;
} 
 
