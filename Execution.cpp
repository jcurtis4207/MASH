/*
* This file contains all of the functions necessary to execute commands
*/

#pragma once
#include <iostream>
#include <map>
#include <pwd.h>
#include <sys/wait.h>
#include <unistd.h>
#include "Parser.cpp"

using namespace std;

bool updatePrompt;
string username;
string hostname;
string currentDir;
const map<int, string> errorCodes = {
    {1, "Unknown Operator"}, 
    {2, "Unknown Command"},
    {3, "Too Many Pipes"},
    {4, "No Closing Quotation"},
    {5, "File/Directory Not Found"},
    {6, "User Does Not Exist"}
};

void throwError(int errorLevel)
{
    cerr << "ERROR: " << errorCodes.at(errorLevel) << "\n";
}

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
    char uName[100], hName[100], cDir[100];
    cuserid(uName);
    gethostname(hName, sizeof(hName));
    getcwd(cDir, sizeof(cDir));
    username = uName;
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

int changeUser(char* arg)
{
    updatePrompt = true;
    uid_t new_euid;
    struct passwd* pwd = getpwnam(arg);
    if(pwd != NULL)
    {
        new_euid = pwd->pw_uid;
        seteuid(new_euid);
        return 0;
    }
    else
        return 1;
}

int executeBuiltins(const string& program, char* arg1)
{
    if(program == "exit")
        exit(0);
    else if(program == "cd")
    {
        if(chdir(arg1) != 0)
            return 5;
        updatePrompt = true;
        return 0;
    }
    else if(program == "clear")
    {
        clearScreen();
        return 0;
    }
    else if(program == "fetch")
    {
        printFetch();
        return 0;
    }
    else if(program == "su")
    {
        if(changeUser(arg1) != 0)
            return 6;
        return 0;
    }
    else
        return -1; // no builtins ran
}

int executeCommand(Command& command, bool pipe)
{
    if(command.op == -1)
    {
        throwError(1);
        return 1;
    }
    if(command.program == "")
        return 0;
    vector<string> stringArgs = getArgs(command.program);
    if(stringArgs.empty())
    {
        throwError(4);
        return 4;
    }
    const string program = getProgram(command.program);
    const char* cmd = program.c_str();
    vector<char*> args = getCharPtrArray(stringArgs);
    if(pipe)
    {
        if(execvp(cmd, args.data()) != 0)
        {
            throwError(2);
            return 2;
        }
        return 0;
    }
    int childExitStatus = 0;
    int builtinError = executeBuiltins(program, args[1]);
    if(builtinError > 0)
    {
        throwError(builtinError);
        return builtinError;
    }
    else if(builtinError < 0)
    {
        if(fork() != 0)
            wait(&childExitStatus);
        else
        {
            if(execvp(cmd, args.data()) != 0)
            {
                throwError(2);
                exit(2);
            }
        }
    }
    if(WIFEXITED(childExitStatus))
        return WEXITSTATUS(childExitStatus);
    else
        return -1;
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
        if(executeCommand(command1, true) != 0)
            exit(2);
    }
    if(fork() == 0)
    {
        dup2(fds[0], STDIN_FILENO);
        close(fds[0]);
        close(fds[1]);
        if(executeCommand(command2, true) != 0)
            exit(2);
    }
    close(fds[0]);
    close(fds[1]);
    wait(NULL);
    wait(NULL);
}