#pragma once

#include <string>
#include <map>
#include <vector>
#include <mutex>
#include <condition_variable>
#include "ConnectionHandler.h"
#include "../include/Game.h"

class Event;

class ClientCommandProtocol
{
private:
    int lastSubscriptionId;
    int lastReceiptId;
    std::string userName;
    std::map<std::string, int>* topicSubscriptions;
    std::map<std::string, std::vector<Game*>>* usersGames;
    std::map<std::string, std::string>* receiptActions;
    std::mutex mu1;
    std::mutex mu3;
    std::condition_variable cond;
    bool receivedResponse;
    bool isConnected;
    void printSummary(std::string& user, std::string& teamA, std::string& teamB, std::string& file);
    bool processQuit();
    bool processLogin(std::vector<std::string>& split);
    void readWhileConnected();
    bool processSummary(std::vector<std::string>& split);
    bool processJoin(std::vector<std::string>& split);
    bool processReport(std::vector<std::string>& split);
    bool processExit(std::vector<std::string>& split);
    bool processLogout(std::vector<std::string>& split);
    

public:
    ConnectionHandler* handler;
    bool terminate;
    ClientCommandProtocol();
    virtual ~ClientCommandProtocol();
    ClientCommandProtocol(const ClientCommandProtocol& other);
    ClientCommandProtocol(const ClientCommandProtocol&& other);
    ClientCommandProtocol& operator=(const ClientCommandProtocol& other);
    ClientCommandProtocol& operator=(ClientCommandProtocol&& other);
    bool processCommand(std::string& command);
   
};