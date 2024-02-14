#pragma once


#include <string>
#include <iostream>
#include <map>
#include <vector>

class Frame
{
private:
    std::string command;
    std::map<std::string, std::string> headers;
    std::string body;

public:
    Frame(std::string command, std::map<std::string, std::string> headers, std::string body);
    Frame(std::string frame);
    void addCommand(std::string c);
    void addHeader(std::string name, std::string value);
    void addBody(std::string b);
    std::string getCommand();
    std::string getHeader(std::string name);
    std::string getBody();
    bool headerExists(std::string name);
    const std::string &toString() const;
};