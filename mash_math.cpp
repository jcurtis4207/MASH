/*
* A simple calculator built for the MASH shell.
* The input comes from MASH as a single string of input.
* The input is parsed into tokens, then calculations are perfomed on those tokens.
* It has operators for addition, subtraction, multiplication, division, and exponents,
*   as well as negative numbers;
* It follows the order of precedence "PEMDAS", including parentheses.
* The values are stored as strings, and converted to doubles for the calculations.
*/

#include <algorithm>
#include <cctype>
#include <deque>
#include <iostream>
#include <math.h>
#include <set>
#include <sstream>
#include <vector>

using namespace std;

const set<string> ops = {"ADD", "SUB", "MUL", "DIV", "POW"};

vector<string> getTokens(string input)
{
    vector<string> tokens;
    stringstream builder;
    for(unsigned long i = 0; i < input.size(); i++)
    {
        if(input[i] == ' ')
            continue;
        else if(isdigit(input[i]) || input[i] == '.')
            builder << input[i];
        else
        {
            if(!builder.str().empty())
                tokens.push_back(builder.str());
            stringstream().swap(builder);
            if(input[i] == '(')
                tokens.push_back("OP");
            else if(input[i] == ')')
                tokens.push_back("CP");
            else if(input[i] == '+')
                tokens.push_back("ADD");
            else if(input[i] == '-')
                tokens.push_back("SUB");
            else if(input[i] == '*')
                tokens.push_back("MUL");
            else if(input[i] == '/')
                tokens.push_back("DIV");
            else if(input[i] == '^')
                tokens.push_back("POW");
            else
                return vector<string>();
        }
    }
    if(!builder.str().empty())
        tokens.push_back(builder.str());
    return tokens;
}

void dequeToVector(vector<string>& tokens, deque<string>& deck)
{
    tokens.clear();
    deque<string>::iterator it;
    for (it = deck.begin(); it != deck.end(); ++it)
        tokens.push_back(*it);
}

string addAndSubtract(vector<string>& tokens)
{    
    unsigned long i = 0;
    deque<string> deck;
    while(i < tokens.size())
    {
        if(tokens[i] == "ADD" || tokens[i] == "SUB")
        {
            double newValue;
            if(tokens[i] == "ADD")
                newValue = stod(deck.back()) + stod(tokens[i + 1]);
            else
                newValue = stod(deck.back()) - stod(tokens[i + 1]);
            deck.pop_back();
            ostringstream valueString;
            valueString << newValue;
            deck.push_back(valueString.str());
            i++;
        }
        else
            deck.push_back(tokens[i]);
        i++;
    }
    return deck.back();
}

void multiplyAndDivide(vector<string>& tokens)
{    
    unsigned long i = 0;
    deque<string> deck;
    while(i < tokens.size())
    {
        if(tokens[i] == "MUL" || tokens[i] == "DIV")
        {
            double newValue;
            if(tokens[i] == "MUL")
                newValue = stod(deck.back()) * stod(tokens[i + 1]);
            else
                newValue = stod(deck.back()) / stod(tokens[i + 1]);
            deck.pop_back();
            ostringstream valueString;
            valueString << newValue;
            deck.push_back(valueString.str());
            i++;
        }
        else
            deck.push_back(tokens[i]);
        i++;
    }
    dequeToVector(tokens, deck);
}

void exponent(vector<string>& tokens)
{
    unsigned long i = 0;
    deque<string> deck;
    while(i < tokens.size())
    {
        if(tokens[i] == "POW")
        {
            double newValue = pow(stod(deck.back()), stod(tokens[i + 1]));
            deck.pop_back();
            ostringstream valueString;
            valueString << newValue;
            deck.push_back(valueString.str());
            i++;
        }
        else
            deck.push_back(tokens[i]);
        i++;
    }
    dequeToVector(tokens, deck);
}

string performCalculations(vector<string>& tokens)
{
    exponent(tokens);
    multiplyAndDivide(tokens);
    return addAndSubtract(tokens);
}

void insideParentheses(vector<string>& tokens)
{
    unsigned long i = 0;
    deque<string> deck;
    while(i < tokens.size())
    {
        if(tokens[i] == "CP")
        {
            vector<string> insideTokens;
            while(deck.back() != "OP")
            {
                insideTokens.push_back(deck.back());
                deck.pop_back();
            }
            deck.pop_back(); // remove open parenthesis
            reverse(insideTokens.begin(), insideTokens.end()); // popped in reverse order
            deck.push_back(performCalculations(insideTokens));
        }
        else
            deck.push_back(tokens[i]);
        i++;
    }
    dequeToVector(tokens, deck);
}

int checkTokenErrors(vector<string>& tokens)
{
    int openPars = 0;
    int closePars = 0;
    for(unsigned long i = 0; i < tokens.size(); i++)
    {
        if(tokens[i] == "OP")
            openPars++;
        else if(tokens[i] == "CP")
            closePars++;
        else if(ops.find(tokens[i]) != ops.end()) // token is operator
        {
            if(i == tokens.size() - 1)
            {
                cerr << "ERROR: Operator has no operand\n";
                return 1;
            }
            else if(ops.find(tokens[i + 1]) != ops.end())
            {
                cerr << "ERROR: Consecutive operators\n";
                return 1;
            }
        }
        else if(tokens[i] == ".")
        {
            cerr << "ERROR: Operand must contain a digit\n";
            return 1;
        }
    }
    if(openPars != closePars)
    {
        cerr << "ERROR: Incorrect number of parentheses\n";
        return 1;
    }
    else
        return 0;
}

void applyNegation(vector<string>& tokens)
{
    for(unsigned long i = 0; i < tokens.size(); i++)
    {
        if(tokens[i] == "SUB")
        {
            // first token is subtraction, or previous token is operator -> negative sign
            if(i == 0 || ops.find(tokens[i - 1]) != ops.end())
            {
                tokens.erase(tokens.begin() + static_cast<int>(i));
                double value = stod(tokens[i]) * -1;
                ostringstream valueString;
                valueString << value;
                tokens[i] = valueString.str();
            }
        }
    }
}

void mathMode(string input)
{
    if(input.find("help") != std::string::npos)
    {
        cout << "USAGE: math [arithmetic expression]\n";
        cout << "   Operator Precedence: ( ) ^ * / + -\n";
        return;
    }
    vector<string> tokens = getTokens(input);
    if(tokens.empty())
    {
        cerr << "ERROR: Unknown operator\n";
        return;
    }
    applyNegation(tokens);
    if(checkTokenErrors(tokens) != 0)
    {
        return;
    }
    insideParentheses(tokens);
    cout << performCalculations(tokens) << "\n";
}