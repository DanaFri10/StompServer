#include <vector>
#include <string>
#include <map>
#include <mutex>
#include "../include/Event.h"

class Game
{
    private:
        std::string teamAName;
        std::string teamBName;
        std::vector<std::string> gameEvents;
        std::map<std::string, std::string> generalStats;
        std::map<std::string, std::string> teamAStats;
        std::map<std::string, std::string> teamBStats;
        std::mutex mu2;

    public:
        Game(std::string teamAName, std::string teamBName);
        Game(const Game& other);
        std::string getTeamAName();
        std::string getTeamBName();
        std::string* getGameEvents();
        std::string* getGeneralStats();
        std::string* getTeamAStats();
        std::string* getTeamBStats();
        void addEvent(Event e);
};