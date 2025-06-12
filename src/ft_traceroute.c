#include "../include/ft_traceroute.h"

void print_probe_results(int success_count, char ip_strs[][INET_ADDRSTRLEN], double rtts[]) {
    if (success_count == PROBES) {
        printf("%s  %.3f ms  %.3f ms  %.3f ms\n", ip_strs[0], rtts[0], rtts[1], rtts[2]);
    } else if (success_count > 0) {
        printf("%s  ", ip_strs[0]);
        for (int i = 0; i < success_count; i++)
            printf("%.3f ms  ", rtts[i]);

        for (int i = success_count; i < PROBES; i++)
            printf("* ");
        printf("\n");
    } else {
        printf("* * *\n");
    }
}


double time_diff(struct timeval *start, struct timeval *end) {
    return (end->tv_sec - start->tv_sec) * 1000.0 +
           (end->tv_usec - start->tv_usec) / 1000.0;
}

int analyze_icmp_packet(char *buffer, int expected_port, struct sockaddr_in *from, double *rtt, char *ip_str, struct timeval *send_time, int *reached) {
    struct ip *ip_hdr = (struct ip *)buffer;
    int ip_header_len = ip_hdr->ip_hl * 4;
    struct icmp *icmp_hdr = (struct icmp *)(buffer + ip_header_len);

    if (icmp_hdr->icmp_type == ICMP_TIME_EXCEEDED || 
        icmp_hdr->icmp_type == ICMP_DEST_UNREACH) {
    
        struct ip *inner_ip = (struct ip *)(icmp_hdr->icmp_data);
        int inner_ip_len = inner_ip->ip_hl * 4;
        struct udphdr *inner_udp = (struct udphdr *)((char *)inner_ip + inner_ip_len);
    
        if (ntohs(inner_udp->uh_dport) == expected_port) {
            ft_strncpy(ip_str, inet_ntoa(from->sin_addr), INET_ADDRSTRLEN);
            struct timeval recv_time;
            gettimeofday(&recv_time, NULL);
            *rtt = time_diff(send_time, &recv_time);
            *reached = (icmp_hdr->icmp_type == ICMP_DEST_UNREACH);
            return 1;
        }
    } else if (icmp_hdr->icmp_type == ICMP_ECHOREPLY) {
        ft_strncpy(ip_str, inet_ntoa(from->sin_addr), INET_ADDRSTRLEN);
        struct timeval recv_time;
        gettimeofday(&recv_time, NULL);
        *rtt = time_diff(send_time, &recv_time);
        *reached = 1;
        return 1;
    }
     
    return 0;
}

ssize_t wait_and_receive(int sock, char *buffer, size_t size, struct sockaddr_in *from) {
    fd_set readfds;
    
    struct timeval timeout = {TIMEOUT_SEC, 0};
    socklen_t from_len = sizeof(*from);
    FD_ZERO(&readfds);
    FD_SET(sock, &readfds);

    int sel = select(sock + 1, &readfds, NULL, NULL, &timeout);
    
    if (sel <= 0)
        return -1;

    ssize_t len = recvfrom(sock, buffer, size, 0, (struct sockaddr *)from, &from_len);

    if (len < 0)
        return -1;

    return len;
}



int receive_icmp_summary(int sock, int expected_port, double *rtt, char *ip_str, struct timeval *send_time, int *reached) {
    char buffer[512];
    struct sockaddr_in from;
    
    ssize_t len;
    len = wait_and_receive(sock, buffer, sizeof(buffer), &from);

    int result;
    if (len <= 0) {
        result = 0;
    } else {
        result = analyze_icmp_packet(buffer, expected_port, &from, rtt, ip_str, send_time, reached);
    }

    return result;
}


int send_udp_probe(int sock, struct sockaddr_in *addr, int port, struct timeval *send_time) {
    char buffer[PACKET_SIZE];
    ft_memset(buffer, 0, sizeof(buffer));
    addr->sin_port = htons(port);

    gettimeofday(send_time, NULL);
    if (sendto(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)addr, sizeof(*addr)) < 0) {
        fprintf(stderr, "sendto: %s\n", strerror(errno));
        return -1;
    }

    return 0;
}

