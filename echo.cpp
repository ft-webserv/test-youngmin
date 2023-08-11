#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#include <iostream>
#include <vector>
#include <map>
#include <string>

void error_handle(const char *msg)
{
	std::cout << msg << std::endl;
}

void disconnect_client(int client_fd, std::map<int, std::string> &clients)
{
	std::cout << "client disconnceted : " << client_fd << std::endl;
	close(client_fd);
	clients.erase(client_fd);
}

void change_events(std::vector<struct kevent> &change_list, uintptr_t idnet, int16_t filter,
	uint16_t flags, uint32_t fflags, intptr_t data, void *udata)
{
	struct kevent	temp_event;

	EV_SET(&temp_event, idnet, filter, flags, fflags, data, udata);
	change_list.push_back(temp_event);
}

int main()
{
	int					server_socket;
	struct sockaddr_in	server_addr;

	if ((server_socket = socket(PF_INET, SOCK_STREAM, 0)) == -1)
		error_handle("socket error");

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(8080);
	int reuse = 1;
	if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) == -1) {
		perror("setsockopt");
		// 에러 처리
	}

	if (bind(server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1)
		error_handle("bind error");

	if (listen(server_socket, 5) == -1)
		error_handle("listen error");
	fcntl(server_socket, F_SETFL, O_NONBLOCK);

	int kq;
	if ((kq = kqueue()) == -1)
		error_handle("kqueue error");

	std::map<int, std::string> clients;
	std::vector<struct kevent> change_list;
	struct kevent	event_list[8];
	change_events(change_list, server_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0 , 0, NULL);
	std::cout << "echo server start!" << std::endl;

	int new_event;
	struct kevent	*curr_event;
	while (1){
		new_event = kevent(kq, &change_list[0], change_list.size(), event_list, 8, NULL);
		if (new_event == -1)
			error_handle("kevent error");

		change_list.clear();

		for(int i = 0; i < new_event; i++){
			curr_event = &event_list[i];
			if(curr_event->flags & EV_ERROR){
				if(curr_event->ident == server_socket)
					error_handle("server socket error");
				else{
					error_handle("client socket error");
					disconnect_client(curr_event->ident, clients);
				}
			}
			else if(curr_event->filter == EVFILT_READ){
				if (curr_event->ident == server_socket){
					int client_socket;
					if((client_socket = accept(server_socket, NULL, NULL)) == -1)
						error_handle("accept error");
					std::cout << "clinent connect : " << client_socket << std::endl;
					fcntl(client_socket, F_SETFL, O_NONBLOCK);
					change_events(change_list, client_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
					change_events(change_list, client_socket, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
					clients[client_socket] = "";
				}
				else if (clients.find(curr_event->ident) != clients.end()){
					char	buf[1024];
					int		n = read(curr_event->ident, buf, sizeof(buf));
					std::cout << buf << std::endl;

					if (n <= 0){
						if (n < 0){
							error_handle("read error!");
						}
						disconnect_client(curr_event->ident, clients);
					}
					else{
						buf[n] = '\0';
						clients[curr_event->ident] += buf;
						std::cout << "received data from " << curr_event->ident << ": " << clients[curr_event->ident];
					}
				}
			}
			if (curr_event->filter == EVFILT_WRITE){
				if (clients.find(curr_event->ident) != clients.end()){
					if (clients[curr_event->ident] != ""){
						int n;
						if ((n = write(curr_event->ident, clients[curr_event->ident].c_str(), clients[curr_event->ident].size())) == -1){
							error_handle("write error");
							disconnect_client(curr_event->ident, clients);
						}
						else
							clients[curr_event->ident].clear();
					}
				}
			}
		}
	}
	return (0);
}
