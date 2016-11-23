#include <ctype.h> 
#include <stdarg.h> 
#include <inttypes.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>
#include "header.h"

// this is for header.h
#define pack754_32(f) (pack754((f), 32, 8)) 
#define pack754_64(f) (pack754((f), 64, 11)) 
#define unpack754_32(i) (unpack754((i), 32, 8)) 
#define unpack754_64(i) (unpack754((i), 64, 11))

typedef float float32_t;
typedef double float64_t;

/********************************************************/
int createTCP_server(char *addrstr, char *port);
int connectTCP_server(char *Server_ip, char *port);
int createUDP_server(char *addrstr, char *port);
int connectUDP_server(char *Server_ip, char *port, struct sockaddr_storage *their_addr, socklen_t *addr_len);
void *get_in_addr(struct sockaddr *sa);

/********************************************************/
// send and receive float data  (number,array etc)

int receiveDataUDP(int TCPsock, int sockfd, struct sockaddr_storage *their_addr, socklen_t *addr_len,
							float *a, int size1, int size2, 
								int total_packet_received, int *packet_recived_array, int try_counter);						
int sendDataUDP(int TCPsock, int sockfd, struct sockaddr_storage *their_addr, socklen_t addr_len,
					float *a, int size1, int size2);
					
int sendDataUDPLost(int sockfd, struct sockaddr_storage *their_addr, socklen_t addr_len,
					float *a, int m, int n, int16_t *packet_lost_array, int packet_lost_number);
					
int checkData(int TCPsock, int sockfd, float *a, int size1, int size2, 
					struct sockaddr_storage *their_addr, socklen_t addr_len);

/********************************************************/
//pack and marshal function
uint64_t pack754(long double f, unsigned bits, unsigned expbits);
long double unpack754(uint64_t i, unsigned bits, unsigned expbits);

void packi16(unsigned char *buf, unsigned int i);
void packi32(unsigned char *buf, unsigned long i);
unsigned int unpacki16(unsigned char *buf);
unsigned long unpacki32(unsigned char *buf);
int32_t pack(unsigned char *buf, char *format, ...);
void unpack(unsigned char *buf, char *format, ...);
int array_marshal (char **buf, float *array, int array_size);
int array_demarshal(char *buf, float *a);
/********************************************************/


