#include "network_function.h"

/* parameter to each thread */
struct thread_info 
{
	int connfd;
	int thread_id;
};

struct register_info
{
	char program_name[100];
	int program_version;
	char procedure_name[100]; 
	char server_ip[100];
	char server_port[100];
}; 

/* store requests the client has sent before */
struct transid_table
{
	int transid;
	float *result;
	int size1;
	int size2;
};

pthread_t threads[THREAD_NUMBER];
int thread_available[THREAD_NUMBER];
pthread_mutex_t running_mutex;
int service[NUMBER_OF_PROCEDURE];
struct transid_table transid_table[TRANSID_TABLE_SIZE];
int table_counter = 0;


/***************************/
/* read from the global file for ip address and port number of the port mapper */
int readPortmaperIP (char *server_ip, char *server_port)
{
	// read IP address and port from file defined in header.h
	FILE *ptr_file;
	char buf[100];
	
	// read the global file
	ptr_file =fopen(FILENAME, "r");
	if (!ptr_file)
	{
		return 0;
		printf("no file open");
	}
	if(fgets(buf,1000, ptr_file)!=NULL)
	{
		// ip address
		strcpy(server_ip, strtok(buf, ","));

		// port number
		strcpy(server_port, strtok(NULL, ","));
	}
	
	if(server_ip && server_port)
		printf("portmapper ip and port is %s %s\n", server_ip, server_port);
		
	fclose(ptr_file);
	
	return 1;
}
	
