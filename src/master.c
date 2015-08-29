#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <ctype.h>
#include "ansi-common.h"

#define N_PERIOD 10

typedef struct {
	char *cmd[10];
	volatile int is_pending;
} message;

message mess;

volatile int port_number;

typedef struct {
	volatile int is_tx_on;
	volatile int is_rx_on;
	volatile int is_ptt_on;
	volatile int is_sq_on;
	volatile int is_reset;
	volatile int is_pending;
	char *status_type;
	volatile int err_code;
} riuc4_status;

riuc4_status riuc1;
riuc4_status riuc2;
riuc4_status riuc3;
riuc4_status riuc4;
riuc4_status riuc;

void riuc4_reset_status(riuc4_status *riuc) {
	riuc->is_tx_on = 0;
	riuc->is_rx_on = 0;
	riuc->is_ptt_on = 0;
	riuc->is_sq_on = 0;
	riuc->is_pending = 0;
	riuc->err_code = 0;
	riuc->status_type = '\0';
}

void riuc4_change_status(riuc4_status *riuc, char *cmd) {
	printf("change status = %s\n",cmd);
	if (strcmp(cmd,"entx") == 0) {
		if (riuc->is_pending != 0) {
			printf("Previous command is pending\n");
		}
		else {
			riuc->is_tx_on = 1;
			riuc->is_pending = 1;
			riuc->status_type = "tx";
		}
	}
	else if (strcmp(cmd,"distx") == 0) {
		if (riuc->is_pending != 0) {
			printf("Previous command is pending\n");
		}
		else {
			riuc->is_tx_on = 0;
			riuc->is_pending = 1;
			riuc->status_type = "tx";
		}
	}	
	else if (strcmp(cmd,"onptt") == 0) {	
		if (riuc->is_pending != 0) {
			printf("Previous command is pending\n");
		}
		else {
			if (riuc->is_tx_on == 1) {
				riuc->is_ptt_on = 1;
				riuc->status_type = "ptt";
			}
			else {
				riuc->status_type = "err";
				riuc->err_code = 1;
			}
			riuc->is_pending = 1;
		}
	}
	else if (strcmp(cmd,"offptt") == 0) {
		if (riuc->is_pending != 0) {
			printf("Previous command is pending\n");
		}
		else {
			if (riuc->is_tx_on == 1) {
				riuc->is_ptt_on = 0;
				riuc->status_type = "ptt";
			}
			else {
				riuc->status_type = "err";
				riuc->err_code = 1;
			}
			riuc->is_pending = 1;
		}
	}
	else if (strcmp(cmd,"enrx") == 0) {
		if (riuc->is_pending != 0) {
			printf("Previous command is pending\n");
		}
		else {
			riuc->is_rx_on = 1;
			riuc->is_pending = 1;
			riuc->status_type = "rx";
		}
	}
	else if (strcmp(cmd,"disrx") == 0) {
		if (riuc->is_pending != 0) {
			printf("Previous command is pending\n");
		}
		else {
			riuc->is_rx_on = 0;
			riuc->is_pending = 1;
			riuc->status_type = "rx";
		}
	}
	else if (strcmp(cmd,"ptt") == 0) {
		if (riuc->is_pending != 0) {
			printf("Previous command is pending\n");
		}
		else {
			riuc->is_pending = 1;
			riuc->status_type = "c_ptt";
		}
	}	
	else if (strcmp(cmd,"tx") == 0) {
		if (riuc->is_pending != 0) {
			printf("Previous command is pending\n");
		}
		else {
			riuc->is_pending = 1;
			riuc->status_type = "c_tx";
		}
	}	
	else if (strcmp(cmd,"rx") == 0) {
		if (riuc->is_pending != 0) {
			printf("Previous command is pending\n");
		}
		else {
			riuc->is_pending = 1;
			riuc->status_type = "c_rx";
		}
	}	
	else if (strcmp(cmd,"ensq") == 0) {
			if (riuc->is_pending != 0) {
			printf("Previous command is pending\n");
		}
		else {
			riuc->is_pending = 1;
			if (riuc->is_rx_on) 
				riuc->is_sq_on = 1;
			riuc->status_type = "sq";
		}
	}		
	else if (strcmp(cmd,"dissq") == 0) {
		if (riuc->is_pending != 0) {
			printf("Previous command is pending\n");
		}
		else {
			riuc->is_pending = 1;
			if (riuc->is_rx_on)
				riuc->is_sq_on = 0;

			riuc->status_type = "sq";

		}
	}	
	else if (strcmp(cmd,"sq") == 0) {
		if (riuc->is_pending != 0) {
			printf("Previous command is pending\n");
		}
		else {
			riuc->is_pending = 1;
			riuc->status_type = "c_sq";
		}
	}	
	else if (strcmp(cmd,"err") == 0) {
		if (riuc->is_pending != 0) {
			printf("Previous command is pending\n");
		}
		else {
			riuc->err_code = 0;
			riuc->is_pending = 1;
			riuc->status_type = "err";
		}
	}
	else {
		if (riuc->is_pending != 0) {
			printf("Previous command is pending\n");
		}
		else {
			riuc->err_code = 0;
			riuc->is_pending = 1;
			riuc->status_type = "err";
		}
	}
}

