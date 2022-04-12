all: statclient statserver statclient_th statserver_th

statclient: statclient.c
	gcc -Wall -o statclient statclient.c -lrt


statserver: statserver.c
	gcc -Wall -o statserver statserver.c -lrt

statclient_th: statclient_th.c
	gcc -Wall -g -o statclient_th statclient_th.c -lpthread -lrt

statserver_th: statserver_th.c
	gcc -Wall -g -o statserver_th statserver_th.c -lpthread -lrt

clean: 
	rm -fr *~ statserver statclient statclient_th statserver_th