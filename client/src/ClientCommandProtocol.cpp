#include <iostream>
#include <fstream>
#include "../include/ClientCommandProtocol.h"
#include "../include/StompProtocol.h"
#include "../include/Frame.h"
#include "../include/Event.h"
#include <boost/lexical_cast.hpp>
#include <thread>
#include <mutex>
#include <chrono>

ClientCommandProtocol::ClientCommandProtocol() : lastSubscriptionId(0), lastReceiptId(0), userName(""), topicSubscriptions(new std::map<std::string, int>()), usersGames(new std::map<std::string, std::vector<Game*>>()), receiptActions(new std::map<std::string, std::string>()), mu1(), mu3(), cond(), receivedResponse(false), isConnected(false), handler(nullptr), terminate(false)
{}

ClientCommandProtocol::~ClientCommandProtocol()
{
    delete topicSubscriptions;
    delete receiptActions;

    for(auto const& games:(*usersGames))
    {
        std::vector<Game*> temp = games.second;
        for(auto game : temp)
        {
            delete game;
        }
        temp.clear();
    }
    delete usersGames;
}

ClientCommandProtocol::ClientCommandProtocol(const ClientCommandProtocol& other) : lastSubscriptionId(other.lastSubscriptionId), lastReceiptId(other.lastReceiptId), userName(other.userName),
topicSubscriptions(new std::map<std::string, int>()), usersGames(new std::map<std::string, std::vector<Game*>>()), receiptActions(new std::map<std::string, std::string>()), 
mu1(), mu3(), cond(), receivedResponse(other.receivedResponse), isConnected(other.isConnected), handler(new ConnectionHandler(*other.handler)), terminate(other.terminate)
{
    for(auto const& topic:(*other.topicSubscriptions))
    {
        this -> topicSubscriptions -> insert({topic.first, topic.second});
    }

    for(auto const& user:(*other.usersGames))
    {
        std::vector<Game*> v;
        for(Game* g : user.second)
        {
            v.push_back(new Game(*g));
        }
        this -> usersGames -> insert({user.first, v});
    }
    
    for(auto const& action:(*other.receiptActions))
    {
        this -> receiptActions -> insert({action.first, action.second});
    } 
}

ClientCommandProtocol::ClientCommandProtocol(const ClientCommandProtocol&& other) : lastSubscriptionId(other.lastSubscriptionId), lastReceiptId(other.lastReceiptId), userName(other.userName),
topicSubscriptions(other.topicSubscriptions), usersGames(other.usersGames), receiptActions(other.receiptActions), 
mu1(), mu3(), cond(), receivedResponse(other.receivedResponse), isConnected(other.isConnected), handler(other.handler), terminate(other.terminate)

{}

ClientCommandProtocol& ClientCommandProtocol::operator=(const ClientCommandProtocol& other)
{
    this -> lastSubscriptionId = other.lastSubscriptionId;
    this -> lastReceiptId = other.lastReceiptId;
    this -> handler = new ConnectionHandler(*other.handler);
    this -> userName = other.userName;

    this -> topicSubscriptions = new std::map<std::string, int>();
    for(auto const& topic:(*other.topicSubscriptions))
    {
        this -> topicSubscriptions -> insert({topic.first, topic.second});
    }

    this -> usersGames = new std::map<std::string, std::vector<Game*>>();
    for(auto const& user:(*other.usersGames))
    {
        std::vector<Game*> v;
        for(Game* g : user.second)
        {
            v.push_back(new Game(*g));
        }
        this -> usersGames -> insert({user.first, v});
    }
    
    this -> receiptActions = new std::map<std::string, std::string>();
    for(auto const& action:(*other.receiptActions))
    {
        this -> receiptActions -> insert({action.first, action.second});
    }
    
    this -> receivedResponse = other.receivedResponse;
    return *this;
}

ClientCommandProtocol& ClientCommandProtocol::operator=(ClientCommandProtocol&& other)
{
    this -> lastSubscriptionId = other.lastSubscriptionId;
    this -> lastReceiptId = other.lastReceiptId;
    this -> handler = other.handler;
    this -> userName = other.userName;
    this -> topicSubscriptions = other.topicSubscriptions;
    this -> usersGames = other.usersGames;
    this -> receiptActions = other.receiptActions;
    this -> receivedResponse = other.receivedResponse;
    return *this;
}

