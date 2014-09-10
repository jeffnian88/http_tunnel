all: base64.o client_header.o server_header.o
	@echo "Build lib success"

base64.o:
	gcc -c ./base64.c -I../include -o base64.o

common_header.o: common_header.c ../include/common_header.h
	gcc -c ./common_header.c -I../include -o ./common_header.o

client_header.o: common_header.o client_header.c ../include/client_header.h
	gcc -c ./client_header.c -I../include -L./common_header.o -o client_header.o

server_header.o: common_header.o server_header.c ../include/server_header.h
	gcc -c ./server_header.c -I../include -L./common_header.o -o server_header.o
clean:
	rm -rf *.o
