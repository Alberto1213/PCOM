#include "utils.h"

void closing(int sock_tcp)
{
	shutdown(sock_tcp, SHUT_RDWR);
	close(sock_tcp);
}

int main(int argc, char **argv)
{
	if (argc <= 1)
		exit(0);

	setvbuf(stdout, NULL, _IONBF, BUFSIZ);

	unordered_map<string, list<sub_message>> store_and_forward;
	unordered_map<string, list<subscription>> topics_subs;
	unordered_map<string, int> active_ids_sockets;
	unordered_map<int, tcpC> active_clients;
	unordered_map<string, tcpC> all_clients;

	list<sub_message> to_send_list;
	list<subscription> empty_list;

	struct sockaddr_in Rstation;

	char buffer[2048];

	int n;
	int on = 1;

	tcpC client;
	tcpC client_to_delete;

	fd_set read_fds;
	FD_ZERO(&read_fds);

	int sock_tcp;
	sock_tcp = socket(PF_INET, SOCK_STREAM, 0);

	if (!(sock_tcp != -1))
		exit(0);

	int sock_udp;
	sock_udp = socket(PF_INET, SOCK_DGRAM, 0);

	if (!(sock_udp != -1))
		exit(0);

	setsockopt(sock_tcp, IPPROTO_TCP,
			   TCP_NODELAY, (void *)&on, sizeof(on));

	int x = atoi(argv[1]);
	Rstation.sin_port = htons(x);
	Rstation.sin_addr.s_addr = INADDR_ANY;
	Rstation.sin_family = AF_INET;

	if (!(bind(sock_tcp, (struct sockaddr *)(&Rstation),
			   sizeof(Rstation)) >= 0))
		exit(0);

	if (!(bind(sock_udp, (struct sockaddr *)(&Rstation),
			   sizeof(Rstation)) >= 0))
		exit(0);

	if (!(listen(sock_tcp, 10) >= 0))
		exit(0);

	FD_SET(0, &read_fds);
	FD_SET(sock_tcp, &read_fds);
	FD_SET(sock_udp, &read_fds);

	int fd_max;
	fd_max = max(sock_tcp, sock_udp);

	while (1)
	{
		fd_set tmp_fds;
		tmp_fds = read_fds;

		int ret;
		ret = select(fd_max + 1, &tmp_fds, NULL, NULL, NULL);

		if (ret < 0)
			exit(0);

		for (int i = fd_max; i >= 0; --i)
		{
			if (FD_ISSET(i, &tmp_fds) != 0)
			{
				if (i == 0)
				{
					memset(buffer, 0, 2048);
					fgets(buffer, 2048, stdin);

					if (strncmp(buffer, "exit", 4) == 0)
					{
						for (int i = fd_max; i > 0; --i)
						{
							if (FD_ISSET(i, &read_fds))
							{
								shutdown(sock_tcp, SHUT_RDWR);
							}
						}
						closing(sock_tcp);
						exit(0);
					}
					continue;
				}

				if (sock_tcp - i == 0)
				{
					int on = 1;
					setsockopt(i, IPPROTO_TCP, TCP_NODELAY, (void *)&on, sizeof(int));

					int clilen = sizeof(sockaddr_in);

					sockaddr_in cli_addr;
					memset(&cli_addr, 0, clilen);

					int newsockfd = accept(sock_tcp,
										   (struct sockaddr *)&cli_addr, (socklen_t *)&clilen);

					if (newsockfd <= -1)
						exit(0);

					char new_client_id[11];
					memset(&new_client_id, 0, sizeof(new_client_id) / sizeof(*new_client_id));

					n = recv(newsockfd, new_client_id, 11, 0);

					if (n <= -1)
						exit(0);

					if (!(active_ids_sockets.end() ==
						  active_ids_sockets.find(new_client_id)))
					{
						cout << "Client ";
						cout << new_client_id;
						cout << " already connected.\n";

						closing(newsockfd);
						continue;
					}

					FD_SET(newsockfd, &read_fds);
					if (!(newsockfd - fd_max <= 0))
					{
						fd_max = newsockfd;
					}

					if (all_clients.find(new_client_id) !=
						all_clients.end())
					{
						all_clients[new_client_id];
						active_clients[newsockfd] = client;
						active_ids_sockets[client.id] = newsockfd;

						all_clients.erase(new_client_id);
						all_clients[new_client_id] = client;

						cout << "New client ";
						cout << new_client_id;
						cout << " connected from ";
						cout << inet_ntoa(cli_addr.sin_addr);
						cout << ":";
						cout << ntohs(cli_addr.sin_port);
						cout << ".\n";

						if (!(store_and_forward.end() ==
							  store_and_forward.find(client.id)))
						{
							list<sub_message>::iterator to_send;
							for (to_send = store_and_forward[client.id].begin();
								 to_send != store_and_forward[client.id].end(); ++to_send)
							{
								send(client.socket, &(*to_send), sizeof(sub_message), 0);
							}

							store_and_forward.erase(store_and_forward.find(client.id));
						}
						continue;
					}

					client.socket = newsockfd;
					strcpy(client.id, new_client_id);

					all_clients[new_client_id] = client;
					active_clients[newsockfd] = client;
					active_ids_sockets[new_client_id] = newsockfd;

					cout << "New client ";
					cout << new_client_id;
					cout << " connected from ";
					cout << inet_ntoa(cli_addr.sin_addr);
					cout << ":";
					cout << ntohs(cli_addr.sin_port);
					cout << ".\n";

					continue;
				}

				if (i == sock_udp)
				{
					msgU message;
					memset(&message, 0, sizeof(message));
					memset(buffer, 0, 2048);

					int szfrmst = sizeof(Rstation);

					n = recvfrom(sock_udp, buffer, 1551, 0,
								 (struct sockaddr *)(&Rstation), (socklen_t *)&szfrmst);

					if (n <= -1)
						exit(0);

					memcpy(&message.content, buffer + 51, 1500);
					memcpy(&message.dataT, buffer + 50, 1);
					memcpy(&message.topic, buffer, 50);

					if (!(topics_subs.end() !=
						  topics_subs.find(message.topic)))
					{
						topics_subs.insert({message.topic, empty_list});
						continue;
					}

					list<subscription>::iterator s;
					for (s = topics_subs[message.topic].begin();
						 s != topics_subs[message.topic].end(); s++)
					{
						client = s->client;

						sub_message to_send;
						memcpy(&to_send.message, &message, sizeof(message));
						memcpy(&to_send.Rstation, &Rstation, sizeof(sockaddr_in));

						if ((active_ids_sockets.end() ==
							 active_ids_sockets.find(client.id)) &&
							strcmp(&(s->sf), "0") == 0)
						{
							continue;
						}

						if (!(active_ids_sockets.end() ==
							  active_ids_sockets.find(client.id)))
						{
							send(client.socket, &to_send, sizeof(sub_message), 0);

							continue;
						}

						if (!((store_and_forward.find(client.id) !=
							   store_and_forward.end()) ||
							  strcmp(&(s->sf), "1") != 0))
						{
							to_send_list.insert(to_send_list.end(), to_send);
							store_and_forward.insert({client.id, to_send_list});

							continue;
						}

						if (strcmp(&(s->sf), "1") == 0)
							store_and_forward.find(client.id)->second.insert(store_and_forward.find(client.id)->second.end(), to_send);
					}
					continue;
				}

				clientR req;

				n = recv(i, &req, sizeof(req), 0);

				if (n == 0)
				{
					memset(&client_to_delete, 0, sizeof(client_to_delete));
					client_to_delete = active_clients.at(i);

					cout << "Client ";
					cout << client_to_delete.id;
					cout << " disconnected.\n";

					closing(i);

					FD_CLR(i, &read_fds);
					active_ids_sockets.erase(active_clients.at(i).id);
					active_clients.erase(i);

					continue;
				}

				if (n <= -1)
					exit(0);

				if (topics_subs.find(req.topic) == topics_subs.end())
				{
					continue;
				}

				char client_ID[11];
				strcpy(client_ID, active_clients.at(i).id);

				if (!(req.req_type != 0))
				{
					list<subscription>::iterator s;
					for (s = topics_subs[req.topic].begin();
						 s != topics_subs[req.topic].end(); ++s)
					{
						if (!(strcmp(s->client.id, client_ID) != 0))
						{
							s->sf = req.sf;
							continue;
						}
					}
					topics_subs.find(req.topic)->second.insert(topics_subs.find(req.topic)->second.end(), {active_clients.at(i), req.sf});
					continue;
				}

				if (!(req.req_type == 0))
				{
					list<subscription>::iterator s;
					for (s = topics_subs[req.topic].begin();
						 s != topics_subs[req.topic].end(); ++s)
					{
						if (!(strcmp(s->client.id, client_ID) != 0))
						{
							topics_subs.find(req.topic)->second.erase(s);
							continue;
						}
					}
				}
			}
		}
	}

	int i = fd_max;
	while (i)
	{
		if (FD_ISSET(i, &read_fds) != 0)
		{
			closing(sock_tcp);
		}
		i--;
	}

	closing(sock_tcp);

	return 0;
}
