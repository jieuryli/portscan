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
#define GETOPT_ALL_POSSIBLE_ALPHA "abcdefghijklmnopqrst:uvwxyzABCDEFGHIJKLMNOPQRST:UVWXYZ"

int main(int argc, char **argv){
	int opt;
	unsigned long int timeout_microseconds = DEFAULT_TIMEOUT_MICROSECONDS;

	struct hostent *host;
	int i, sock;
	unsigned long int port_l;
	struct sockaddr_in sa;
	fd_set fdset;
	struct timeval tv;
	regex_t regex;

	while ((opt = getopt(argc,argv,GETOPT_ALL_POSSIBLE_ALPHA)) != -1){
		switch(opt){
			case 'T':
				if (strtoul(optarg,NULL,10) > 0){
					timeout_microseconds = strtoul(optarg,NULL,10);
				}
				else{
					usage_and_die(argv,"Invalid value for -T.");
				}
				break;
			case 't':
				if (strtoul(optarg,NULL,10) > 0){
					timeout_microseconds = strtoul(optarg,NULL,10) * 1000000;
				}
				else{
					usage_and_die(argv,"Invalid value for -t.");
				}
				break;
			default:
				usage_and_die(argv,"Invalid option detected.");
		}
	}

	int arg_offset = optind;

	if (argc < (2+arg_offset)) usage_and_die(argv,NULL);

	char *hostname = argv[arg_offset];

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

	for (i=(1+arg_offset); i<argc; i++){
		// Check if number is legit 0 < N < 65536 using strtoul
		port_l = strtoul(argv[i],NULL,10);
		if (port_l < 1 || port_l > 65535){
			fprintf(stdout,"%s invalid\n",argv[i]);
			fflush(stdout);
			continue;
		}
		sa.sin_port = htons((int)port_l);
		sock = socket(AF_INET, SOCK_STREAM, 0);
		fcntl(sock, F_SETFL, O_NONBLOCK);
		connect(sock, (struct sockaddr *)&sa, sizeof sa);

		FD_ZERO(&fdset);
		FD_SET(sock, &fdset);
		tv.tv_sec = (int)(timeout_microseconds / 1000000);
		tv.tv_usec = (int)(timeout_microseconds % 1000000);

		if (select(sock+1, NULL, &fdset, NULL, &tv) == 1){
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
			fprintf(stdout,"%lu closed\n",port_l);
			fflush(stdout);
		}
		close(sock);
	}

	regfree(&regex);
}