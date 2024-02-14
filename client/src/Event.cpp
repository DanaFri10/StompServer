#include "../include/Event.h"
#include "../include/json.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <sstream>
using json = nlohmann::json;

Event::Event(std::string team_a_name, std::string team_b_name, std::string name, int time,
             std::map<std::string, std::string> game_updates, std::map<std::string, std::string> team_a_updates,
             std::map<std::string, std::string> team_b_updates, std::string description)
    : team_a_name(team_a_name), team_b_name(team_b_name), name(name),
      time(time), game_updates(game_updates), team_a_updates(team_a_updates),
      team_b_updates(team_b_updates), description(description)
{
}

Event::~Event()
{
}

const std::string &Event::get_team_a_name() const
{
    return this->team_a_name;
}

const std::string &Event::get_team_b_name() const
{
    return this->team_b_name;
}

const std::string &Event::get_name() const
{
    return this->name;
}

int Event::get_time() const
{
    return this->time;
}

const std::map<std::string, std::string> &Event::get_game_updates() const
{
    return this->game_updates;
}

const std::map<std::string, std::string> &Event::get_team_a_updates() const
{
    return this->team_a_updates;
}

const std::map<std::string, std::string> &Event::get_team_b_updates() const
{
    return this->team_b_updates;
}

const std::string &Event::get_description() const
{
    return this->description;
}

Event::Event(const std::string &frame_body) : team_a_name(""), team_b_name(""), name(""), time(0), game_updates(), team_a_updates(), team_b_updates(), description("")
{
    std::string body = frame_body;
    std::vector<std::string> split;
    size_t pos = 0;
    std::string token;
    while ((pos = body.find('\n')) != std::string::npos) {
        token = body.substr(0, pos);
        split.push_back(token);
        body = body.erase(0, pos + 1);
    }
    split.push_back(body);
    bool generalUpdates = false;
    bool teamAUpdates = false;
    bool teamBUpdates = false;
    bool descriptionRead = false;
    for(int i = 0; i < split.size(); i++){
        std::string line = split.at(i);
        int divider = line.find(':');
        std::string type = line.substr(0,divider);
        if(type == "event name"){
            name = line.substr(divider+2);
        }
        if(type == "team a"){
            team_a_name = line.substr(divider+2);
        }
        if(type == "team b"){
            team_b_name = line.substr(divider+2);
        }
        if (type == "time")
        {
            time = std::stoi(line.substr(divider+2));
        }
        if(generalUpdates){
            if(type.at(0) != '\t'){
                generalUpdates = false;
            }else{
                type = type.substr(1);
                if(type != "active" && type != "before halftime"){
                    game_updates[type] = line.substr(divider+2);
                }
            }
        }
        if(teamAUpdates){
            if(type.at(0) != '\t'){
                teamAUpdates = false;
            }else{
                type = type.substr(1);
                team_a_updates[type] = line.substr(divider+2);
            }
        }
        if(teamBUpdates){
            if(type.at(0) != '\t'){
                teamBUpdates = false;
            }else{
                type = type.substr(1);
                team_b_updates[type] = line.substr(divider+2);
            }
        }
        if(descriptionRead){
            description+=line + "\n";
        }
        if(type == "general game updates"){
            generalUpdates = true;
        }
        if(type == "team a updates"){
            teamAUpdates = true;
        }
        if(type == "team b updates"){
            teamBUpdates = true;
        }
        if(type == "description"){
            descriptionRead = true;
        }
    }
}

names_and_events parseEventsFile(std::string json_path)
{
    std::ifstream f(json_path);
    json data = json::parse(f);

    std::string team_a_name = data["team a"];
    std::string team_b_name = data["team b"];

    // run over all the events and convert them to Event objects
    std::vector<Event> events;
    for (auto &event : data["events"])
    {
        std::string name = event["event name"];
        int time = event["time"];
        std::string description = event["description"];
        std::map<std::string, std::string> game_updates;
        std::map<std::string, std::string> team_a_updates;
        std::map<std::string, std::string> team_b_updates;
        for (auto &update : event["general game updates"].items())
        {
            if (update.value().is_string())
                game_updates[update.key()] = update.value();
            else
                game_updates[update.key()] = update.value().dump();
        }

        for (auto &update : event["team a updates"].items())
        {
            if (update.value().is_string())
                team_a_updates[update.key()] = update.value();
            else
                team_a_updates[update.key()] = update.value().dump();
        }

        for (auto &update : event["team b updates"].items())
        {
            if (update.value().is_string())
                team_b_updates[update.key()] = update.value();
            else
                team_b_updates[update.key()] = update.value().dump();
        }
        
        events.push_back(Event(team_a_name, team_b_name, name, time, game_updates, team_a_updates, team_b_updates, description));
    }
    names_and_events events_and_names{team_a_name, team_b_name, events};

    return events_and_names;
}