bool ClientCommandProtocol::processCommand(std::string& msg){
    receivedResponse = false;

    std::vector<std::string> split;
    size_t pos = 0;
    std::string token;
    while ((pos = msg.find(" ")) != std::string::npos) {
        token = msg.substr(0, pos);
        split.push_back(token);
        msg = msg.erase(0, pos + 1);
    }
    split.push_back(msg);

    std::string command = split.at(0);
    if(command == "quit"){
        if(!processQuit())
        {
            return false;
        }
        return true;
    }
    else if(command == "login"){
        if(!processLogin(split))
        {
            return false;
        }
    }
    else if(command == "summary"){
        if(!processSummary(split))
        {
            return false;
        }
    }
    else if(command == "join"){
        if(!processJoin(split))
        {
            return false;
        }
    }
    else if(command == "report"){
        if(!processReport(split))
        {
            return false;
        }
    }
    else if(command == "exit"){
        if(!processExit(split))
        {
            return false;
        }
    }
    else if(command == "logout"){
        if(!processLogout(split))
        {
            return false;
        }
    }
    else{
        std::cout << "Unknown command\n";
        return false;
    }

    std::unique_lock<std::mutex> lock1(mu1);
    bool sentLine = handler->sendFrame(userName, split,lastSubscriptionId,lastReceiptId, topicSubscriptions, receiptActions);
    if(isConnected && command != "summary" && command != "report")
    {
        cond.wait(lock1, [&]{return receivedResponse;});
    }
    lock1.unlock();
    if(!sentLine && isConnected){
        handler->close();
        isConnected = false;
        std::cout << "Failed to communicate with server\n";
    }
    return sentLine;
}


bool ClientCommandProtocol::processQuit()
{
    if(isConnected){
        std::vector<std::string> logout;
        logout.push_back("logout");
        handler->sendFrame(userName, logout,lastSubscriptionId,lastReceiptId, topicSubscriptions, receiptActions);
    }
    
    while(isConnected){}
    std::cout << "Goodbye." << "\n";
    terminate = true;
    return true;
}

bool ClientCommandProtocol::processLogin(std::vector<std::string>& split)
{
    // error detection
    if(isConnected){
        std::cout << "The client is already logged in, log out before trying again\n";
        return false;
    }
    if(split.size() < 4){
        std:: cout << "Missing arguments\n";
        return false;
    }
    if(split.size() > 4){
        std:: cout << "Too many arguments\n";
        return false;
    } 
    std::string hostPort = split.at(1);
    userName = split.at(2);
    if(hostPort.find(':') == std::string::npos){
        std::cout << "Login command missing port\n";
        return false;
    }
    
    StompProtocol protocol;
    handler = new ConnectionHandler(hostPort.substr(0, hostPort.find(':')),boost::lexical_cast<short>(hostPort.substr(hostPort.find(':')+1)),protocol);
    if(handler->connect()){
        isConnected = true;
        std::thread readServer(&ClientCommandProtocol::readWhileConnected, this);
        readServer.detach();
    }
    return true;
}

void ClientCommandProtocol::readWhileConnected()
{
    while(isConnected){
        std::string answer;
                
        if(handler->getFrame(answer)){
            if(answer != ""){
                Frame f = Frame(answer);
                if(f.getCommand() == "CONNECTED" || f.getCommand() == "ERROR"){
                    std::unique_lock<std::mutex> lock1(mu1);
                    if(f.headerExists("message")){
                        std::cout << f.getHeader("message") << "\n\n"; 
                    }
                    std::cout << f.getBody();
                    receivedResponse=true;
                          
                    lock1.unlock();
                    cond.notify_all();
                }
                        
                else if(f.getCommand() == "RECEIPT"){
                    std::unique_lock<std::mutex> lock1(mu1);
                    std::string id = f.getHeader("receipt-id");
                    auto it = receiptActions->find(id);
                    std::string action = it->second;
                    if(action == "LOGOUT"){
                        isConnected = false;
                    }
                    else{
                        std::cout << action << "\n";
                    }
                    receivedResponse=true;
                            
                    lock1.unlock();
                    cond.notify_all();
                }
                
                else if(f.getCommand() == "MESSAGE"){
                    std::string gameName = f.getHeader("destination").substr(1);
                    std::string team_a = gameName.substr(0,gameName.find("_"));
                    std::string team_b = gameName.substr(gameName.find("_")+1);
                    std::string frameBody = f.getBody();
                    int startOfName = frameBody.find(':')+2;
                    int endOfName = frameBody.find('\n');
                    std::string name = frameBody.substr(startOfName, endOfName-startOfName);
                            
                    std::unique_lock<std::mutex> lock3(mu3);
                    if(usersGames->find(name) == usersGames->end()){
                        std::vector<Game*> events;
                        usersGames->insert({name,events});
                    }
                            
                    auto it = usersGames -> find(name);
                    std::vector<Game*>& userEvents = (it -> second);
                    Game* game = nullptr;
                    if(!userEvents.empty()){
                        for(Game* g : userEvents){
                            if(g->getTeamAName() == team_a && g->getTeamBName() == team_b){
                                game = g;
                                break;
                            }
                        }
                    }
                    if(game == nullptr){
                        game = new Game(team_a, team_b);
                        userEvents.push_back(game);
                    }
                    lock3.unlock();

                    std::string event = frameBody.substr(endOfName+1);
                    Event e(event);
                    game->addEvent(e);
                }
            }
        }
        
        else{
            isConnected = false;
        }
    }
    handler->close();
    delete handler;
}

