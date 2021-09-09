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
    int op = 0;
    int error = 0;
    Command(const string prog, const int oper)
        : program(prog), op(oper){}
    Command(const int el)
        : error(el){}
};

string username;
string hostname;
string currentDir;
bool updatePrompt;
const map<int, string> errors = {
    {1, "Unknown Operator"}, 
    {2, "Unknown Command"},
    {3, "Too Many Pipes"}
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

void throwError(int errorLevel)
{
    cerr << "ERROR: " << errors.at(errorLevel) << "\n";
}

void getCommands(vector<Command>& commands, string input)
{
    stringstream builder;
    for(unsigned long i = 0; i < input.length(); i++)
    {
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
                commands.push_back(Command(1));
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

vector<char*> getCharPtrArray(vector<string>& input)
{
    vector<char*> result;
    result.reserve(input.size() + 1);
    transform(begin(input), end(input), 
        back_inserter(result), 
        [](string& s){ return s.data(); }
    );
    result.push_back(NULL);
    return result;
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

void executeCommand(Command& command, bool pipe)
{
    if(command.error != 0)
    {
        throwError(command.error);
        return;
    }
    vector<string> stringArgs = getArgs(command.program);
    const string program = getProgram(command.program);
    const char* cmd = program.c_str();
    vector<char*> args = getCharPtrArray(stringArgs);
    // execute command in pipe
    if(pipe)
    {
        if(execvp(cmd, args.data()) != 0)
        {
            command.error = 2;
            throwError(command.error);
        }
        return;
    }
    // execute command
    else if(executeBuiltins(program, args[1]))
    {
        if(fork() != 0)
            wait(NULL);
        else
        {
            if(execvp(cmd, args.data()) != 0)
            {
                command.error = 2;
                throwError(command.error);
            }
        }
    }
}

void executePipe(Command& command1, Command& command2)
{
    int fds[2];
    pipe(fds);
    if(fork() == 0)
    {
        dup2(fds[1], STDOUT_FILENO);
        close(fds[0]);
        close(fds[1]);
        executeCommand(command1, true);
    }
    if(fork() == 0)
    {
        dup2(fds[0], STDIN_FILENO);
        close(fds[0]);
        close(fds[1]);
        executeCommand(command2, true);
    }
    close(fds[0]);
    close(fds[1]);
    wait(NULL);
    wait(NULL);
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
        Command pipeIn = {0};
        bool piping = false;
        for(auto& command : commands)
        {
            if(command.op == PIPE)
            {
                pipeIn = command;
                piping = true;
                continue;
            }
            if(piping)
            {
                executePipe(pipeIn, command);
                piping = false;
            }
            else
            {
                if(command.op == PIPE)
                    throwError(3);
                else if(currentOperator == SEMICOLON)
                    executeCommand(command, false);
                else if(currentOperator == DOUBLE_OR && errorLevel != 0)
                    executeCommand(command, false);
                else if(currentOperator == DOUBLE_AND && errorLevel == 0)
                    executeCommand(command, false);
            }
            currentOperator = command.op;
            errorLevel = command.error;
        }
      commands.clear();
    }
    return errorLevel;
}