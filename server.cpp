#include <iostream>
#include <algorithm>
#include <set>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <poll.h>
#include <string.h>
#define POLL_SIZE 2048


int set_nonblock(int fd){
	int flags;
#if defined(O_NONBLOCK)
	if(-1 == (flags = fcntl(fd,F_GETFL, 0)))
		flags = 0;
	return fcntl(fd,F_SETFL,flags | O_NONBLOCK);
#else
	flags = 1;
	return ioctl(fd,FIOBIO, &flags);
#endif
}



int main(int argc, char **argv){
	
	int MasterSocket = socket(AF_INET,SOCK_STREAM, IPPROTO_TCP);
	std::set<int> SlaveSockets;
	printf("[+] Setup MasterSocket...\n");

	struct sockaddr_in SockAddr;
	SockAddr.sin_family = PF_INET;
	SockAddr.sin_port = htons(12346);
	SockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	printf("[+] Setup SockAddr...\n");

	bind(MasterSocket, (struct sockaddr *)(&SockAddr), sizeof(SockAddr));
	printf("[+] Binding MasterSocket...\n");
	
	set_nonblock(MasterSocket);
	printf("[+] Setup MasterSocket on NonBlock...\n");

	listen(MasterSocket,SOMAXCONN);
	printf("[+] Listening...\n");

	struct pollfd Set[POLL_SIZE];
	Set[0].fd = MasterSocket;
	Set[0].events = POLLIN;
	printf("[+] Create Set[0] as MasterSocket...\n");

	while(true){


		unsigned int Index = 1;
		for(auto Iter = SlaveSockets.begin();
			Iter != SlaveSockets.end();
			Iter++){
				Set[Index].fd = *Iter;
				Set[Index].events = POLLIN;
				Index++;			
		}
		

		unsigned int SetSize = 1 + SlaveSockets.size();
		
		poll(Set, SetSize, -1);
		
		for(unsigned int i = 0; i < SetSize; i++){
			if(Set[i].revents & POLLIN) {
				if(i){
					
				    char Buffer[1024] = "";
				  

					int RecvSize = recv(Set[i].fd,
							Buffer,
							1024,
							MSG_NOSIGNAL);
					if((RecvSize == 0) &&
						 (errno != EAGAIN)){
						shutdown(Set[i].fd,
							SHUT_RDWR);
						close(Set[i].fd);
					SlaveSockets.erase(Set[i].fd);	
					printf("SYSTEM: %d client disconnected.\n",i);
						} else if(RecvSize > 0){
							char str[20];
							strcpy(str,"mistake\r\n");
							printf("%d client: %s \n",i,Buffer);

							if(memcmp(Buffer,str,sizeof(str)) == 0)
							printf("%d client: %s \n",i,Buffer);
							

							
							
					 	}
				} else {
					int SlaveSocket = accept(MasterSocket,0,0);
					set_nonblock(SlaveSocket);
					SlaveSockets.insert(SlaveSocket);
					printf("[+] Create a new connect #%d...\n", Index);
					}
			
                 }	
	}
}	
return 0;
}