/* thread */
void *thread_work(void *arg)
{
	char server_ip[100], server_port[100], serverUDP_ip[100], serverUDP_port[100] ;
	char procedure_name[20], buf[MAX_BUF_LEN];	
	int connfd, UDPsock,  numbytes, packetsize, thread_id;
	int16_t transid =0, size1 = 0, size2 = 0, size3 = 0, size4 = 0, size5 = 0, size6 = 0;
	float *a, *b, *c;
	struct sockaddr_storage their_addr;
	char TCPclient_addr[50];
	socklen_t TCPaddr_len, addr_len;
	struct thread_info *info;
	int task = 0, i, same_transid =0; 
	
	info = (struct thread_info*) arg;
	
	connfd = info->connfd;
	thread_id = info->thread_id;
	
	if (recv(connfd,buf,MAX_BUF_LEN,0) == -1)
	{
		perror ("receive error");
	}
	unpack(buf,"hshhhhhh", &transid, procedure_name, 
				&size1, &size2, &size3, &size4, &size5, &size6);
				
	printf("Receive transid: %d, procedure name: %s, size: %d %d %d %d %d %d\n", 
							transid, procedure_name, size1 , size2, size3, size4, size5, size6);
				
	if (strcmp(procedure_name,"multiply") == 0)
	{	
		if (service[0] == 1) 
			printf("/**** DO MULTIPLY1  ****/ \n");
		else 
		{
				pack (buf,"h",(int16_t)0);
				send(connfd, buf, sizeof(buf), MSG_NOSIGNAL);
				pthread_exit(NULL);
		}	
	}
	else if (strcmp(procedure_name,"sort") == 0)
	{	
		if (service[1] == 1) 
			printf("/**** DO SORT  ****/ \n");
		else 
		{
			pack (buf,"h",(int16_t)0);
			send(connfd, buf, sizeof(buf), MSG_NOSIGNAL);
			pthread_exit(NULL);
		}	
	}
	else if (strcmp(procedure_name,"min") == 0)
	{	
		if (service[2] == 1) 
			printf("/**** DO MIN  ****/ \n");
		else 
		{
			pack (buf,"h",(int16_t)0);
			send(connfd, buf, sizeof(buf), MSG_NOSIGNAL);
			pthread_exit(NULL);
		}	
	}
	else if (strcmp(procedure_name,"max") == 0)
	{	
		if (service[3] == 1) 
			printf("/**** DO MAX  ****/ \n");
		else 
		{
			pack (buf,"h",(int16_t)0);
			send(connfd, buf, sizeof(buf), MSG_NOSIGNAL);
			pthread_exit(NULL);
		}
	}
			
	UDPsock = createUDP_server(serverUDP_ip, serverUDP_port);
	addr_len = sizeof their_addr;
				
	// check if transid and client ip is the same, if yes -> send saved results
	for (i=0; i< TRANSID_TABLE_SIZE; i++)
	{
		if (transid_table[i].transid == transid)
		{
			printf("client already requested, send result only\n");
			memset(buf,0,sizeof(buf));
			packetsize = pack(buf, "hs", (int16_t) 2, serverUDP_port); // reply code 0: transaction id already
			
			// send reply code to client
			if ((numbytes = send(connfd, buf, MAX_MES_LEN,MSG_NOSIGNAL)) == -1)
			{
				perror("send UDP port to client");
				pthread_mutex_lock (&running_mutex);
				thread_available[thread_id] = 0; // set thread as available
				pthread_mutex_unlock (&running_mutex);
				pthread_exit(NULL);
			}
			
			// get client address
			if (recvfrom(UDPsock, buf,MAX_MES_LEN, 0, (struct sockaddr *) &their_addr, &addr_len) == -1)
			{
				perror("listening from client");
				pthread_mutex_lock (&running_mutex);
				thread_available[thread_id] = 0;
				pthread_mutex_unlock (&running_mutex);
				pthread_exit(NULL);
			}
			
			// send back the data
			sendDataUDP(connfd, UDPsock, &their_addr, addr_len, transid_table[i].result
										, transid_table[i].size1, transid_table[i].size2);
						
			same_transid = 1;
			printf("/**** END SERVICE FROM CLIENT  ****/ \n");
		}
	}
	// new client 			
	if (same_transid == 0)
	{
		a = (float *) calloc (size1*size2, sizeof(float));
		b = (float *) calloc (size3*size4, sizeof(float));
		c = (float *) calloc (size5*size6, sizeof(float));
			
		printf("Server UDP IP and port is: %s %s\n", serverUDP_ip, serverUDP_port);
						
		if (strcmp(procedure_name,"multiply") == 0)
		{	
			// receive parameters
			receiveDataUDP(connfd, UDPsock, &their_addr, &addr_len, a, size1, size2, 0, NULL, 0);
			
			doMultiply(a,b,c,size1, size2, size4);
			memset(buf, 0, sizeof(buf));
			sendDataUDP(connfd, UDPsock, &their_addr, addr_len, c, size5, size6);
		}
		else if (strcmp(procedure_name,"sort") == 0)
		{	
			// receive parameters
			receiveDataUDP(connfd, UDPsock, &their_addr, &addr_len, a, size1, size2, 0, NULL, 0);
			
			doSort(a,c,size2);
			memset(buf, 0, sizeof(buf));
			sendDataUDP(connfd, UDPsock, &their_addr, addr_len, c, size5, size6);
		}
		else if (strcmp(procedure_name,"min") == 0)
		{	
			// receive parameters
			receiveDataUDP(connfd, UDPsock, &their_addr, &addr_len, a, size1, size2, 0, NULL, 0);
		
			doMin(a,c,size2);
			memset(buf, 0, sizeof(buf));
			size5 = 1;
			size6 = 1;
			sendDataUDP(connfd, UDPsock, &their_addr, addr_len, c, size5, size6);
		}
		else if (strcmp(procedure_name,"max") == 0)
		{
			// receive parameters
			receiveDataUDP(connfd, UDPsock, &their_addr, &addr_len, a, size1, size2, 0, NULL, 0);
			
			doMax(a,c, size2);
			memset(buf, 0, sizeof(buf));
			size5 = 1;
			size6 = 1;
			sendDataUDP(connfd, UDPsock, &their_addr, addr_len, c, size5, size6);
		}
		
		
		free(a); 
		free(b);
		free(c);
						
	} 	
	close(connfd);
	}

