server1: server1.o stabla.o  
	gcc -o server1 server1.o stabla.o

server1.o: server1.c stabla.h
	gcc -c -o server1.o server1.c	
	
Server2: Server2.o stabla.o  
	gcc -pthread -o Server2 Server2.o stabla.o
	
Server2.o: Server2.c stabla.h
	gcc -c -o Server2.o Server2.c	

Akib: Akib.o stabla.o  
	gcc -pthread -o Akib Akib.o stabla.o
	
Akib.o: Akib.c stabla.h
	gcc -c -o Akib.o Akib.c	

baza: baza.o stabla.o
	gcc -o baza $(mysql_config --cflags) baza.o stabla.o $(mysql_config --libs)

baza.o: baza.c stabla.h
	gcc -c -o baza.o $(mysql_config --cflags) baza.c $(mysql_config --libs)
	
korisnikA: korisnikA.o stabla.o
	gcc -pthread -o korisnikA korisnikA.o stabla.o

korisnikA.o: korisnikA.c stabla.h
	gcc  -c -o korisnikA.o korisnikA.c
	
client: client.o stabla.o
	gcc -pthread -o client client.o  -lsctp stabla.o `mysql_config --libs`
	
client.o: client.c stabla.h
	gcc -c -o client.o `mysql_config --cflags` -lsctp client.c 

clientFink: clientFink.o stabla.o
	gcc -pthread -o clientFink clientFink.o  -lsctp stabla.o `mysql_config --libs`
	
clientFink.o: clientFink.c stabla.h
	gcc -c -o clientFink.o `mysql_config --cflags` -lsctp clientFink.c 

stabla.o: stabla.c stabla.h
	gcc  -c -o stabla.o stabla.c