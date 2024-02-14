#pragma once

#include "../include/Event.h"
#include <string>
#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class StompProtocol;

class ConnectionHandler {
private:
	const std::string host_;
	const short port_;
	boost::asio::io_service io_service_;   // Provides core I/O functionality
	tcp::socket socket_;
	StompProtocol& protocol_;

public:
	ConnectionHandler(std::string host, short port, StompProtocol& protocol);
	ConnectionHandler(const ConnectionHandler& other);

	virtual ~ConnectionHandler();

	// Connect to the remote machine
	bool connect();

	// Read a fixed number of bytes from the server - blocking.
	// Returns false in case the connection is closed before bytesToRead bytes can be read.
	bool getBytes(char bytes[], unsigned int bytesToRead);

	// Send a fixed number of bytes from the client - blocking.
	// Returns false in case the connection is closed before all the data is sent.
	bool sendBytes(const char bytes[], int bytesToWrite);

	// Read an ascii line from the server
	// Returns false in case connection closed before a newline can be read.
	bool getFrame(std::string &line);

	// Send an parsed command from the server
	// Returns the frame sent to the server
	bool sendFrame(std::string userName, std::vector<std::string>& clientCommand, int& subscriptionCounter, int& receiptCounter, std::map<std::string, int>* topicSubscriptions, std::map<std::string, std::string>* receiptActions);

	// Get Ascii data from the server until the delimiter character
	// Returns false in case connection closed before null can be read.
	bool getFrameAscii(std::string &frame, char delimiter);

	// Send a message to the remote host.
	// Returns false in case connection is closed before all the data is sent.
	bool sendFrameAscii(const std::string &frame, char delimiter);

	// Close down the connection properly.
	void close();

}; //class ConnectionHandler
