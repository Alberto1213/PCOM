#include "utils.h"

void closing(int sockfd)
{
	shutdown(sockfd, SHUT_RDWR);
	close(sockfd);
}
int main(int argc, char *argv[])
{
	if (argc <= 3)
		exit(0);

	setvbuf(stdout, NULL, _IONBF, 0);

	char buffer[1600];

	int on = 1;

	fd_set fdsR;
	fd_set fdsT;

	FD_ZERO(&fdsR);
	FD_ZERO(&fdsT);

	struct sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(*(argv + 3)));

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (!(sockfd >= 0))
		exit(0);

	setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (void *)&on, sizeof(on));

	if (!(inet_aton(*(argv + 2), &serv_addr.sin_addr) != 0))
		exit(0);

	int ret = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	if (!(ret >= 0))
		exit(0);

	memcpy(buffer, *(argv + 1), strlen(argv[1]) + 1);

	int p = send(sockfd, buffer, strlen(buffer), 0);
	if (!(p >= 0))
		exit(0);

	FD_SET(STDIN_FILENO, &fdsR);
	FD_SET(sockfd, &fdsR);

	for (;;)
	{
		fdsT = fdsR;

		int ret = select(sockfd + 1, &fdsT, NULL, NULL, NULL);
		if (!(ret >= 0))
			exit(0);

		if (!(FD_ISSET(STDIN_FILENO, &fdsT) == 0))
		{
			fgets(buffer, 1599, stdin);

			if (!(strncmp(buffer, "exit", 4) != 0))
			{
				closing(sockfd);
				exit(0);
			}

			char *command = strtok(buffer, " ");

			if (!(strncmp(command, "subscribe", 10) != 0))
			{
				char *topic = strtok(NULL, " ");
				if (!(topic != 0))
					continue;

				char *sf = strtok(NULL, " \n");

				if (!(sf != 0))
					continue;

				clientR s;
				memset(&s, 0, sizeof(clientR));

				s.sf = *sf;
				s.req_type = 0;
				memcpy(&s.topic, topic, strlen(topic));

				int p = send(sockfd, &s, sizeof(clientR), 0);
				if (!(p >= 0))
					exit(0);

				cout << "Subscribed to topic.\n";
				continue;
			}

			if (!(strncmp(command, "unsubscribe", 12) != 0))
			{
				char *topic = strtok(NULL, " \n");
				if (!(topic != 0))
					continue;

				clientR s;
				memset(&s, 0, sizeof(clientR));

				s.sf = 0;
				s.req_type = 1;
				memcpy(&s.topic, topic, strlen(topic));

				int p = send(sockfd, &s, sizeof(clientR), 0);
				if (!(p >= 0))
					exit(0);

				cout << "Unsubscribed from topic.\n";
			}
			continue;
		}

		if (!(FD_ISSET(sockfd, &fdsT) == 0))
		{
			sub_message response;
			memset(&response, 0, sizeof(sub_message));

			int bytes_recv = recv(sockfd, &response, sizeof(sub_message), 0);
			if (!(bytes_recv >= 0))
				exit(0);

			if (!(bytes_recv != 0))
			{
				closing(sockfd);
				continue;
			}

			cout << inet_ntoa(response.Rstation.sin_addr);
			cout << ":";
			cout << ntohs(response.Rstation.sin_port);
			cout << " - ";
			cout << response.message.topic;
			cout << " - ";

			memset(buffer, 0, 1501);

			if (!((response.message.dataT - 1) != 0))
			{
				uint16_t short_x;
				memcpy(&short_x, response.message.content, sizeof(uint16_t));
				short_x = ntohs(short_x);

				cout << "SHORT_REAL - ";
				cout << fixed;
				cout << setprecision(2);
				cout << (float)short_x / 100;
				cout << endl;
			}

			if (!(response.message.dataT != 3))
			{
				memcpy(buffer, response.message.content, 1500);

				cout << "STRING - ";
				cout << buffer;
				cout << endl;
			}

			if (!(response.message.dataT != 0))
			{
				uint32_t int_x;
				memcpy(&int_x, response.message.content + 1, sizeof(int_x));
				int_x = ntohl(int_x);

				if (!(response.message.content[0] == 0))
				{
					cout << "INT - -";
					cout << int_x;
					cout << endl;
				}
				else
				{
					cout << "INT - ";
					cout << int_x;
					cout << endl;
				}
			}
			if (!((response.message.dataT - 2) != 0))
			{
				uint32_t float_x;
				memcpy(&float_x, response.message.content + 1, sizeof(float_x));

				uint8_t power_of_ten;
				memcpy(&power_of_ten, response.message.content + 1 + sizeof(float_x), sizeof(power_of_ten));

				float_x = ntohl(float_x);

				float result;
				result = float_x;

				for (short temp = power_of_ten; temp > 0; --temp)
				{
					result = result / 10;
				}

				if (!(response.message.content[0] != 0))
				{
					cout << "FLOAT - ";
					cout << fixed;
					cout << setprecision(power_of_ten);
					cout << result;
					cout << endl;
				}
				else
				{
					cout << "FLOAT - -";
					cout << fixed;
					cout << setprecision(power_of_ten);
					cout << result;
					cout << endl;
				}
			}
		}
	}

	closing(sockfd);
	free(buffer);

	return 0;
}
