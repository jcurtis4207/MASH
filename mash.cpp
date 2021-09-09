/*
* clang++ -std=c++17 -O2 mash.cpp -o mash 
*   with added warnings
*/

#include <iostream>
#include <map>
#include <regex>
#include <sstream>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

using namespace std;

#define SEMICOLON 0
#define DOUBLE_AND 1
#define DOUBLE_OR 2
#define PIPE 3

struct Command
{
    string program;
    vector<string> stringArgs;
    int op = 0;
    int error = 0;
    Command(const string prog, vector<string> args, const int oper)
        : program(prog), stringArgs(args), op(oper){}
    Command(const int el)
        : error(el){}
};

string username;
string hostname;
string currentDir;
bool updatePrompt;
const map<int, string> errors = {
    {1, "Unknown Operator"}, 
    {2, "Unknown Command"}
};

void clearScreen()
{
    cout << "\033[2J"; // clear screen
    cout << "\033[1;1H"; // move cursor to beginning
}

void printFetch()
{
    cout << ".___  ___.     _        ___         _         _______.    _     __    __  \n";
    cout << "|   \\/   |  /\\| |/\\    /   \\     /\\| |/\\     /       | /\\| |/\\ |  |  |  | \n";
    cout << "|  \\  /  |  \\ ' ' /   /  ^  \\    \\ ' ' /    |   (----' \\ ' ' / |  |__|  | \n";
    cout << "|  |\\/|  | |_     _| /  /_\\  \\  |_     _|    \\   \\    |_     _||   __   | \n";
    cout << "|  |  |  |  / . . \\ /  _____  \\  / . . \\ .----)   |    / . . \\ |  |  |  | \n";
    cout << "|__|  |__|  \\/|_|\\//__/     \\__\\ \\/|_|\\/ |_______/     \\/|_|\\/ |__|  |__| \n";
}

void setGlobalParameters()
{
    char hName[100], cDir[100];
    gethostname(hName, sizeof(hName));
    getcwd(cDir, sizeof(cDir));
    username = getlogin();
    hostname = hName;
    currentDir = cDir;
    updatePrompt = false;
}

void showPrompt()
{
    if(updatePrompt)
        setGlobalParameters();
    stringstream prompt;
    prompt << "\033[1;31m" << "M*A*S*H "; // red
    prompt << "\033[1;32m" << username << "@" << hostname; // green
    prompt << "\033[1;37m" << ":"; // white
    prompt << "\033[1;34m" << currentDir; // blue
    prompt << "\033[1;37m" << "$ "; // white
    prompt << "\033[0;37m"; // normal
    cout << prompt.str();
}

void throwError(const string& message)
{
    cout << "ERROR: " << message << "\n";
}

void getCommands(vector<Command>& commands, string input)
{
    stringstream builder;
    for(unsigned long i = 0; i < input.length(); i++)
    {
        if(input[i] == ';')
        {
            commands.push_back(Command(builder.str(), vector<string>(), SEMICOLON));
            stringstream().swap(builder);
        }
        else if(input[i] == '&')
        {
            if(input[i + 1] == '&')
            {
                commands.push_back(Command(builder.str(), vector<string>(), DOUBLE_AND));
                i++;
                stringstream().swap(builder);
            }
            else
            {
                commands.push_back(Command(1));
                stringstream().swap(builder);
            }
        }
        else if(input[i] == '|')
        {
            if(input[i + 1] == '|')
            {
                commands.push_back(Command(builder.str(), vector<string>{}, DOUBLE_OR));
                i++;
            stringstream().swap(builder);
            }
            else
            {
                commands.push_back(Command(builder.str(), vector<string>(), PIPE));
                stringstream().swap(builder);
            }
        }
        else
            builder << input[i];
    }
    commands.push_back(Command(builder.str(), vector<string>(), SEMICOLON));
}

string getProgram(string input)
{
    input = regex_replace(input, regex("^ +"), "");
    return input.substr(0, input.find(" "));
}

vector<string> getArgs(string input)
{
    vector<string> args;
    stringstream inputStream(regex_replace(input, regex("^ +"), ""));
    string temp;    
    while (getline(inputStream, temp, ' '))
        args.push_back(temp);
    return args;
}

bool executeBuiltins(const string& program, char* arg1)
{
    if(program == "exit")
        exit(0);
    else if(program == "cd")
    {
        chdir(arg1);
        updatePrompt = true;
        return false;
    }
    else if(program == "clear")
    {
        clearScreen();
        return false;
    }
    else if(program == "fetch")
    {
        printFetch();
        return false;
    }
    else
        return true;

    // su - updatePrompt = true
}

void executeCommand(Command& command)
{
    const vector<string> stringArgs = getArgs(command.program);
    const string program = getProgram(command.program);
    if(command.error != 0)
    {
        throwError(errors.at(command.error));
        return;
    }
    // convert strings to char*
    const unsigned long argSize = stringArgs.size();
    const char* cmd = program.c_str();
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wvla-extension"
    char* args[argSize + 1];
#pragma GCC diagnostic pop
    for(unsigned long i = 0; i < argSize; i++)
    {
        args[i] = new char[stringArgs[i].size() + 1];
        strcpy(args[i], stringArgs[i].c_str());
    }
    args[argSize] = NULL;
    // execute command
    if(executeBuiltins(program, args[1]))
    {
        if(fork() != 0)
            wait(NULL);
        else
        {
            if(execvp(cmd, args) != 0)
            {
                command.error = 2;
                throwError(errors.at(command.error));
            }
        }
    }
    // clean up pointers
    for(unsigned long i = 0; i < argSize; i++)
        delete [] args[i];
}

int main()
{
    vector<Command> commands;
    clearScreen();
    printFetch();
    updatePrompt = true;
    int errorLevel = 0;
    int currentOperator = SEMICOLON;
    while(true)
    {
        showPrompt();
        string line;
        getline(cin, line);
        getCommands(commands, line);
        for(auto& command : commands)
        {
            // operator and error level determines execution
            if(currentOperator == SEMICOLON)
                executeCommand(command);
            else if(currentOperator == DOUBLE_OR && errorLevel != 0)
                executeCommand(command);
            else if(currentOperator == DOUBLE_AND && errorLevel == 0)
                executeCommand(command);
            currentOperator = command.op;
            errorLevel = command.error;
        }
      commands.clear();
    }
    return errorLevel;
}