#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <regex.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "portscan.h"

#define IP_ADDRESS_REGEX "^[1-9][0-9]*\\.[1-9][0-9]*\\.[1-9][0-9]*\\.[1-9][0-9]*$"

int main(int argc, char **argv){
	if (argc < 3){
		fprintf(stderr,"Usage: %s host port [port2] [port3] [port4] ...\n",argv[0]);
		exit(1);
	}
	struct hostent *host;
	int err, i, sock;
	unsigned long int port_l;
	struct sockaddr_in sa;

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

		if (sock < 0) {
			fprintf(stderr,"Problem obtaining socket for port %lu",port_l);
			continue;
		}

		err = connect(sock, (struct sockaddr *)&sa, sizeof sa);

		if (err < 0) {
			// Can't connect
			fprintf(stdout,"%lu closed\n",port_l);
		}
		else{
			// Sucess, can connect
			fprintf(stdout,"%lu open\n",port_l);
		}

		fflush(stdout);
	}

	regfree(&regex);

}