#pragma once

#include "../include/Event.h"
#include <string>
#include <map>
#include <vector>

// TODO: implement the STOMP protocol
class StompProtocol
{
private:
    std::vector<std::string>* createLoginCommand(std::vector<std::string>& clientCommand);
    std::vector<std::string>* createJoinCommand(std::vector<std::string>& clientCommand, int& subscriptionCounter, int& receiptCounter, std::map<std::string, int>* topicSubscriptions,std::map<std::string, std::string>* receiptActions);
    std::vector<std::string>* createExitCommand(std::vector<std::string>& clientCommand, int& receiptCounter, std::map<std::string, int>* topicSubscriptions,std::map<std::string, std::string>* receiptActions);
    std::vector<std::string>* createReportCommand(std::string userName, std::vector<std::string>& clientCommand,  std::map<std::string, int>* topicSubscriptions);
    std::vector<std::string>* createLogoutCommand(std::vector<std::string>& clientCommand, int& receiptCounter, std::map<std::string, std::string>* receiptActions);

public:
    StompProtocol();
    std::vector<std::string>* process(std::string userName, std::vector<std::string>& clientCommand, int& subscriptionCounter, int& receiptCounter, std::map<std::string, int>* topicSubscriptions, std::map<std::string, std::string>* receiptActions);
};