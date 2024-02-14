#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <thread>
#include "../include/Game.h"
#include "../include/Event.h"

Game::Game(std::string teamAName, std::string teamBName) : teamAName(teamAName), teamBName(teamBName), gameEvents(), generalStats(), teamAStats(), teamBStats(), mu2()
{}

Game::Game(const Game& other) : teamAName(other.teamAName), teamBName(other.teamBName), gameEvents(), generalStats(), teamAStats(), teamBStats(), mu2()
{
    for(std::string event:other.gameEvents)
    {
        this -> gameEvents.push_back(event);
    }

    for(auto const& stat:(other.generalStats))
    {
        this -> generalStats.insert({stat.first, stat.second});
    }
        
    for(auto const& stat:(other.teamAStats))
    {
        this -> teamAStats.insert({stat.first, stat.second});
    }
    
    for(auto const& stat:(other.teamBStats))
    {
        this -> teamBStats.insert({stat.first, stat.second});
    }
}

std::string Game::getTeamAName()
{
    return teamAName;
}

std::string Game::getTeamBName()
{
    return teamBName;
}

std::string* Game::getGameEvents()
{
    std::unique_lock<std::mutex> lock2(mu2);

    std::sort(gameEvents.begin(), gameEvents.end(), [](std::string& e1, std::string& e2){ int time1 = stoi(e1.substr(0, e1.find('-')-1)); int time2 = stoi(e2.substr(0, e2.find('-')-1)); return (time1 < time2);});

    std::string* res = new std::string("");
    for(std::string e : gameEvents)
    {
        *res += e;
    }
    return res;
}

std::string* Game::getGeneralStats()
{
    std::unique_lock<std::mutex> lock2(mu2);

    std::string* res = new std::string("General stats:\n");
    for(const auto& stat : generalStats)
    {
        *res += "\t" + (stat.first) + ":" + (stat.second) + "\n";
    }
    return res;
}

std::string* Game::getTeamAStats()
{
    std::unique_lock<std::mutex> lock2(mu2);

    std::string* res = new std::string(teamAName + " stats:\n");
    for(const auto& stat : teamAStats)
    {
        *res += "\t" + (stat.first) + ":" + (stat.second) + "\n";
    }
    return res;
}

std::string* Game::getTeamBStats()
{
    std::unique_lock<std::mutex> lock2(mu2);

    std::string* res = new std::string(teamBName + " stats:\n");
    for(const auto& stat : teamBStats)
    {
        *res += "\t" + (stat.first) + ":" + (stat.second) + "\n";
    }
    return res;
}

void Game::addEvent(Event e){
    std::unique_lock<std::mutex> lock2(mu2);
    std::string res = "";
    res += std::to_string(e.get_time()) + " - " + e.get_name() + ":\n";
    res += e.get_description();
    gameEvents.push_back(res);
    for(const auto &t : e.get_game_updates()){
        generalStats[t.first] = t.second;
    }
    for(const auto &t : e.get_team_a_updates()){
        teamAStats[t.first] = t.second;
    }
    for(const auto &t : e.get_team_b_updates()){
        teamBStats[t.first] = t.second;
    }
    lock2.unlock();
}


