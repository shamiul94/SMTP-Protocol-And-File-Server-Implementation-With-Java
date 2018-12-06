#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char *argv[]){

	int sockfd;
	int bind_flag;
	char buffer[1024];
	struct sockaddr_in server_address;
	struct sockaddr_in client_address;

	if(argc != 2){
		printf("%s <ip address>\n", argv[0]);
		exit(1);
	}

	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(4747);
	server_address.sin_addr.s_addr = inet_addr("192.168.10.100");

	client_address.sin_family = AF_INET;
	client_address.sin_port = htons(4747);
	client_address.sin_addr.s_addr = inet_addr(argv[1]);

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	bind_flag = bind(sockfd, (struct sockaddr*) &client_address, sizeof(sockaddr_in));

	while(true){
		gets(buffer);
		if(!strcmp(buffer, "shutdown")) break;
		sendto(sockfd, buffer, 1024, 0, (struct sockaddr*) &server_address, sizeof(sockaddr_in));
	}

	close(sockfd);

	return 0;

}