void riuc4_gen_status(riuc4_status riuc, int port, int fdm) {
	char status[4];
	// Gen TX status
	if (strcmp(riuc.status_type,"tx") == 0 || strcmp(riuc.status_type,"c_tx") == 0) {			
		if (riuc.is_tx_on) {
			strcpy(status, "TX1");
			status[1] = port + '0';
			fprintf(stdout, "_%s_\n", status);
			fflush(stdout);
			write(fdm, status, strlen(status));
		}					
		else {
			strcpy(status, "TX0");
			status[1] = port + '0';
			fprintf(stdout, "_%s_\n", status);
			fflush(stdout);
			write(fdm, status, strlen(status));
		}
	}
	// Gen RX status
	if (strcmp(riuc.status_type,"rx") == 0 || strcmp(riuc.status_type,"c_rx") == 0) {			
		if (riuc.is_rx_on) {
			strcpy(status, "RX1");
			status[1] = port + '0';
			fprintf(stdout, "_%s_\n", status);
			fflush(stdout);
			write(fdm, status, strlen(status));
		}
		else {
			strcpy(status, "RX0");
			status[1] = port + '0';
			fprintf(stdout, "_%s_\n", status);
			fflush(stdout);
			write(fdm, status, strlen(status));
		}
	}
	
	// Gen SQ status
	if ( riuc.is_rx_on && (strcmp(riuc.status_type,"sq") == 0 || strcmp(riuc.status_type,"c_sq") == 0)) {			
		if (riuc.is_sq_on) {
			strcpy(status, "QX1");
			status[1] = port + '0';
			fprintf(stdout, "_%s_\n", status);
			fflush(stdout);
			write(fdm, status, strlen(status));
		}
		else {
			strcpy(status, "QX0");
			status[1] = port + '0';
			fprintf(stdout, "_%s_\n", status);
			fflush(stdout);
			write(fdm, status, strlen(status));
		}
	}

	// Gen PTT status
	if (strcmp(riuc.status_type,"ptt") == 0 || strcmp(riuc.status_type,"c_ptt") == 0) {			
		if (riuc.is_ptt_on) {
			strcpy(status, "LX1");
			status[1] = port + '0';
			fprintf(stdout, "_%s_\n", status);
			fflush(stdout);
			write(fdm, status, strlen(status));
		}
		else {
			strcpy(status, "LX0");
			status[1] = port + '0';
			fprintf(stdout, "_%s_\n", status);
			fflush(stdout);
			write(fdm, status, strlen(status));
		}
	}
	//Handle error
	if (strcmp(riuc.status_type,"err") == 0) {
		if (riuc.err_code == 1)
			strcpy(status, "E1");	
		else if (riuc.err_code == 2)
			strcpy(status, "E2");
		else if (riuc.err_code == 0)
			strcpy(status, "E0");
		fprintf(stdout, "_%s_\n", status);
		fflush(stdout);
		write(fdm, status, strlen(status));
	}
}

