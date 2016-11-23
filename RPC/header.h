#include <stdio.h>
#include <stdint.h> 
#include <stdlib.h>

#define FILENAME "//afs/cs.pitt.edu/usr0/ankita/public/CS2510Prj1anm249/portmapper_ip" //file to write portmapper ip and port
#define TRY_TIME 2 // try time to do the procedure after fail
#define MAX_BUF_LEN 1500 // maximum message length to be sent on network
#define MAX_MES_LEN 1500 // maximum buffer size
#define RESEND_NUMBER 3 // for UDP asking to resend lost packets
#define TEST_UDP_LOST_PACKET_NUMBER 0 // lost packets percentage; for test only; default is 0
#define UDP_BURST_PACKET 50 // how many UDP packets are sent at once
#define UDP_BURST_WAIT_TIME  100000000// how many nano second wait between 2 UDP burst
#define THREAD_NUMBER 8 // number of thread to work on server stub and portmapper
#define NUMBER_OF_PROCEDURE 4 // number of possible procedure in servers side
#define TRANSID_TABLE_SIZE 20 //table size for transaction id table stored in server
#define SERVERLIST_TABLE_SIZE 100 //table size for server list stored in portmapper
#define SERVER_REGISTER_WAIT_TIME 100 // for SERVER_REGISTER_WAIT_TIME seconds server will send register packet to Portmapper
#define SERVER_REGISTER_TIME_OUT 200 // after SERVER_REGISTER_TIME_OUT seconds, if Portmapper do not receive register packet from server -> TIMEOUT

#define TIMEOUT1 20 // use by checkData (wait reply from the receiver to sender)
#define TIMEOUT2 2 // use by recvfrom  (wait if sender continues to send data or not), must be smaller than TIMEOUT1

int multiply(char *program_name, int program_version,
					float*a, float *b, float *c, int m , int n , int l);
int sort(char *program_name, int program_version, float *a, float *b, int n);
int min(char *program_name, int program_version, float *a, float *b, int n);
int max(char *program_name, int program_version, float *a, float *b, int n);
