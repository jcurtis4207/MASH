# MASH
## Welcome to the 4077
### The Majorly Awesome Shell

MASH is a simple linux-based shell written in C++.

The prompt shows the current user, hostname, and directory (with a colorscheme that matches my prompt).

It can accept multiple commands using semicolons, or conditional execution using && and ||.

In addition, non-consecutive piping is allowed.

A line ending in a backslash will allow for multiple lines of input.

If the input is not a built-in function, the input is executed with basic error checking for unknown commands and operators.

The up and down arrow keys cycle through the command history, which has a hardcoded size of 10.

### Built-ins
The current built-in functions are:
* cd
* clear
* fetch (displaying an ASCII art rendition of MASH)
* exit
* su
* math (explained below)
* ~ expands to the user's home directory

### Math Mode
The command 'math' allows for inline arithmetic calculations. 
There are operators for exponents, multiplication, division, addition, and subtraction.
Parentheses are also allowed.
The order of operations follows PEMDAS.
It accepts positive and negative numbers, integers and decimal numbers, and whitespace is ignored.
Also, entering the word 'help' after the 'math' command displays the usage.
An example of math mode looks like this:

```
math 5 / (2 + 2)
``` 
And the output is simply:
```
1.25
```

# 

### To-do
These are the features that will eventually be implemented:
* dollar sign variables
* backslashes as escape characters
