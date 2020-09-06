#include   <sys/types.h>   /* basic system data types */
#include   <sys/socket.h>  /* basic socket definitions */
#include   <netinet/in.h>  /* sockaddr_in{} and other Internet defns */
#include   <arpa/inet.h>   /* inet(3) functions */
#include   <errno.h>
#include   <stdio.h>
#include   <stdlib.h>
#include   <unistd.h>
#include   <string.h>

#define MAXLINE 1024
#define SA      struct sockaddr

static int read_cnt;
static char *read_ptr;
static char read_buf[MAXLINE];

static ssize_t my_read(int fd, char *ptr) {
    if (read_cnt <= 0) {
        again:
        if ((read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0) {
            if (errno == EINTR)
                goto again;
            return - 1;
        } else if (read_cnt == 0)
            return 0;
        read_ptr = read_buf;
    }

    read_cnt --;
    *ptr = *read_ptr ++;
    return 1;
}

ssize_t readline(int fd, void *vptr, size_t maxlen) {
    ssize_t n, rc;
    char c, *ptr;

    ptr = vptr;
    for ( n = 1; n < maxlen; n ++ ) {
        if ((rc = my_read(fd, &c)) == 1) {
            *ptr ++ = c;
            if (c == '\n')
                break;    /* newline is stored, like fgets() */
        } else if (rc == 0) {
            *ptr = 0;
            return (n - 1);    /* EOF, n - 1 bytes were read */
        } else
            return (- 1);        /* error, errno set by read() */
    }

    *ptr = 0;    /* null terminate like fgets() */
    return (n);
}

/* end readline */

ssize_t Readline(int fd, void *ptr, size_t maxlen) {
    ssize_t n;
    if ((n = readline(fd, ptr, maxlen)) < 0)
        perror("readline error");
    return (n);
}


void Fputs(const char *ptr, FILE *stream) {
    if (fputs(ptr, stream) == EOF)
        perror("fputs error");
}


char *Fgets(char *ptr, int n, FILE *stream) {
    char *rptr;

    if ((rptr = fgets(ptr, n, stream)) == NULL && ferror(stream))
        perror("fgets error");

    return rptr;
}

/* Write "n" bytes to a descriptor. */
//ssize_t writen(int fd, const void *vptr, size_t n) {
//    size_t nleft;
//    ssize_t nwritten;
//    const char *ptr;
//
//    ptr = vptr;
//    nleft = n;
//    while (nleft > 0) {
//        if ((nwritten = write(fd, ptr, nleft)) <= 0) {
//            if (nwritten < 0 && errno == EINTR)
//                nwritten = 0;        /* and call write() again */
//            else
//                return (- 1);            /* error */
//        }
//
//        nleft -= nwritten;
//        ptr += nwritten;
//    }
//    return (n);
//}

/* end writen */

//void Writen(int fd, void *ptr, size_t nbytes) {
//    if (writen(fd, ptr, nbytes) != nbytes)
//        perror("writen error");
//}

void str_cli(FILE *fp, int sockfd) {
    char sendline[MAXLINE], recvline[MAXLINE];

    //while (Fgets(sendline, MAXLINE, fp) != NULL) {]

    //Writen(sockfd, sendline, strlen(sendline));

    if (Readline(sockfd, recvline, MAXLINE) == 0) {
        perror("str_cli: server terminated prematurely");
        exit(0);
    }
    //Fputs(recvline, stdout);
    printf("%s", recvline);
    printf("%lu", strlen(recvline));

}

int args_handler(char *ip_address, long *port_number, char **argv, int argc) {
    if (argc == 1) {
        printf("Note: type -h for help.\n");
        return 1;
    }

    if (strcmp(argv[1], "-h") == 0) {
            printf("Usage: ./client <IPv4Address> <PortNumber(def.27)>\n");
            return 1;
        }

    strcpy(ip_address, argv[1]);
    if (argc == 3) {
        *port_number = strtol(argv[2], NULL, 10);
        return 0;
    }
    return 0;
}

void servaddr_init(struct sockaddr_in *servaddr, long port_number) {
    bzero(servaddr, sizeof(*servaddr));
    servaddr->sin_family = AF_INET;
    servaddr->sin_port = htons(port_number);
}

int socket_init(int sockfd) {
    int true = 1;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "socket error : %s\n", strerror(errno));
        return 1;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &true, sizeof(int)) == - 1) {
        fprintf(stderr, "setsockopt error: %s\n", strerror(errno));
        return 1;
    }

    return sockfd;
}

int convert_ip(int err, char *ip_address, struct sockaddr_in *servaddr) {
    if ((err = inet_pton(AF_INET, ip_address, &servaddr->sin_addr)) == - 1) {
        fprintf(stderr, "ERROR: inet_pton error for %s : %s \n", ip_address, strerror(errno));
        return 1;
    } else if (err == 0) {
        fprintf(stderr, "ERROR: Invalid format of IP address  - %s \n", ip_address);
        return 1;
    }
    return 0;
}

int socket_connect(int sockfd, struct sockaddr_in servaddr, char *ip_address, long port_number) {
    printf("Connecting to %s:%ld ...\n", ip_address, port_number);
    if (connect(sockfd, (SA *) &servaddr, sizeof(servaddr)) < 0) {
        fprintf(stderr, "Connection error : %s \n", strerror(errno));
        return 1;
    }
    printf("Connected!\n");
    return 0;
}

int get_new_port(long *new_port_number, int sockfd) {
    char recvline[MAXLINE];

    while (*new_port_number < 0 || (int) *new_port_number > 65535) {
        if (Readline(sockfd, recvline, MAXLINE) == 0) {
            perror("server terminated prematurely");
            exit(0);
        }
        *new_port_number = strtol(recvline, NULL, 10);
    }
    return 0;
}

int final_connect(int sockfd, struct sockaddr_in servaddr, long port_number, int err, char *ip_address, int *cpy_sockfd) {
    sockfd = socket_init(sockfd);
    servaddr_init(&servaddr, port_number);
    convert_ip(err, ip_address, &servaddr);

    if (socket_connect(sockfd, servaddr, ip_address, port_number) == 1)
        return 1;

    *cpy_sockfd = sockfd;
    return 0;
}


int main(int argc, char **argv) {
    int sockfd = 0;
    int cpy_sockfd = 0;
    int err = 0;

    struct sockaddr_in servaddr;
//    char recvline[MAXLINE + 1];
//    char sendline[MAXLINE + 1];
    char ip_address[256];
    long port_number = 27;
    long new_port_number = - 1;

    if (args_handler(ip_address, &port_number, argv, argc) == 1){
        return 1;
    };

    if (final_connect(sockfd, servaddr, port_number, err, ip_address, &cpy_sockfd) == 1) {
        return 1;
    }

    get_new_port(&new_port_number, cpy_sockfd);
    cpy_sockfd = 0;

    if (final_connect(sockfd, servaddr, new_port_number, err, ip_address, &cpy_sockfd) == 1) {
        return 1;
    }


    printf("%ld\n", new_port_number);

    fprintf(stderr, "OK\n");
    fflush(stderr);

    return 0;
}
