#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <termios.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "ansi-common.h"

#define N_PERIOD 10

typedef struct {
	char *cmd[10];
	int is_pending;
} message;

volatile message mess;

void usage(char *app) {
	printf("%s <device_file>\n", app);
	exit(-1);
}

void *master_thread(void *argv) {
	int cnt, cnt1;
	int fd;
	int n;
	int rc;
	char buffer[10];
	struct termios term_settings;
	struct termios new_term_settings;

	struct timeval timeout;

	fd_set readset;
	fd_set writeset;
	fd_set exceptset;

	fd = open(argv, O_RDWR);
	ANSI_EXIT_IF_TRUE(fd < 0, "Cannot open file\n");
	tcgetattr(fd, &term_settings);
	new_term_settings = term_settings;
	//cfmakeraw(&new_term_settings);
	cfsetispeed(&new_term_settings, B9600);
	cfsetospeed(&new_term_settings, B9600);

	new_term_settings.c_cflag |= (CREAD |CLOCAL);
	new_term_settings.c_cflag &= ~CSIZE; /* Mask the character size bits */
	new_term_settings.c_cflag |= CS8;    /* Select 8 data bits */
	new_term_settings.c_cflag &= ~CRTSCTS;
	new_term_settings.c_iflag &= ~(IXON | IXOFF | IXANY);
	//options->c_lflag |= (ICANON | ECHO | ECHOE); // Canonical mode

	new_term_settings.c_lflag &= ~(ICANON | ECHO); // RAW mode
	new_term_settings.c_cc[VMIN] = 0;
	new_term_settings.c_cc[VTIME] = 5; // measured in 0.1 second
	tcsetattr(fd, TCSANOW, &new_term_settings);
	cnt = 0;
	cnt1 = 0;
	while(1) {/*
		if (cnt > 3) {
			fprintf(stdout, "Channel probing failed. Device disconnected\n");
			fflush(stdout);
			break;
		}*/
		FD_ZERO(&readset);
		FD_SET(fd, &readset);
		FD_ZERO(&writeset);
		FD_SET(fd, &writeset);
		FD_ZERO(&exceptset);
		FD_SET(fd, &exceptset);
		timeout.tv_sec = 0;
		timeout.tv_usec = 20*1000;

		rc = select(fd + 1, &readset, &writeset, &exceptset, &timeout);
		if( rc > 0 ) {
			if(FD_ISSET(fd, &readset)) {
				n = read(fd, buffer, sizeof(buffer) - 1);
				ANSI_EXIT_IF_TRUE_V2(n < 0);
				if( n > 0 ) {
					buffer[n] = '\0'; 
					fprintf(stdout, "1-%d\n", cnt);fflush(stdout);
					cnt = 0;
					write(2, buffer, n);
					fprintf(stdout,"\n");
					fflush(stdout);
	//				write(fd,(char *)mess.cmd, strlen((const char*)mess.cmd));

				}
			}
			if(FD_ISSET(fd, &writeset)) {
				if (mess.is_pending) {
					cnt1 = (cnt1 + 1) % N_PERIOD;
					if (cnt1 == 0) {
						cnt++; 
					//	fprintf(stdout, "probing %d\n", cnt);
					//	fflush(stdout);
						write(fd,(char *)mess.cmd, strlen((const char*)mess.cmd));
					//	fprintf(stdout,"\n");
					//	fflush(stdout);
	
						mess.is_pending = 0;
					}
				}
			}

		}
		usleep(100*1000);
	}
	printf("main thread end\n");
	return NULL;
}

int main(int argc, char *argv[]) {
	if( argc < 2 ) usage(argv[0]);

	int f_quit = 0;
	int len;
	char buffer[10];
	char dev[20];
	pthread_t thread;

	// UI thread
	f_quit = 0;
	pthread_create(&thread, NULL, &master_thread, (void *)argv[1]);
	strcpy(dev,argv[1]);
	while(!f_quit) {
		fgets(buffer, sizeof(buffer), stdin);
		sscanf(buffer,"%s", (char *)mess.cmd);
		mess.is_pending = 1;
	}
	pthread_join(thread, NULL);
	return 0;
}
