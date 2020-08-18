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

void help_function();

int main(int argc, char **argv)
{
	int	sockfd, n, err;
	struct sockaddr_in	servaddr;
	char recvline[MAXLINE + 1];
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
	servaddr.sin_port   = htons(port_number);	/* daytime server */

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

	while ( (n = read(sockfd, recvline, MAXLINE)) > 0) {
		recvline[n] = 0;	/* null terminate */
		if (fputs(recvline, stdout) == EOF){
			fprintf(stderr,"fputs error : %s\n", strerror(errno));
			return 1;
		}
	}
	
	if (n < 0)
		fprintf(stderr,"read error : %s\n", strerror(errno));

	fprintf(stderr,"OK\n");
	fflush(stdout);
	fgetc(stdin);	
	exit(0);
}

