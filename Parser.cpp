/*
* This file contains all of functions necessary to parse user input
*/

#pragma once
#include <algorithm>
#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <vector>
#include "Aliases.cpp"
#include "History.cpp"

using namespace std;

#define SEMICOLON 0
#define DOUBLE_AND 1
#define DOUBLE_OR 2
#define PIPE 3

#define ESC 27
#define BACKSPACE 127
#define UP_ARROW 'A'
#define DOWN_ARROW 'B'
#define LEFT_ARROW 'D'
#define RIGHT_ARROW 'C'

extern map<string, string> aliases;

struct Command
{
    string program;
    int op = 0;
    Command(const string prog, const int oper)
        : program(prog), op(oper){}
};

char getch() {
    /*
    https://stackoverflow.com/questions/421860/capture-characters-from-standard-input-without-waiting-for-enter-to-be-pressed
    */
    char buf = 0;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
    struct termios old = {0};
#pragma GCC diagnostic pop
    if (tcgetattr(0, &old) < 0)
        perror("tcsetattr()");
    old.c_lflag &= static_cast<unsigned int>(~ICANON);
    old.c_lflag &= static_cast<unsigned int>(~ECHO);
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &old) < 0)
        perror("tcsetattr ICANON");
    if (read(STDIN_FILENO, &buf, 1) < 0)
        perror ("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &old) < 0)
        perror ("tcsetattr ~ICANON");
    return (buf);
}

void clearInput(string& input)
{
    while(!input.empty())
    {
        cout << "\b \b";
        input.pop_back();
    }
}

string parseArrowKeys(string& input)
{
    getch();
    char ch = getch();
    switch(ch)
    {
    case UP_ARROW:
        clearInput(input);
        return getOlderHistoryEntry();
        break;
    case DOWN_ARROW:
        clearInput(input);
        return getNewerHistoryEntry();
        break;
    default:
        string output = input;
        clearInput(input);
        return output;
        break;
    }
}

string getInput()
{
    string output;
    bool backslash = false;
    while(true)
    {
        char ch = getch();
        if(ch == ESC)
        {
            output = parseArrowKeys(output);
            cout << output;
            continue;
        }
        if(ch == BACKSPACE)
        {
            if(!output.empty())
            {
                cout << "\b \b";
                output.pop_back();
            }
            continue;
        }
        cout << ch;
        if(ch == '\n' && backslash)
        {
            cout << "> ";
            continue;
        }
        else if(ch == '\n' && !backslash)
            break;
        backslash = (ch == '\\') ? true : false;
        if(backslash)
            continue;
        output.append(1, ch);
    }
    return output;
}

int getCommands(vector<Command>& commands, string input)
{
    string program;
    for(unsigned long i = 0; i < input.length(); i++)
    {
        if(program == "math")
            return 1;
        if(input[i] == ';')
        {
            commands.push_back(Command(program, SEMICOLON));
            program.clear();
        }
        else if(input[i] == '&')
        {
            if(input[i + 1] == '&')
            {
                commands.push_back(Command(program, DOUBLE_AND));
                i++;
                program.clear();
            }
            else
            {
                commands.push_back(Command("", -1));
                program.clear();
            }
        }
        else if(input[i] == '|')
        {
            if(input[i + 1] == '|')
            {
                commands.push_back(Command(program, DOUBLE_OR));
                i++;
                program.clear();
            }
            else
            {
                commands.push_back(Command(program, PIPE));
                program.clear();
            }
        }
        else
            program.append(1, input[i]);
    }
    commands.push_back(Command(program, SEMICOLON));
    return 0;
}

string getProgram(string input)
{
    string output;
    bool commandStart = false;
    for(unsigned long i = 0; i < input.size(); i++)
    {
        if(input[i] == ' ' && !commandStart) // skip leading whitespace
            continue;
        else if(input[i] == ' ' && commandStart) // end on first space
            break;
        else
        {
            commandStart = true;
            output.append(1, input[i]);
        }
    }
    return output;
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
    string argument;
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
            args.push_back(argument);
            argument.clear();
            inQuotes = false;
        }
        else if(input[i] == ' ' && !reachedFirstChar) // ignore leading whitespace
            continue;
        else if(input[i] == ' ' && !inQuotes) // split at whitespace
        {
            args.push_back(argument);
            argument.clear();
        }
        else
        {
            reachedFirstChar = true;
            argument.append(1, input[i]);
        }
    }
    if(inQuotes)
        return vector<string>();
    args.push_back(argument);
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

string expandAlias(string input)
{
    string program;
    bool reachedFirstChar = false;
    unsigned long i;
    for(i = 0; i < input.size(); i++)
    {
        if(input[i] == ' ' && !reachedFirstChar)
            continue;
        else if(input[i] == ' ')
            break;
        else
        {
            reachedFirstChar = true;
            program.append(1, input[i]);
        }   
    }
    map<string, string>::iterator it = aliases.find(program);
    if(it == aliases.end())
        return input;
    string newCommand = it->second;
    return input.replace(0, i, newCommand);
}