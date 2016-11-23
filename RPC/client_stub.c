#include "network_function.h"

/* read ip address and port number from the global file of the port mapper */
int readPortmaperIP (char *server_ip, char *server_port)
{
	// read IP address and port from filename defined in header.h
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

/* request the port mapper for the ip address and port number of the server */
int getServerAddress(char *program_name, int program_version, 
						char *procedure_name, char *server_ip, char *server_port)
{
	char portmapper_ip[100], buf[MAX_BUF_LEN], portmapper_port[100];
	int sockfd, numbytes, packetsize, reply_code;
		
	// read port mapper ip and port through file
	if (!readPortmaperIP(portmapper_ip, portmapper_port))
		return 0;
	
	// open socket to connect to portmapper
	if ((sockfd = connectTCP_server(portmapper_ip, portmapper_port)) == 0)
		return 0;
	
	memset(buf, 0, sizeof(buf));
			
	// pack the request
	packetsize = pack(buf, "hshs",(int16_t)0, program_name,(int16_t)program_version,
						procedure_name); // Lookup: 0, Register: 1, Unregister: 2
	
	// send packet to the port mapper
	if ((numbytes = send(sockfd, buf, packetsize,0)) == -1)
	{
		perror("send to portmapper");
		return 0;
	}
	
	// receive packet from port mapper
	if (recv(sockfd, buf, MAX_MES_LEN, 0) == -1)
	{
		perror("receive from portmapper");
		return 0;
	}
	
	// unpack received data
	unpack(buf,"h", &reply_code);
	if (reply_code == 0)
	{
		printf("No server supports this procedure\n");
		return 0;
	}
	else if (reply_code == 1)
	{
		unpack(buf,"hss", &reply_code, server_ip, server_port);
		printf("SERVER TCP IP and Port is: %s %s\n", server_ip, server_port);
	}
	
	close(sockfd);
	return 1;
}

int doRPC_client(char *procedure_name, char *server_ip, 
		char* server_port, int trans_id, float *a, float *b, float *c, 
			int m1, int n1, int m2, int n2, int m3, int n3)
{
	// create socket
	// connect to server
	// ask for procedure name, transaction id
	// yes -> get udp socket -> return 0;
	// no -> return 1; //Server does not support
	
	
	//TODO: because we dont have result from Portmapper -> server write to file; 
	// need to delete in the future
	char buf[MAX_BUF_LEN], serverUDP_port[100];
	int sockfd, sockUDP, numbytes, packetsize,i,j, nready, packet_lost_array[100];
	int16_t reply, packet_lost;
	struct sockaddr_storage their_addr;
	socklen_t addr_len;
	
	// open socket to connect to server
	if ((sockfd = connectTCP_server(server_ip, server_port)) == 0)
		return 0;
	
	// send transaction id and procedure name
	packetsize = pack(buf, "hshhhhhh",(int16_t) trans_id, procedure_name,
					(int16_t)m1, (int16_t)n1, (int16_t)m2, (int16_t)n2, (int16_t)m3, (int16_t)n3); 
	
	if ((numbytes = send(sockfd, buf, packetsize,0)) == -1)
	{
		perror("send to portmapper");
		return 0;
	}
	
	if (recv(sockfd,buf,MAX_MES_LEN,0) == -1)
	{
		perror ("receive error");
		return 0;
	}
				
	unpack(buf,"h", &reply);
	// reply = 1 -> new trans, procedure ok, run as normal
	// reply = 0 -> no procedure -> abort
	// reply = 2 -> old trans with result, need to receive only
	
	// TODO: switch case here, now consider only reply = 1
	unpack(buf, "hs", &reply, serverUDP_port);
	
	if (reply ==0)
	{
		printf("Server does not support this procedure\n");
		return 0;
	}
	else if (reply ==1)
	{
		printf ("Do normal RPC; UDP port: %s OK TO CONTINUE \n", serverUDP_port);
		printf ("Server UDP IP and PORT:  %s %s \n", server_ip, serverUDP_port);
		// send first matrix
		sockUDP = connectUDP_server(server_ip, serverUDP_port, &their_addr, &addr_len);
	
		if (!sendDataUDP(sockfd, sockUDP, &their_addr, addr_len , a, m1, n1))
			return 0;
		if (!sendDataUDP(sockfd, sockUDP, &their_addr, addr_len , b, m2, n2))
			return 0;
		if (!receiveDataUDP(sockfd, sockUDP, &their_addr, &addr_len, c, m3, n3, 0, NULL, 0))
			return 0;
	}
	else if (reply == 2)
	{
		printf ("Same transaction ID, receive results, server UDP port: %s \n", serverUDP_port);
		sockUDP = connectUDP_server(server_ip, serverUDP_port, &their_addr, &addr_len);
		memset(&buf, 0 ,sizeof(buf));
		pack(buf,"h",(int16_t)1);
		if (sendto(sockUDP, buf, MAX_MES_LEN-1, 0, (struct sockaddr *) &their_addr, addr_len) == -1)
			return 0;
		if (!receiveDataUDP(sockfd, sockUDP, &their_addr, &addr_len, c, m3, n3, 0, NULL, 0))
			return 0;	
	}
	else
		return 0;
	
	close(sockfd);
	return 1;
}

/* multiply two matrices */
int multiply(char *program_name, int program_version,
					float*a, float *b, float *c, int m , int n , int l)
{
	int trans_id, counter = 0;
	char server_ip[100], serverTCP_port[100];
	char procedure_name[10] = "multiply";
	time_t t;
	//1. Get server ip and port from port mapper
	//2. Connect to server and send data
	//3. receive data from server
	
	srand((unsigned) time(&t));
	rand();
	trans_id = rand() % 10000;
	
	//trans_id = 101;
	printf("transid: %d\n", trans_id);
	while (counter < TRY_TIME)
	{
		// 1. request the port mapper for ip address and port number of server
		if (getServerAddress(program_name, program_version, procedure_name, server_ip, serverTCP_port) == 0)
		{
			counter++;
			continue;
		}
	
		//2. send: matrix a[m][n], matrix b[n][l], transid (random), procedure name
		if (doRPC_client(procedure_name, server_ip, serverTCP_port, trans_id, a, b, c, m, n, n, l, m, l) == 1)
			return 1;
			
			
		counter++;
	}
	
	return 0;
}

/* sort vector a[n], result is stored in vector b[n] */
int sort(char *program_name, int program_version, float *a, float *b, int n)
{
	int trans_id, counter = 0;
	char server_ip[100], serverTCP_port[100];
	char procedure_name[10] = "sort";
	time_t t;
	
	srand((unsigned) time(&t));
	rand();
	trans_id = rand() % 10000;
	//trans_id = 102;
	printf("transid: %d\n", trans_id);
	while (counter < TRY_TIME)
	{
		// 1. request the port mapper for ip address and port number of server
		if (getServerAddress(program_name, program_version, procedure_name, server_ip, serverTCP_port) == 0)
		{
			counter++;
			continue;
		}
	
		// 2. send: matrix a[m][n], matrix b[n][l], transid (random), procedure name
		if (doRPC_client(procedure_name, server_ip, serverTCP_port, trans_id, a, NULL , b, 1, n, 0, 0, 1, n) == 1)
			return 1;
			
		counter++;
	}
	return 0;
}

/* find min in vector a[n], result is stored in b[n] */
int min(char *program_name, int program_version, float *a, float *b, int n)
{
	int trans_id, counter = 0;
	char server_ip[100], serverTCP_port[100];
	char procedure_name[10] = "min";
	time_t t;
	
	srand((unsigned) time(&t));
	rand();
	trans_id = rand() % 10000;
	//trans_id = 103;
	printf("transid: %d\n", trans_id);
	while (counter < TRY_TIME)
	{
		// 1. request the port mapper for ip address and port number of server
		if (getServerAddress(program_name, program_version, procedure_name, server_ip, serverTCP_port) == 0)
		{
			counter++;
			continue;
		}
	
		// 2. send: matrix a[m][n], matrix b[n][l], transid (random), procedure name
		if (doRPC_client(procedure_name, server_ip, serverTCP_port, trans_id, a, NULL , b, 1, n, 0, 0, 1, 1) == 1)
			return 1;
			
		counter++;
	}
	return 0;
	
}

/* find max in vector a[n], result is stored in b[n] */
int max(char *program_name, int program_version, float *a, float *b, int n)
{
	int trans_id, counter = 0;
	char server_ip[100], serverTCP_port[100];
	char procedure_name[10] = "max";
	time_t t;
	
	srand((unsigned) time(&t));
	rand();
	trans_id = rand() % 10000;
	//trans_id = 104;
	printf("transid: %d\n", trans_id);
	while (counter < TRY_TIME)
	{
		// 1. request the port mapper for ip address and port number of server
		if (getServerAddress(program_name, program_version, procedure_name, server_ip, serverTCP_port) == 0)
		{
			counter++;
			continue;
		}
	
		// 2. send: matrix a[m][n], matrix b[n][l], transid (random), procedure name
		if (doRPC_client(procedure_name, server_ip, serverTCP_port, trans_id, a, NULL , b, 1, n, 0, 0, 1, 1) == 1)
			return 1;
			
		counter++;
	}
	return 0;
}
