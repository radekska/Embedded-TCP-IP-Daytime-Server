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

static int	read_cnt;
static char	*read_ptr;
static char	read_buf[MAXLINE];

static ssize_t my_read(int fd, char *ptr){

	if (read_cnt <= 0) {
		again:
		if ( (read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0) {
			if (errno == EINTR)
				goto again;
			return(-1);
		} else if (read_cnt == 0)
			return(0);
		read_ptr = read_buf;
	}

	read_cnt--;
	*ptr = *read_ptr++;
	return(1);
}

ssize_t readline(int fd, void *vptr, size_t maxlen)
{
	ssize_t	n, rc;
	char	c, *ptr;

	ptr = vptr;
	for (n = 1; n < maxlen; n++) {
		if ( (rc = my_read(fd, &c)) == 1) {
			*ptr++ = c;
			if (c == '\n')
				break;	/* newline is stored, like fgets() */
		} else if (rc == 0) {
			*ptr = 0;
			return(n - 1);	/* EOF, n - 1 bytes were read */
		} else
			return(-1);		/* error, errno set by read() */
	}

	*ptr = 0;	/* null terminate like fgets() */
	return(n);
}

/* end readline */

ssize_t Readline(int fd, void *ptr, size_t maxlen)
{
	ssize_t		n;

	if ( (n = readline(fd, ptr, maxlen)) < 0)
		perror("readline error");
	return(n);
}




void Fputs(const char *ptr, FILE *stream)
{
	if (fputs(ptr, stream) == EOF)
		perror("fputs error");
}


char *Fgets(char *ptr, int n, FILE *stream)
{
	char	*rptr;

	if ( (rptr = fgets(ptr, n, stream)) == NULL && ferror(stream))
		perror("fgets error");

	return (rptr);
}

/* Write "n" bytes to a descriptor. */
ssize_t writen(int fd, const void *vptr, size_t n)
{
	size_t		nleft;
	ssize_t		nwritten;
	const char	*ptr;

	ptr = vptr;
	nleft = n;
	while (nleft > 0) {
		if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
			if (nwritten < 0 && errno == EINTR)
				nwritten = 0;		/* and call write() again */
			else
				return(-1);			/* error */
		}

		nleft -= nwritten;
		ptr   += nwritten;
	}
	return(n);
}
/* end writen */

void Writen(int fd, void *ptr, size_t nbytes)
{
	if (writen(fd, ptr, nbytes) != nbytes)
		perror("writen error");
}

void str_cli(FILE *fp, int sockfd)
{
	char	sendline[MAXLINE], recvline[MAXLINE];
	
	printf("Enter text:");

	while (Fgets(sendline, MAXLINE, fp) != NULL) {

		Writen(sockfd, sendline, strlen(sendline));

		if (Readline(sockfd, recvline, MAXLINE) == 0){
			perror("str_cli: server terminated prematurely");
			exit(0);
		}
		Fputs(recvline, stdout);
		printf("Enter text:");
	}
}

int main(int argc, char **argv)
{
	int	sockfd, n, err;
	struct sockaddr_in	servaddr;
	char recvline[MAXLINE + 1];
	char sendline[MAXLINE + 1];
	char ip_address[256];
	int port_number = 27;


	if (argc == 1){
		printf("Note: type -h for help.\n");
		return 0;
	}

	if (strcmp(argv[1], "-h") == 0){
		printf("Usage: ./client <IPv4Address> <PortNumber(def.27)>\n");
		return 0;
	}

	if (argc > 1){
		strcpy(ip_address, argv[1]);
	}

	if (argc > 2){
		port_number = atoi(argv[2]);
	}


	if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		fprintf(stderr,"socket error : %s\n", strerror(errno));
		return 1;
	}


	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port   = htons(port_number);	

	if ( (err=inet_pton(AF_INET, ip_address, &servaddr.sin_addr)) == -1){
		fprintf(stderr,"ERROR: inet_pton error for %s : %s \n", ip_address, strerror(errno));
		return 1;
	}else if(err == 0){
		fprintf(stderr,"ERROR: Invalid format of IP address  - %s \n", ip_address);
		return 1;
	}
	
	if (connect(sockfd, (SA *) &servaddr, sizeof(servaddr)) < 0){
		fprintf(stderr,"connect error : %s \n", strerror(errno));
		return 1;
	}


	
	str_cli(stdin, sockfd);		/* ECHO PART */


	fprintf(stderr,"OK\n");
	fflush(stderr);

	exit(0);
}

