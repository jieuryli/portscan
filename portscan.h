void usage(char **argv,const char* extra_message){
	if (extra_message){
		fprintf(stderr,"%s\n",extra_message);
	}
	fprintf(stderr,"Usage: %s [-t timeout (s)] [-T timeout (µs)] host port [port2] [port3] [port4] ...\n",argv[0]);
}

void usage_and_die(char **argv,const char* extra_message){
	usage(argv,extra_message);
	exit(1);
}