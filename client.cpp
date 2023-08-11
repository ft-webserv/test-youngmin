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
	int clint_sock;
	struct sockaddr_in serv_addr;
	char message[1024] = "Hello world!";

	clint_sock = socket(PF_INET, SOCK_STREAM, 0);
	if (clint_sock == -1)
		error_handling("socket error");

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(std::atoi(argv[2]));

	if (connect(clint_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
		error_handling("connect error");

	if (write(clint_sock, message, sizeof(message) - 1) == -1)
		error_handling("write error");

	close(clint_sock);
	return (0);
}
