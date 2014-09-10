all: ss cc
	@echo "Build success"
ss: base64.o common_header.c server_header.c ss.c
	gcc base64.o ss.c -o ss.exe
cc: base64.o common_header.c client_header.c cc.c
	gcc base64.o cc.c -o cc.exe
base64.o: ./base64/base64.c ./base64/base64.h
	gcc -c ./base64/base64.c -o base64.o
clean:
	rm -rf *.o *.exe

hawawa:
	@nohup ./ss.exe 127.0.0.1 22 8000 1> /dev/null 2>/dev/null &
	@nohup ./ss.exe 127.0.0.1 22 8001 1> /dev/null 2>/dev/null &
miao:
	@nohup ./ss.exe 140.123.101.220 22 8004 1> /dev/null 2>/dev/null &
	@nohup ./ss.exe 140.123.101.220 22 8014 1> /dev/null 2>/dev/null &
gavin:
	@nohup ./ss.exe 140.112.28.143 22 8005 1> /dev/null 2>/dev/null &
alex:
	@nohup ./ss.exe 140.114.26.55 22 8006 1> /dev/null 2>/dev/null &
debra:
	@nohup ./ss.exe 140.112.28.143 22 8007 1> /dev/null 2>/dev/null &
kurt:
	@nohup ./ss.exe 127.0.0.1 22 8008 1> /dev/null 2>/dev/null &
	@nohup ./ss.exe 127.0.0.1 22 8889 1> /dev/null 2>/dev/null &
wayne:
	@nohup ./ss.exe 127.0.0.1 22 8009 1> /dev/null 2>/dev/null &
dimitri:
	@nohup ./ss.exe 140.113.235.116 22 8013 1> /dev/null 2>/dev/null &
tony:
	@nohup ./ss.exe 127.0.0.1 22 8015 1> /dev/null 2>/dev/null &
john:
	@nohup ./ss.exe 140.112.28.143 22 8017 1> /dev/null 2>/dev/null &
jin:
	@nohup ./ss.exe 140.112.28.143 22 8018 1> /dev/null 2>/dev/null &
josh:
	@nohup ./ss.exe 140.112.28.143 22 8019 1> /dev/null 2>/dev/null &
owen:
	@nohup ./ss.exe 140.112.28.143 22 8020 1> /dev/null 2>/dev/null &
eric:
	@nohup ./ss.exe 140.112.28.143 22 8021 1> /dev/null 2>/dev/null &
startup: hawawa gavin alex debra kurt wayne dimitri
	@echo ""
	
ps:
	ps -xo "%u %p %c %a"

