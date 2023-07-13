CFLAGS = -Wall -Wextra -g -lm

all: server subscriber

server: server.cpp
	g++ $(CFLAGS) -std=c++17 server.cpp -o server

subscriber: subscriber.cpp 
	g++ $(CFLAGS) -std=c++17 subscriber.cpp -o subscriber

clean:
	rm -f server subscriber
