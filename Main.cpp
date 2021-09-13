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

extern bool updatePrompt;

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
    updatePrompt = true;
    int errorLevel = 0;
    int currentOperator = SEMICOLON;
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
        Command pipeStart = {"", SEMICOLON};
        bool piping = false;
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
                    executePipe(pipeStart, command);
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
                    errorLevel = executeCommand(command, false);
                else if(currentOperator == DOUBLE_OR && errorLevel != 0)
                    errorLevel = executeCommand(command, false);
                else if(currentOperator == DOUBLE_AND && errorLevel == 0)
                    errorLevel = executeCommand(command, false);
            }
            currentOperator = command.op;
        }
        commands.clear();
    }
    return errorLevel;
}