# Makefile for the C++ socket example code
#

service_objects = libsocket.c unsock.c sbslog.c

all : socketsrv

socketsrv: 
	gcc --shared -fPIC -o libsocket.so $(service_objects) -lpthread 

clean:
	rm -f *.o socketsrv
