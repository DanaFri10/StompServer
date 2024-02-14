#include <iostream>
#include <fstream>
#include "../include/StompProtocol.h"
#include "../include/Frame.h"
#include "../include/Event.h"

StompProtocol::StompProtocol()
{
}

 std::vector<std::string>* StompProtocol::process(std::string userName, std::vector<std::string>& clientCommand, int& subscriptionCounter, int& receiptCounter, std::map<std::string, int>* topicSubscriptions, std::map<std::string,std::string>* receiptActions)
 {
   std::string command=clientCommand.at(0);
   if (command == "login") return createLoginCommand(clientCommand);
   else if (command == "join") return createJoinCommand(clientCommand, subscriptionCounter, receiptCounter, topicSubscriptions,receiptActions);
   else if (command == "exit") return createExitCommand(clientCommand, receiptCounter, topicSubscriptions, receiptActions);
   else if (command == "report") return createReportCommand(userName, clientCommand, topicSubscriptions);
   else if (command == "logout") return createLogoutCommand(clientCommand, receiptCounter, receiptActions);
   std::vector<std::string>* messages = new std::vector<std::string>();
   return messages;
 }

 std::vector<std::string>* StompProtocol::createLoginCommand(std::vector<std::string>& clientCommand)
 {
   std::vector<std::string>* messages = new std::vector<std::string>();

    std::string frame = "CONNECT\n";
    frame += "accept-version:1.2\n";
    frame += "host:stomp.cs.bgu.ac.il\n";
    frame += "login:" + clientCommand.at(2)+ "\n";
    frame += "passcode:" + clientCommand.at(3)+ "\n";
    frame += "\n\0";

   messages -> push_back(frame);
    return messages;
 }

  std::vector<std::string>* StompProtocol::createJoinCommand(std::vector<std::string>& clientCommand, int& subscriptionCounter, int& receiptCounter, std::map<std::string, int>* topicSubscriptions,std::map<std::string,std::string>* receiptActions)
 {
   std::vector<std::string>* messages = new std::vector<std::string>();

    std::string frame = "SUBSCRIBE\n";
    frame += "destination:/" + clientCommand.at(1) + "\n";
    frame += "id:" + std::to_string(subscriptionCounter) + "\n";
    frame += "receipt:" + std::to_string(receiptCounter) + "\n";
    frame += "\n\0";
    receiptActions->insert({std::to_string(receiptCounter),"Joined channel " + clientCommand.at(1)});
    topicSubscriptions->insert({clientCommand.at(1),subscriptionCounter});
    subscriptionCounter++;
    receiptCounter++;

    messages -> push_back(frame);
    return messages;
 }

std::vector<std::string>* StompProtocol::createExitCommand(std::vector<std::string>& clientCommand, int& receiptCounter, std::map<std::string, int>* topicSubscriptions,std::map<std::string,std::string>* receiptActions)
 {
   std::vector<std::string>* messages = new std::vector<std::string>();

   std::string frame = "UNSUBSCRIBE\n";
   frame += "destination:/" + clientCommand.at(1) + "\n";

   auto it = topicSubscriptions -> find(clientCommand.at(1));
   frame += "id:" + std::to_string(it -> second) + "\n";
   frame += "receipt:" + std::to_string(receiptCounter) + "\n";
   frame += "\n\0";
   receiptActions->insert({std::to_string(receiptCounter),"Exited channel " + clientCommand.at(1)});
   receiptCounter++;

   topicSubscriptions -> erase(clientCommand.at(1));

   messages -> push_back(frame);
   return messages;
 }

 std::vector<std::string>* StompProtocol::createReportCommand(std::string userName, std::vector<std::string>& clientCommand,  std::map<std::string, int>* topicSubscriptions)
 {
    std::vector<std::string>* messages = new std::vector<std::string>();

    std::ifstream reportFile;
    reportFile.open(clientCommand.at(1));
    if(reportFile)// file exists
    {
      reportFile.close();
      names_and_events parsedFile = parseEventsFile(clientCommand.at(1));
      
      for(Event e : parsedFile.events)
      {
         std::string topic = parsedFile.team_a_name + "_" + parsedFile.team_b_name;
         std::string frame = "SEND\n";
         frame += "destination:/" + topic + "\n\n";
         frame += "user: " + userName + "\n";
         frame += "team a: " + parsedFile.team_a_name + "\n";
         frame += "team b: " + parsedFile.team_b_name + "\n";
         frame += "event name: " + e.get_name() + "\n";
         frame += "time: " + std::to_string(e.get_time()) + "\n";
            
         frame += "general game updates:\n";
         for(const auto& update : e.get_game_updates())
         {
            frame += "\t" + (update.first) + ": " + (update.second) + "\n";
         }

         frame += "team a updates:\n";
         for(const auto& update : e.get_team_a_updates())
         {
            frame += "\t" + (update.first) + ": " + (update.second) + "\n";
         }

         frame += "team b updates:\n";
         for(const auto& update : e.get_team_b_updates())
         {
            frame += "\t" + (update.first) + ": " + (update.second) + "\n";
         }
            
         frame += "description:\n" + e.get_description() + "\n\0";

         messages -> push_back(frame);
      }
   }

   else 
   {
      std::cout << "This file does not exist." << clientCommand.at(1) << std::endl;
   }
   return messages;
 }

 std::vector<std::string>* StompProtocol::createLogoutCommand(std::vector<std::string>& clientCommand, int& receiptCounter, std::map<std::string,std::string>* receiptActions)
 {
   std::vector<std::string>* messages = new std::vector<std::string>();

   std::string frame = "DISCONNECT\n";
   frame += "receipt:" + std::to_string(receiptCounter) + "\n";
   frame += "\n\0";
   receiptActions->insert({std::to_string(receiptCounter),"LOGOUT"});
   receiptCounter++;
   messages -> push_back(frame);
   return messages;
 }
