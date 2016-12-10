#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <regex.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include "portscan.h"

#define IP_ADDRESS_REGEX "^[1-9][0-9]*\\.[1-9][0-9]*\\.[1-9][0-9]*\\.[1-9][0-9]*$"
#define DEFAULT_TIMEOUT_MICROSECONDS 1000000
#define DEBUG 1

int main(int argc, char **argv){
	if (argc < 3){
		fprintf(stderr,"Usage: %s host port [port2] [port3] [port4] ...\n",argv[0]);
		exit(1);
	}
	struct hostent *host;
	int i, sock;
	unsigned long int port_l;
	struct sockaddr_in sa;
	fd_set fdset;
	struct timeval tv;
	int timeout_microseconds = DEFAULT_TIMEOUT_MICROSECONDS;

	regex_t regex;

	char *hostname = argv[1];

	// Initialize sockaddr_in structure
	strncpy((char *)&sa, "", sizeof sa);
	sa.sin_family = AF_INET;

	if (!regcomp(&regex, IP_ADDRESS_REGEX, 0) && !regexec(&regex, hostname, 0, NULL, 0)){
		sa.sin_addr.s_addr = inet_addr(hostname);
	}
	else if ((host = gethostbyname(hostname)) != 0){
		strncpy((char *)&sa.sin_addr, (char *)host->h_addr, sizeof sa.sin_addr);
	}
	else{
		fprintf(stderr,"Cannot resolve IP/hostname.\n");
		exit(2);
	}

	for (i=2; i<argc; i++){
		// Check if number is legit 0 < N < 65536 using strtoul
		port_l = strtoul(argv[i],NULL,10);
		if (port_l < 1 || port_l > 65535){
			fprintf(stderr,"** Port %lu is not a valid port number.  Skipping it.\n",port_l);
			continue;
		}
		sa.sin_port = htons((int)port_l);
		sock = socket(AF_INET, SOCK_STREAM, 0);
		fcntl(sock, F_SETFL, O_NONBLOCK);
		if (DEBUG) fprintf(stderr," ---> Before connecting to sock here for port %lu\n",port_l);
		connect(sock, (struct sockaddr *)&sa, sizeof sa);
		if (DEBUG) fprintf(stderr," ---> After connecting to sock here for port %lu\n",port_l);

		FD_ZERO(&fdset);
		FD_SET(sock, &fdset);
		tv.tv_sec = (int)(timeout_microseconds / 1000000);
		tv.tv_usec = timeout_microseconds % 1000000;

		if (DEBUG) fprintf(stderr," ---> Before select port %lu\n",port_l);
		if (select(sock+1, NULL, &fdset, NULL, &tv) == 1){
		if (DEBUG) fprintf(stderr," ---> Could select port %lu\n",port_l);
			int so_error;
			socklen_t len = sizeof so_error;

			getsockopt(sock, SOL_SOCKET, SO_ERROR, &so_error, &len);

			if (so_error == 0){
				fprintf(stdout,"%lu open\n",port_l);
				fflush(stdout);
			}
			else{
				fprintf(stdout,"%lu closed\n",port_l);
				fflush(stdout);
			}
		}
		else{
			if (DEBUG) fprintf(stderr," ---> Could not select port %lu\n",port_l);
		}
		if (DEBUG) fprintf(stderr," ---> Closing socket now for port %lu\n",port_l);
		close(sock);
		if (DEBUG) fprintf(stderr," ---> Closed socked now for port %lu\n",port_l);
	}

	if (DEBUG) fprintf(stderr," ---> Out of loop\n");
	regfree(&regex);

}