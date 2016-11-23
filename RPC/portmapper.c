#include "network_function.h"

struct server_address
{
	char server_ip[100];
	char server_port[100];
	int time;
};

struct thread_info 
{
	int connfd;
	int thread_id;
};

struct serverlist_table
{
	char program_name[100];
	int program_version;
	char procedure_name[100];
	struct server_address server_address[SERVERLIST_TABLE_SIZE];
	int round_robin_index; // round robin policy counter
	int current_server;    // total number of servers available for 1 procedure
};

//threads
pthread_t threads[THREAD_NUMBER];
int thread_available[THREAD_NUMBER];
pthread_mutex_t running_mutex;

//Table list array 
struct serverlist_table serverlist_table[SERVERLIST_TABLE_SIZE];
int current_serverlist = 0;

int write_portmapper_ip(char *addrstr, char *port)
{
	FILE *ptr_file;

	ptr_file =fopen(FILENAME, "w");
	if (!ptr_file)
	{
		perror("server: open file to write");
		return 0;
	}
	fprintf(ptr_file,"%s,%s", addrstr, port);
	fclose(ptr_file);
}

void *thread_work(void *arg)
{
	int16_t register_type, program_version;
	char buf[MAX_BUF_LEN];
	char program_name[20], procedure_name[20];
	int i, j, resetTimeout, newServer, packet_size;
	char server_ip[100], server_port[100];		
	struct thread_info *info;
	int task = 0, same_transid =0, connfd, thread_id, numbytes; 
	
	info = (struct thread_info*) arg;
	
	connfd = info->connfd;
	thread_id = info->thread_id;
	
	
	if (recv(connfd,buf,MAX_BUF_LEN,0) == -1)
	{
		perror ("receive error");
	}
			
	unpack(buf,"hshs", &register_type, program_name, &program_version, procedure_name);
			
	//read the message and reply
						
	switch (register_type)
	{
		case 0: //client lookup
				
			printf("Portmapper Receive from client: program: %s, version: %d, procedure: %s\n", 
					program_name, program_version, procedure_name);
					
			// check if there is any server support this procedure
			for (i=0; i<current_serverlist; i++)
			{
				if (strcmp(program_name, serverlist_table[i].program_name) == 0 &&
						strcmp(procedure_name, serverlist_table[i].procedure_name) == 0 &&
							program_version == serverlist_table[i].program_version
								&& serverlist_table[i].current_server > 0)  // if server is found
				{
					// send found to client
							
					printf("%d %d %s\n",i, serverlist_table[i].round_robin_index, serverlist_table[0].server_address[0].server_ip);
					printf("Server Found: %s %s\n", serverlist_table[i].server_address[serverlist_table[i].round_robin_index].server_ip,
					serverlist_table[i].server_address[serverlist_table[i].round_robin_index].server_port);
								
					memset(buf,0,sizeof(buf));
					packet_size = pack(buf,"hss",(int16_t)1, 
											serverlist_table[i].server_address[serverlist_table[i].round_robin_index].server_ip,
											serverlist_table[i].server_address[serverlist_table[i].round_robin_index].server_port);
					serverlist_table[i].round_robin_index = (serverlist_table[i].round_robin_index+1)
											% serverlist_table[i].current_server;
					send(connfd,buf, packet_size, 0);
					break;
				}
			}
					
			//send no found to client
			memset(buf,0,sizeof(buf));
			packet_size = pack(buf,"h",(int16_t)0);
			send(connfd,buf, packet_size, 0);
			break;
						
					
		case 1: //server register
			unpack(buf,"hshsss", &register_type, program_name, &program_version, procedure_name, server_ip, server_port);
					
			printf("Portmapper Receive from server:program: %s, version: %d, procedure: %s ip: %s port: %s\n", 
								 program_name, program_version, procedure_name, server_ip, server_port);
			
			newServer = 0;
			
			for (i=0; i<SERVERLIST_TABLE_SIZE; i++)
			{
				
				if (strcmp(program_name, serverlist_table[i].program_name) == 0 &&
						strcmp(procedure_name, serverlist_table[i].procedure_name) == 0 &&
							program_version == serverlist_table[i].program_version)  // Procedure already register
				{
					// check if server resend the register packet; yes -> reset timeout
					resetTimeout = 0;
					newServer = 1;
					for (j=0; j< serverlist_table[i].current_server; j++)
					{
						if (strcmp(serverlist_table[i].server_address[j].server_ip, server_ip) == 0 &&
							strcmp(serverlist_table[i].server_address[j].server_port, server_port) == 0)
						{
							printf("Server %s %s resent register packet \n", server_ip, server_port);
							resetTimeout = 1;
							pthread_mutex_lock (&running_mutex);
							serverlist_table[i].server_address[j].time =2;
							pthread_mutex_unlock (&running_mutex);
							break;
						}
					}
					// new server provide same service, add to the link list
					if (resetTimeout == 0)
					{
						pthread_mutex_lock (&running_mutex);
						serverlist_table[i].current_server++;
						strcpy(serverlist_table[i].server_address[serverlist_table[i].current_server-1].server_ip, server_ip);
						strcpy(serverlist_table[i].server_address[serverlist_table[i].current_server-1].server_port, server_port);
						serverlist_table[i].server_address[serverlist_table[i].current_server-1].time = 2;
						pthread_mutex_unlock (&running_mutex);
					}
				}
			}	
			//totally new
			if (newServer == 0)
			{
				pthread_mutex_lock (&running_mutex);
				current_serverlist++;
				strcpy(serverlist_table[current_serverlist-1].program_name,program_name);
				strcpy(serverlist_table[current_serverlist-1].procedure_name,procedure_name);
				serverlist_table[current_serverlist-1].program_version = program_version;
				serverlist_table[current_serverlist-1].round_robin_index = 0;
				serverlist_table[current_serverlist-1].current_server = 1;
				strcpy(serverlist_table[current_serverlist-1].server_address[0].server_ip, server_ip);
				strcpy(serverlist_table[current_serverlist-1].server_address[0].server_port, server_port);
												
				serverlist_table[current_serverlist-1].server_address[0].time = 2;
				pthread_mutex_unlock (&running_mutex);
			}
					
						
			//memset(buf,0,sizeof(buf));
			memset(buf,0,sizeof(buf));
			packet_size = pack(buf, "h", (int16_t) 1); 
			if ((numbytes = send(connfd, buf, MAX_MES_LEN,0)) == -1)
			{
				perror("send reply to server");
			}
			break;
					
		default:
		// this is wrong message
		// reply wrong message
		break;
	}
	close(connfd);
	pthread_mutex_lock (&running_mutex);
	thread_available[thread_id] = 0;
	pthread_mutex_unlock (&running_mutex);
	pthread_exit(NULL);
}