/* to accept new request from client */
int runServer(sockfd)
{
	int connfd, thread_counter = 0, i;
	struct sockaddr_storage client_addr;
	socklen_t addr_len;
	struct thread_info *t;
	pthread_attr_t attr;
	char s[INET6_ADDRSTRLEN];
	
	// Prevent SIGPIPE
	signal (SIGPIPE, SIG_IGN);
	// thread stuff initialize
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	pthread_mutex_init (&running_mutex, NULL);	
	// initialize all threads as available
	for (i = 0; i< THREAD_NUMBER; i++)
		thread_available[THREAD_NUMBER] = 0;
	
	// initialize transaction id table
	for (i = 0; i< TRANSID_TABLE_SIZE; i++)
		transid_table[i].transid = 0;
		
	t = (struct thread_info *) malloc(sizeof(struct thread_info));	
	
	addr_len = sizeof(client_addr);
	while (1)
	{
		// Accept new request for a connection
		connfd = accept(sockfd,NULL, NULL);

		printf("new client connecting\n");
			
		if (connfd == -1)
		{
			perror("server: accept");
		}
	
		// create thread here
		while (1)
		{
			thread_counter++;
			if (thread_counter > THREAD_NUMBER-1)
			{
					thread_counter = 0;
					sleep_burst();
			}
			if (thread_available[THREAD_NUMBER] == 0)
				break;
		}
		
		// TODO: if not work use (long)
		t->connfd = connfd;
		t->thread_id = thread_counter;
			
		/* create the new thread */
		if (pthread_create(&threads[thread_counter], &attr, thread_work, (void *)t) != 0) 
		{
			printf("Failed to create thread %d", thread_counter);
		}
		else 
		{
			thread_available[thread_counter] == 1;
			printf("Thread %d is running \n", thread_counter );	
		}
			
	}
	free(t);
	pthread_attr_destroy(&attr)	;
	
	pthread_exit(NULL);
	return 0;
}

void *register_periodically(void *arg)
{

	char portmapper_ip[100], portmapper_port[100], buf[MAX_BUF_LEN];
	int sockfd, numbytes, packetsize, reply_code=100;
	int program_version;
	struct register_info *t;
	
	t = (struct register_info *) arg;
	// Prevent SIGPIPE
	signal (SIGPIPE, SIG_IGN);
	// read port mapper ip and port through file
	readPortmaperIP(portmapper_ip, portmapper_port);
	printf("Register Program_name %s Version %d Procedure %s\n", t->program_name, 
				t->program_version, t->procedure_name);
	while (1)
	{
		// open socket to connect to portmapper
		if ((sockfd = connectTCP_server(portmapper_ip, portmapper_port)) == 0)
		{
			perror("Connection to portmapper:");
			pthread_exit(NULL);
		}
		
		memset(buf, 0, sizeof(buf));
		
		// pack the register data
		packetsize = pack(buf, "hshsss",(int16_t)1, t->program_name,(int16_t)t->program_version,
							t->procedure_name, t->server_ip, t->server_port); // Lookup: 0, Register: 1, Unregister: 2
		
		// send
		if ((numbytes = send(sockfd, buf, packetsize,MSG_NOSIGNAL)) == -1)
		{
			perror("send to portmapper");
			pthread_exit(NULL);
		}
		
		// receive
		if (recv(sockfd, buf, MAX_MES_LEN, 0) == -1)
		{
			perror("receive from portmapper");
			pthread_exit(NULL);
		}
		
		// unpack reply message
		unpack(buf,"h", &reply_code);
			
		if (reply_code == 0)
		{
			printf("register problem\n");
			pthread_exit(NULL);
		}
		else if (reply_code == 1)
		{
			//printf("register successful\n");
		}
		
		close(sockfd);
		sleep(SERVER_REGISTER_WAIT_TIME);
	} // end while
	
	pthread_exit(NULL);

}

/* register a service to the port mapper */
int sendRegister(char *program_name, int program_version, 
						char *procedure_name, char *server_ip, char *server_port)
{
	struct register_info *t;
	pthread_t checkTimeout_thread;
	
	
	// remember it in Procedure table
	if (strcmp(procedure_name,"multiply") == 0)
	{	
		service[0] = 1;
	}
	else if (strcmp(procedure_name,"sort") == 0)
	{	
		service[1] = 1;
	}
	else if (strcmp(procedure_name,"min") == 0)
	{	
		service[2] = 1;
	}
	else if (strcmp(procedure_name,"max") == 0)
	{	
		service[3] = 1;
	}
	
	t = (struct register_info *)malloc(sizeof(struct register_info));
	strcpy(t->program_name, program_name);
	strcpy(t->procedure_name, procedure_name);
	strcpy(t->server_ip, server_ip);
	strcpy(t->server_port, server_port);
	t->program_version= program_version;
	if (pthread_create(&checkTimeout_thread, NULL, register_periodically, (void *)t) != 0) 
	{
		printf("Failed to create thread to check timeout\n");
		return 0;
	}
	sleep(1);
	return 1;
}
