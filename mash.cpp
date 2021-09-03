/*
* clang++ -std=c++17 -O2 mash.cpp -o mash 
*   with added warnings
*/

#include <iostream>
#include <sstream>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

using namespace std;

string username;
string hostname;
string currentDir;
bool update;

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
    update = false;
}

void showPrompt()
{
    if(update)
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

string getProgram(string input)
{
    return input.substr(0, input.find(" "));
}

vector<string> getArgs(string input)
{
    vector<string> args;
    stringstream inputStream(input);
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
        update = true;
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

    // su - update = true
}

int main()
{
    clearScreen();
    printFetch();
    update = true;
    while(true)
    {
        showPrompt();
        // read command
        string line;
        getline(cin, line);
        string program = getProgram(line);
        vector<string> stringArgs = getArgs(line);
        // convert strings to char arrays
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
                execvp(cmd, args);
        }
        // clean up
        for(unsigned long i = 0; i < argSize; i++)
            delete [] args[i];
            
    }
    return 0;
}
