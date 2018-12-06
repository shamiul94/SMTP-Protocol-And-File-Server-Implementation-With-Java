#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char *argv[]){

	int sockfd;
	int bind_flag;
	int sent_bytes;
	char buffer[1024];
	struct sockaddr_in server_address;
	struct sockaddr_in client_address;

	if(argc != 2){
		printf("%s <ip address>\n", argv[0]);
		exit(1);
	}


	

	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(4747);
	//server_address.sin_addr.s_addr = inet_addr("192.168.10.100");
	inet_pton(AF_INET,"192.168.10.100",&server_address.sin_addr);	

	client_address.sin_family = AF_INET;
	client_address.sin_port = htons(4747);
	//client_address.sin_addr.s_addr = inet_addr(argv[1]);
	inet_pton(AF_INET,argv[1],&client_address.sin_addr);

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	bind_flag = bind(sockfd, (struct sockaddr*) &client_address, sizeof(sockaddr_in));
	printf("%d\n", bind_flag);
	if(bind_flag==0)printf("successful bind");

	while(true){
		gets(buffer);
		
		sent_bytes=sendto(sockfd, buffer, 1024, 0, (struct sockaddr*) &server_address, sizeof(sockaddr_in));
		
		if(!strcmp(buffer, "shutdown")) break;
	}

	close(sockfd);

	return 0;

}
