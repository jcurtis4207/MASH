/*
* This file contains all of functions necessary to parse user input
*/

#pragma once
#include <iostream>
#include <sstream>
#include <vector>

using namespace std;

#define SEMICOLON 0
#define DOUBLE_AND 1
#define DOUBLE_OR 2
#define PIPE 3

struct Command
{
    string program;
    int op = 0;
    Command(const string prog, const int oper)
        : program(prog), op(oper){}
};

string getInput()
{
    string line;
    getline(cin, line);
    while(line[line.size() - 1] == '\\')
    {
        cout << "> ";
        line = line.substr(0, line.size() - 1);
        string temp;
        getline(cin, temp);
        line.append(temp);
    }
    return line;
}

int getCommands(vector<Command>& commands, string input)
{
    stringstream builder;
    for(unsigned long i = 0; i < input.length(); i++)
    {
        if(builder.str() == "math")
            return 1;
        if(input[i] == ';')
        {
            commands.push_back(Command(builder.str(), SEMICOLON));
            stringstream().swap(builder);
        }
        else if(input[i] == '&')
        {
            if(input[i + 1] == '&')
            {
                commands.push_back(Command(builder.str(), DOUBLE_AND));
                i++;
                stringstream().swap(builder);
            }
            else
            {
                commands.push_back(Command("", -1));
                stringstream().swap(builder);
            }
        }
        else if(input[i] == '|')
        {
            if(input[i + 1] == '|')
            {
                commands.push_back(Command(builder.str(), DOUBLE_OR));
                i++;
                stringstream().swap(builder);
            }
            else
            {
                commands.push_back(Command(builder.str(), PIPE));
                stringstream().swap(builder);
            }
        }
        else
            builder << input[i];
    }
    commands.push_back(Command(builder.str(), SEMICOLON));
    return 0;
}

string getProgram(string input)
{
    stringstream output;
    bool commandStart = false;
    for(unsigned long i = 0; i < input.size(); i++)
    {
        if(input[i] == ' ' && !commandStart) // skip leading whitespace
            continue;
        else if(input[i] == ' ' && commandStart)
            break;
        else
        {
            commandStart = true;
            output << input[i];
        }
    }
    return output.str();
}

void removeEmptyElements(vector<string>& strings)
{
    for(unsigned long i = 0; i < strings.size(); i++)
    {
        if(strings[i].empty())
        {
            strings.erase(strings.begin() + static_cast<int>(i));
            i--;
        }
    }
}

vector<string> getArgs(string input)
{
    vector<string> args;
    stringstream builder;
    bool reachedFirstChar = false;
    bool inQuotes = false;
    for(unsigned long i = 0; i < input.length(); i++)
    {
        if(input[i] == '"' && !inQuotes) // open quote
        {
            inQuotes = true;
        }
        else if(input[i] == '"' && inQuotes) // close quote
        {
            args.push_back(builder.str());
            stringstream().swap(builder);
            inQuotes = false;
        }
        else if(input[i] == ' ' && !reachedFirstChar) // ignore leading whitespace
            continue;
        else if(input[i] == ' ' && !inQuotes) // split at whitespace
        {
            args.push_back(builder.str());
            stringstream().swap(builder);
        }
        else
        {
            reachedFirstChar = true;
            builder << input[i];
        }
    }
    if(inQuotes)
        return vector<string>();
    args.push_back(builder.str());
    removeEmptyElements(args);
    return args;
}

void expandTildeToHome(vector<string>& input)
{
    for(auto& arg : input)
    {
        for(size_t i = 0; i < arg.size(); i++)
        {
            if(arg[i] == '~')
            {
                arg.replace(i, 1, getenv("HOME"));
                break;
            }
        }
    }
}

vector<char*> getCharPtrArray(vector<string>& input)
{
    expandTildeToHome(input);
    vector<char*> result;
    result.reserve(input.size() + 1);
    transform(begin(input), end(input), 
        back_inserter(result), 
        [](string& s){ return s.data(); }
    );
    result.push_back(NULL);
    return result;
}