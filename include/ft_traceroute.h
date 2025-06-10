#ifndef FT_TRACEROUTE_H
#define FT_TRACEROUTE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

#define MAX_TTL 30
#define PROBES 3
#define BASE_PORT 33434
#define PACKET_SIZE 60
#define TIMEOUT_SEC 1


char *ft_strncpy(char *dest, const char *src, size_t n);
int ft_strcmp(const char *str1, const char *str2);
void *ft_memset(void *b, int c, size_t len);


#endif
