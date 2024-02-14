#include <thread>
#include <stdlib.h>
#include "../include/ClientCommandProtocol.h"

int main(int argc, char *argv[]) {
    ClientCommandProtocol commandClient;
    while(!commandClient.terminate)
    {
        std::string line;
        std::getline(std::cin, line);
        commandClient.processCommand(line);
    }
    return 0;
}