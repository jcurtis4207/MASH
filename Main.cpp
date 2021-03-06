/*
* clang++ -std=c++17 -O2 mash.cpp -o mash 
*   with added warnings
*/

#include <iostream>
#include <vector>
#include "Aliases.cpp"
#include "Math.cpp"
#include "Parser.cpp"
#include "Execution.cpp"

using namespace std;

bool piping = false;
int errorLevel = 0;
int currentOperator = SEMICOLON;

void processCommands(vector<Command>& commands)
{
    piping = false;
    Command pipeStart = {"", SEMICOLON};
    for(auto& command : commands)
    {
        if(piping)
        {
            if(command.op == PIPE)
            {
                throwError(3);
                break;
            }
            else
            {
                errorLevel = initiatePipe(pipeStart, command);
                piping = false;
            }
        }
        else
        {
            if(command.op == PIPE)
            {
                pipeStart = command;
                piping = true;
                continue;
            }
            if(currentOperator == SEMICOLON)
                errorLevel = initiateCommand(command, false);
            else if(currentOperator == DOUBLE_OR && errorLevel != 0)
                errorLevel = initiateCommand(command, false);
            else if(currentOperator == DOUBLE_AND && errorLevel == 0)
                errorLevel = initiateCommand(command, false);
        }
        currentOperator = command.op;
    }
}

int main()
{
    if(setvbuf(stdout, NULL, _IONBF, 0) != 0)
    {
        cerr << "Stdout failed to be set unbuffered\n";
        exit(1);
    }
    seteuid(getuid());
    openAliasesFile();
    clearScreen();
    printFetch();
    setUpdatePrompt();
    vector<Command> commands;
    while(true)
    {
        showPrompt();
        string line = getInput();
        pushHistory(line);
        if(getCommands(commands, line) == 1)
        {
            mathMode(line.substr(4));
            continue;
        }
        processCommands(commands);
        commands.clear();
    }
    return errorLevel;
}