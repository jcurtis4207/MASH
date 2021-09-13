#pragma once
#include <fstream>
#include <iostream>
#include <map>

using namespace std;

map<string, string> aliases;
extern bool aliasesOpen;

void parseAlias(string input)
{
    if(input.empty())
        return;
    else if(input.substr(0, 5) != "alias")
        return;
    string alias;
    string command;
    unsigned long index = 6;
    while(input[index] != '=')
        alias.append(1, input[index++]);
    index++;
    while(index < input.size())
    {
        if(input[index] == '#')
            break;
        else if(input[index] == '"')
            index++;
        else
            command.append(1, input[index++]);
    }
    aliases.insert(pair<string, string>(alias, command));
}

void openAliasesFile()
{
    aliases.clear();
    string fileLocation = getenv("HOME");
    fileLocation.append("/.mash_aliases");
    fstream aliasFile;
    aliasFile.open(fileLocation, ios::in);
    if(!aliasFile)
    {
        aliasesOpen = false;
        return;
    }
    string line;
    while(aliasFile)
    {
        getline(aliasFile, line);
        parseAlias(line);
    }
    aliasFile.close();
    aliasesOpen = true;
}