// Thread to check TIMEOUT server
void *checkTimeout(void *arg)
{
	int i,j,k;
	char ip[100], port[100];
	
	while (1)
	{
		for (i=0; i<current_serverlist; i++)
		{
			for (j=0; j< serverlist_table[i].current_server; j++)
			{
				serverlist_table[i].server_address[j].time--;
				if (serverlist_table[i].server_address[j].time == 0)
				{
					for (k = j; k< serverlist_table[i].current_server - 1; k++)
					{
						pthread_mutex_lock (&running_mutex);
		
						strcpy(serverlist_table[i].server_address[k].server_port, serverlist_table[i].server_address[k+1].server_port);
						strcpy(serverlist_table[i].server_address[k].server_port, serverlist_table[i].server_address[k+1].server_port);

						pthread_mutex_unlock (&running_mutex);
					}
					serverlist_table[i].current_server--;
					printf("One server is out of time\n");
				}
			}
		}
		sleep(SERVER_REGISTER_TIME_OUT/2);
	}
	pthread_exit(NULL);
}

void run_portmapper(int sockfd)
{
	int connfd, packetsize, numbytes, thread_counter;
	struct thread_info *t;
	int i;
	pthread_t checkTimeout_thread;
	
	// Prevent SIGPIPE
	signal (SIGPIPE, SIG_IGN);
	
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	pthread_mutex_init (&running_mutex, NULL);
	
	for (i = 0; i< THREAD_NUMBER; i++)
		thread_available[THREAD_NUMBER] = 0; //initialize all thread as available
	
	if (pthread_create(&checkTimeout_thread, NULL, checkTimeout, NULL) != 0) 
	{
		printf("Failed to create thread to check timeout\n");
	}
	
	t = (struct thread_info *) malloc(sizeof(struct thread_info));	
	
	while(1) 
	{
		connfd = accept(sockfd,NULL, NULL);/*Accept new request for a connection*/
		//printf("\n\nnew client connects\n");
		if (connfd == -1)
		{
			perror("server: accept");
		}
			
		while (1)
		{
			thread_counter++;
			if (thread_counter > THREAD_NUMBER-1)
			{
				thread_counter = 0;
				sleep_burst();
			}
			if (thread_available[THREAD_NUMBER] == 0) break;
		}
		
		t->connfd = connfd;
		t->thread_id = thread_counter;
					
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
   pthread_exit(NULL);
}

int main()
{
	int sockfd;
	char portmapper_ip[100], portmapper_port[1000];
	// open TCP socket and bind
	sockfd = createTCP_server(portmapper_ip, portmapper_port);
	printf("potmapper ip and port is: %s %s \n", portmapper_ip, portmapper_port);
	
	// write IP address and port to File
	write_portmapper_ip(portmapper_ip, portmapper_port);
	
	// do Portmapper while loop
	run_portmapper(sockfd);
	
	close (sockfd);
}
