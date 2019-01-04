all:   build_server build_client

build_server:
	g++ -g3 -std=gnu++0x  -o server -g server.cpp

build_client:
	gcc -O0 -o client client.c








	