void handle_hop(int send_sock, int recv_sock, struct sockaddr_in *target, int ttl, int *reached) {
    char buf[16];
    int len = snprintf(buf, sizeof(buf), "%d", ttl);
    write(STDOUT_FILENO, buf, len);

    write(STDOUT_FILENO, "  ", 2);

    double rtts[PROBES] = {0};
    char ip_strs[PROBES][INET_ADDRSTRLEN] = {{0}};
    int success_count = 0;

    for (int probe = 0; probe < PROBES; probe++) {
        int port = (BASE_PORT + ttl * PROBES) + probe;
        
        //ENVOI
        struct timeval send_time;
        if (send_udp_probe(send_sock, target, port, &send_time) < 0)
            continue;


        //RECEPTION
        double rtt;
        char ip_str[INET_ADDRSTRLEN] = {0};
        int success = receive_icmp_summary(recv_sock, port, &rtt, ip_str, &send_time, reached);

        struct timeval pause = {0, 100000};
        select(0, NULL, NULL, NULL, &pause);

        if (success) {
            ft_strncpy(ip_strs[success_count], ip_str, INET_ADDRSTRLEN);
            rtts[success_count++] = rtt;
        }
    }

    print_probe_results(success_count, ip_strs, rtts);
}


void run_traceroute(int send_sock, int recv_sock, struct sockaddr_in *target) {
    int reached = 0; 
    for (int ttl = 1; ttl <= MAX_TTL && !reached; ttl++) {
        if (setsockopt(send_sock, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0) {
            perror("setsockopt");
            break;
        }
        handle_hop(send_sock, recv_sock, target, ttl, &reached);
    }
}

void print_header(const char *input, struct sockaddr_in *target) {
    printf("ft_traceroute to %s (%s), %d hops max, %d byte packets\n",
           input,                
           inet_ntoa(target->sin_addr), 
           MAX_TTL,              
           PACKET_SIZE);         
}

int init_sockets(int *send_sock, int *recv_sock) {
    *send_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (*send_sock < 0) {
        perror("socket send");
        return -1;
    }

    *recv_sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (*recv_sock < 0) {
        perror("socket recv");
        close(*send_sock);
        return -1;
    }
    return 0;
}


int resolve_target(const char *input, struct sockaddr_in *target) {
    ft_memset(target, 0, sizeof(*target));
    target->sin_family = AF_INET; 

    if (inet_pton(AF_INET, input, &target->sin_addr) == 1)
        return 0;

    struct addrinfo hints;     
    struct addrinfo *res;    
    ft_memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    int err = getaddrinfo(input, NULL, &hints, &res);
    if (err != 0) {
        return -1;
    }

    *target = *(struct sockaddr_in *)res->ai_addr; 
    freeaddrinfo(res);
    return 0;
}

void print_help(void) {
    printf("Usage: ft_traceroute [--help] <IPv4 address or hostname>\n");
}

int parse_args(int argc, char *argv[])
{
    if (argc < 2) {
        print_help();
        exit(0);
    } 
    if (argc > 2) {
        for (int i = 2; i <= argc; i++) {
                if (i == argc - 1 && i <= 2)
                    {
                        fprintf(stderr,
                            "Cannot handle \"packetlen\" cmdline arg `%s' on position %d (argc %d)\n",
                            argv[i], i, argc - 1);
                        exit(0);  
                    }
                else
                    {
                        fprintf(stderr,
                        "Extra arg `%s' (position 3, argc 3)\n",
                        argv[3]);
                        exit(0);
                    }
        }
    }

    else if (ft_strcmp(argv[1], "--help") == 0) {
        print_help();
        exit(0);
    }

    return 0;
}

int main(int argc, char *argv[]) {

    parse_args(argc, argv);

    struct sockaddr_in target;
    if (resolve_target(argv[1], &target) < 0)
        {
            fprintf(stderr, "%s: Temporary failure in name resolution\n", argv[1]);
            fprintf(stderr, "Cannot handle \"host\" cmdline arg `%s' on position 1 (argc 1)\n", argv[1]);
            exit(0);
        }

    int send_sock, recv_sock;
    if (init_sockets(&send_sock, &recv_sock) < 0)
        exit(0);

    print_header(argv[1], &target);
    run_traceroute(send_sock, recv_sock, &target);

    close(send_sock);
    close(recv_sock);
    return EXIT_SUCCESS;
}
