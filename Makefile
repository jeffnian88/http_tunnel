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

client:
	 ./cc.exe 192.168.50.3 3128 140.112.30.32 8001 8001 username password
tests:
	./ss.exe 127.0.0.1 22 50000
testc:
	./cc.exe 192.168.50.2 3128 140.112.30.32 50000 50000
ps:
	ps -xo "%u %p %c %a"
server:
	@nohup ./ss.exe 127.0.0.1 22 8000 1> /dev/null 2>/dev/null &
	@nohup ./ss.exe 127.0.0.1 22 8001 1> /dev/null 2>/dev/null &
	@nohup ./ss.exe 127.0.0.1 22 8002 1> /dev/null 2>/dev/null &
	@nohup ./ss.exe 140.113.235.116 22 8003 1> /dev/null 2>/dev/null &
		 
alex:
	@nohup ./ss.exe 140.114.26.55 22 8005 1> /dev/null 2>/dev/null &
	@nohup ./ss.exe 140.114.26.55 22 8006 1> /dev/null 2>/dev/null &
cat:
	@nohup ./ss.exe 140.123.101.220 22 8004 1> /dev/null 2>/dev/null &

debra:
	@nohup ./ss.exe 140.112.28.143 22 8007 1> /dev/null 2>/dev/null &
	@nohup ./ss.exe 140.112.28.143 22 8888 1> /dev/null 2>/dev/null &
	@nohup ./ss.exe 140.112.28.143 22 8889 1> /dev/null 2>/dev/null &
kurt:
	@nohup ./ss.exe 127.0.0.1 22 8008 1> /dev/null 2>/dev/null &
	@nohup ./ss.exe 127.0.0.1 22 8889 1> /dev/null 2>/dev/null &
wayne:
	@nohup ./ss.exe 127.0.0.1 22 8009 1> /dev/null 2>/dev/null &
	@nohup ./ss.exe 127.0.0.1 22 8010 1> /dev/null 2>/dev/null &
	@nohup ./ss.exe 127.0.0.1 22 8011 1> /dev/null 2>/dev/null &
	@nohup ./ss.exe 127.0.0.1 22 8012 1> /dev/null 2>/dev/null &
dimitri:
	@nohup ./ss.exe 140.113.235.116 22 8012 1> /dev/null 2>/dev/null &
	@nohup ./ss.exe 140.113.235.116 22 8013 1> /dev/null 2>/dev/null &
james:
	@nohup ./ss.exe 127.0.0.1 22 8088 1> /dev/null 2>/dev/null &

startup: server alex debra kurt wayne dimitri james
	echo "Done"
