CFLAGS:=-c -Wall -Weffc++ -g -std=c++11 -Iinclude
LDFLAGS:=-lboost_system -lpthread

all: StompWCIClient

StompWCIClient: bin/ConnectionHandler.o bin/StompClient.o bin/Event.o bin/ClientCommandProtocol.o bin/StompProtocol.o bin/Game.o bin/Frame.o
	g++ -o bin/StompWCIClient bin/ConnectionHandler.o bin/StompClient.o bin/Event.o bin/ClientCommandProtocol.o bin/StompProtocol.o bin/Game.o bin/Frame.o $(LDFLAGS)


bin/ConnectionHandler.o: src/ConnectionHandler.cpp
	g++ $(CFLAGS) -o bin/ConnectionHandler.o src/ConnectionHandler.cpp
	
bin/StompProtocol.o: src/StompProtocol.cpp
	g++ $(CFLAGS) -o bin/StompProtocol.o src/StompProtocol.cpp

bin/Event.o: src/Event.cpp
	g++ $(CFLAGS) -o bin/Event.o src/Event.cpp

bin/StompClient.o: src/StompClient.cpp
	g++ $(CFLAGS) -o bin/StompClient.o src/StompClient.cpp

bin/ClientCommandProtocol.o: src/ClientCommandProtocol.cpp
	g++ $(CFLAGS) -o bin/ClientCommandProtocol.o src/ClientCommandProtocol.cpp

bin/Game.o: src/Game.cpp
	g++ $(CFLAGS) -o bin/Game.o src/Game.cpp

bin/Frame.o: src/Frame.cpp
	g++ $(CFLAGS) -o bin/Frame.o src/Frame.cpp

.PHONY: clean
clean:
	rm -f bin/*
	