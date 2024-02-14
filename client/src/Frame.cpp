#include "../include/Frame.h"
#include <iostream>
#include <map>

Frame::Frame(std::string command, std::map<std::string, std::string> headers, std::string body) : 
command(command), headers(headers), body(body)
{}

Frame::Frame(std::string frame) : headers(){
    int endOfCommand=frame.find('\n');
    command=frame.substr(0,endOfCommand);
    frame=frame.substr(endOfCommand+1);
    std::vector<std::string> split;
    size_t pos = 0;
    std::string token;
    while ((pos = frame.find('\n')) != std::string::npos) {
        token = frame.substr(0, pos);
        split.push_back(token);
        frame = frame.erase(0, pos + 1);
    }
    split.push_back(frame);
    int i = 0;
    while(split.at(i) != ""){
        std::string headerLine = split.at(i);
        int divider = headerLine.find(':');
        headers[headerLine.substr(0,divider)] = headerLine.substr(divider+1);
        i++;
    }
    i++;
    body = "";
    while(split.at(i) != "\0"){
        body += split.at(i) + "\n";
        i++;
    }
}

void Frame::addCommand(std::string c)
{
    command=c;
}

void Frame::addHeader(std::string name, std::string value)
{
    headers[name] = value;
}

void Frame::addBody(std::string b)
{
    body=b;
}

const std::string& Frame::toString() const
{
    std::string res=command + '\n';
    for (auto const& header : headers)
    {
        res += header.first + ':' + header.second + '\n';
    }
    res += body + '\n' + '\0';
    return res;
}

std::string Frame::getCommand(){
    return command;
}
std::string Frame::getBody(){
    return body;
}
std::string Frame::getHeader(std::string name){
    auto id = headers.find(name);
    return id->second;
}

bool Frame::headerExists(std::string name){
    return headers.find(name) != headers.end();
}



