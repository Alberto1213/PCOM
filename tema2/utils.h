#include <bits/stdc++.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <vector>

#define SF_OFF '0'
#define SF_ON '1'

using namespace std;

struct tcpC {
	char id[11];
	int socket;
};

struct msgU {
	char topic[51];
	char dataT;
	char content[1500];
};

struct subscription {
	tcpC client;
	char sf;
};

struct clientR {
	char topic[51];
	char sf;
	int req_type;
};

struct sub_message {
    msgU message;
    sockaddr_in Rstation;
};

