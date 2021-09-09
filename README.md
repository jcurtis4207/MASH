# MASH
## Welcome to the 4077
### The Majorly Awesome Shell

MASH is a simple linux-based shell written in C++.

It has a prompt showing the current user, hostname, and directory (with a colorscheme that matches my prompt).

It can accept multiple commands using semicolons, or conditional execution using && and ||.

If the input is not a built-in function, the input is executed with no error checking.

### Built-ins
The current built-in functions are:
* cd
* clear
* fetch (displaying an ASCII art rendition of MASH)
* exit

# 

### To-do
These are the features that will eventually be implemented:
* piping
* expanding '~' to the user's home directory
* dollar sign variables
* arithmetic
* scrollable history
* quotation marks
* backslash escaping newlines
