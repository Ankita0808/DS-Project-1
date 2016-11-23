all: clean client server portmapper

portmapper: portmapper.o network_function.o
	gcc portmapper.o network_function.o -o portmapper -lpthread

client: client.o client_lib.o
	gcc client.o client_lib.o -o client -lm
	
server: server.o server_lib.o
	gcc server.o server_lib.o -o server -lpthread 

portmapper.o: portmapper.c
	gcc -c portmapper.c
		
server.o: server.c
	gcc -c server.c

server_lib.o: server_stub.o network_function.o
	ld -r server_stub.o network_function.o -o server_lib.o
	
server_stub.o: server_stub.c
	gcc -c server_stub.c

client_lib.o: client_stub.o network_function.o
	ld -r client_stub.o network_function.o -o client_lib.o
	 
client_stub.o: client_stub.c
	gcc -c client_stub.c

network_function.o: network_function.c
	gcc -c network_function.c

clean:
	rm -rf *o client server portmapper