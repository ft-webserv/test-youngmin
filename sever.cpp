#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

void error_handling(char *message)
{
	std::cerr << message << std::endl;
	exit(1);
}

int main(int argc, char *argv[])
{
	int serv_sock;
	int clint_sock;

	struct sockaddr_in serv_addr;
	struct sockaddr_in clint_addr;
	socklen_t clint_addr_size;

	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	if (serv_sock == -1)
		error_handling("socket error!");

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(std::atoi(argv[1]));

	if (bind(serv_sock, (struct sockaddr *)(&serv_addr), sizeof(serv_addr)) == -1)
		error_handling("bind error!");

	if (listen(serv_sock, 5) == -1)
		error_handling("listen error!");

	clint_addr_size = sizeof(clint_addr);
	clint_sock = accept(serv_sock, (struct sockaddr *)(&clint_addr), &clint_addr_size);
	if (clint_sock == -1)
		error_handling("accept error!");

	char msg[] = "Hello this is server!\n";
	write(clint_sock, msg, sizeof(msg));

	close(serv_sock);
	close(clint_sock);
}
