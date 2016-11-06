all: server client
client:client.c
	gcc  -o  client  client.c
server:server.c 
	gcc  -o  server  proxy.c
clean:
	rm -rf server
	rm -rf client
