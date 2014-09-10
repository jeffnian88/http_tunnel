all: ss cc
	@echo "Build success"

mylib:
	make -C ./src all

ss: mylib
	gcc -I./include ./src/base64.o ./src/common_header.o ./src/server_header.o ss.c -o ss.exe

cc: mylib
	gcc -I./include ./src/base64.o ./src/client_header.o ./src/common_header.o cc.c -o cc.exe

clean:
	make -C ./src clean
	rm -rf *.o *.exe *.log

server:
	 nohup ./ss.exe 127.0.0.1 22 8001 12> /dev/null &
client:
	 ./cc.exe 192.168.50.3 3128 140.112.30.32 8001 8001 username password