void *master_thread(void *p_data) {
	int cnt, cnt1;
	int fdm;
	int rc;
	int n;
	char buffer[10];
	int *f_quit = 0;
	fd_set readset;
	fd_set writeset;
	struct timeval timeout;
	char pts_name[50];
	int len;
	char cmd[10];

	fdm = posix_openpt(O_RDWR);
	ANSI_EXIT_IF_TRUE(fdm < 0, "Cannot create pseudo-terminal master\n");

	rc = grantpt(fdm);
	ANSI_EXIT_IF_TRUE(rc != 0, "Cannot grantpt (change access right)\n");

	rc = unlockpt(fdm);
	ANSI_EXIT_IF_TRUE(rc != 0, "Cannot unlockpt (unlock slave side)\n");

	bzero(pts_name, sizeof(pts_name));
	//len = ptsname_r(fdm, pts_name, sizeof(pts_name));
	// printf("Pseudo terminal file: %s\n", pts_name);
	printf("Pseudo terminal file: %s\n", ptsname(fdm));

	f_quit = p_data;
	timeout.tv_sec = 0;
	timeout.tv_usec = 10*1000;
	while(!(*f_quit)) {
		FD_ZERO(&readset);
		FD_SET(fdm, &readset);
		FD_ZERO(&writeset);
		FD_SET(fdm, &writeset);
		rc = select(fdm + 1, &readset, &writeset, NULL, &timeout);
		if( rc > 0 ) {
			if(FD_ISSET(fdm, &readset)) {
				n = read(fdm, buffer, sizeof(buffer) - 1);
				if( n > 0 ) {
					buffer[n] = '\0'; 
					fprintf(stdout, "1-%d\n", cnt);fflush(stdout);
					cnt = 0;
					//len = strlen(buffer);
					printf("Received command: = %s\n",buffer);
					sscanf(buffer,"%s", cmd);
					len = strlen(cmd);
					printf("Port = %d", cmd[len-1]);
					if (isdigit(buffer[len-1])) {
						port_number = buffer[len-1] - '0';
						buffer[len-1] = '\0';
						switch(port_number) {
							case 1:
								riuc4_change_status(&riuc1, buffer);
								break;
							case 2:
								riuc4_change_status(&riuc2, buffer);
								break;
							case 3:
								riuc4_change_status(&riuc3, buffer);			
								break;
							case 4:
								riuc4_change_status(&riuc4, buffer);		
								break;

							default:
								riuc4_change_status(&riuc, "err");
								printf("Received: %d", port_number);				
								break;
						}
					}
					else {
						riuc4_change_status(&riuc, "err");
						printf("Received (wrong port): %s\n", buffer);
					}
				}
				else ANSI_EXIT_IF_TRUE_V2(n < 0);
			}
			if(FD_ISSET(fdm, &writeset)) {
				if (port_number == 1){ 	
					if (riuc1.is_pending) {
						if (riuc1.is_reset) {
							write(fdm, "ready\n",6);
							riuc1.is_reset = 0;
						}
						else {	
							riuc4_gen_status(riuc1, port_number, fdm);
						}
						riuc1.is_pending = 0;
					}
				}
				else if (port_number == 2){
					if (riuc2.is_pending) {
						if (riuc2.is_reset) {
							write(fdm, "ready\n",6);
							riuc2.is_reset = 0;
						}
						else {	
							riuc4_gen_status(riuc2, port_number, fdm);
						}
						riuc2.is_pending = 0;
					}
				}	
				else if (port_number == 3){			

					if (riuc3.is_pending) {
						if (riuc3.is_reset) {
							write(fdm, "ready\n",6);
							riuc3.is_reset = 0;
						}
						else {	
							riuc4_gen_status(riuc3, port_number, fdm);
						}
						riuc3.is_pending = 0;
					}
				}
				else if (port_number == 4){
					if (riuc4.is_pending) {
						if (riuc4.is_reset) {
							write(fdm, "ready\n",6);
							riuc4.is_reset = 0;
						}
						else {	
							riuc4_gen_status(riuc4, port_number, fdm);
						}
						riuc4.is_pending = 0;
					}
				}
				else {
					if (riuc.is_pending) {
						if (riuc.is_reset) {
							write(fdm, "ready\n",6);
							riuc.is_reset = 0;
						}
						else {	
							riuc4_gen_status(riuc, port_number, fdm);
						}
						riuc.is_pending = 0;
					}

				}
			}
		}
		usleep(100*1000);
	}
	printf("main thread end\n");
	return NULL;
}

int app_main() {
	int f_quit = 0;
	char buffer[10];
	char cmd[10];
	int len;
	pthread_t thread;

	riuc4_reset_status(&riuc1);
	riuc4_reset_status(&riuc2);
	riuc4_reset_status(&riuc3);
	riuc4_reset_status(&riuc4);	
	riuc4_reset_status(&riuc);

	// UI thread
	f_quit = 0;
	pthread_create(&thread, NULL, &master_thread, &f_quit);

	while (!f_quit) {
		fgets(buffer, sizeof(buffer), stdin);
		sscanf(buffer,"%s", cmd);
		len = strlen(cmd);
		if (isdigit(cmd[len-1])) {
			port_number = cmd[len-1] - '0';
			cmd[len-1] = '\0';
			switch(port_number) {
				case 1:
					riuc4_change_status(&riuc1, cmd);
					break;
				case 2:
					riuc4_change_status(&riuc2, cmd);
					break;
				case 3:
					riuc4_change_status(&riuc3, cmd);
					break;
				case 4:
					riuc4_change_status(&riuc4, cmd);
					break;

				default:
					riuc4_change_status(&riuc, "err");
					printf("Received: %d", port_number);
					break;
			}
		}
		else {
			riuc4_change_status(&riuc, "err");
			printf("Received: %s\n", cmd);
		}
	}
	pthread_join(thread, NULL);
	return 0;
}	

int main(int argc, char *argv[]) {
	return app_main();
}