bool ClientCommandProtocol::processSummary(std::vector<std::string>& split)
{
    if(split.size() < 4){
        std:: cout << "Missing arguments\n";
        return false;
    }
    if(split.size() > 4){
        std:: cout << "Too many arguments\n";
        return false;
    }
    
    std::string gameName = split.at(1);
    std::string teamA = gameName.substr(0, gameName.find('_'));
    std::string teamB = gameName.substr(gameName.find('_') + 1, gameName.length() - teamA.length() -1);
    
    if(teamA == teamB){
        std:: cout << "Illegal topic name.\n";
        return false;
    }
    
    printSummary(split.at(2), teamA, teamB, split.at(3));
    return true;
}

void ClientCommandProtocol::printSummary(std::string& user, std::string& teamA, std::string& teamB, std::string& file)
{
    std::unique_lock<std::mutex> lock3(mu3);
    if(usersGames->find(user) == usersGames->end()){
        std::cout << "Never recieved a report from " << user << ".\n";
        return;
    }
    if(topicSubscriptions->find(teamA + "_" + teamB) == topicSubscriptions->end()){
        std::cout << "Not subscribed to topic " << teamA << "_" << teamB << ".\n";
        return;
    }
    
    std::string summary = teamA + " vs " + teamB + "\nGame stats:\n";
    std::string statsA = teamA + "stats:\n";
    std::string statsB = teamB + "stats:\n";
    bool gameFound = false;
    auto it = usersGames -> find(user);
    std::vector<Game*>& userGames = (it -> second);
    for(Game* g : userGames)
    {
        std::string teamAName = g->getTeamAName();
        std::string teamBName = g->getTeamBName();
        if(teamAName == teamA && teamBName == teamB)
        {
            gameFound = true;
            std::string *generalStats = g->getGeneralStats();
            std::string *teamAStats = g->getTeamAStats();
            std::string *teamBStats = g->getTeamBStats();
            std::string *gameEvents = g->getGameEvents();

            summary += *generalStats + *teamAStats + *teamBStats;
            summary += "Game event reports:\n";
            summary += *gameEvents;

            delete generalStats;
            delete teamAStats;
            delete teamBStats;
            delete gameEvents;
        }
    }
    lock3.unlock();

    //print to file
    if(gameFound)
    {
        std::ofstream sumFile;
        sumFile.open(file);
        sumFile << summary;
        sumFile.close();
    }
    else
    {
        std::cout << "No report on topic " << teamA << "_" << teamB << " recieved from the user " << user << "." <<std::endl;
    }
}

bool ClientCommandProtocol::processJoin(std::vector<std::string>& split)
{
    if(split.size() < 2){
        std:: cout << "Missing arguments\n";
        return false;
    }
    if(split.size() > 2){
        std:: cout << "Too many arguments\n";
        return false;
    }
    return true;
}

bool ClientCommandProtocol::processReport(std::vector<std::string>& split)
{
    if(split.size() < 2){
        std:: cout << "Missing arguments\n";
        return false;
    }
    if(split.size() > 2){
        std:: cout << "Too many arguments\n";
        return false;
    }
    return true;
}

bool ClientCommandProtocol::processExit(std::vector<std::string>& split)
{
    if(split.size() < 2){
        std:: cout << "Missing arguments\n";
        return false;
    }
    if(split.size() > 2){
        std:: cout << "Too many arguments\n";
        return false;
    }
    return true;
}

bool ClientCommandProtocol::processLogout(std::vector<std::string>& split)
{
    if(split.size() > 1){
        std:: cout << "Too many arguments\n";
        return false;
    }
    return true;
}