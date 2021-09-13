/*
* This file contains all of the functions necessary to keep command history
* The up and down arrows cycle through the history
*/

#pragma once
#include <iostream>
#include <vector>

using namespace std;

#define HISTORY_MAX_SIZE 10

vector<string> history;
unsigned long historyIndex = 0;

void pushHistory(string input)
{
    if(!input.empty())
        history.push_back(input);
    if(history.size() > HISTORY_MAX_SIZE)
        history.erase(history.begin());
    historyIndex = history.size() - 1;
}

string getOlderHistoryEntry()
{
    if(historyIndex == 0)
        return history[0];
    else
        return history[historyIndex--];
}

string getNewerHistoryEntry()
{
    if(historyIndex == history.size() - 1)
        return "";
    else
        return history[historyIndex++];
}

string getLastArg()
{
    string command = history[history.size() - 1];
    unsigned long i = command.size() -1;
    while(i > 0)
    {
        if(command[--i] == ' ')
            break;
    }
    if(i == 0)
        return "";
    else
        return command.substr(i + 1);
}