# Simple Command-Line Interpreter
## About
This program implements a simple shell program. A shell is a command-line interpreter: it accepts input from the user under the form of command lines and executes them. Well-known UNIX shells include for example `bash` (default shell on Ubuntu) and `zsh` (default shell on MacOS).
## Features
This simple shell program implements the following set of core features of typical shells:
1. Execution of user-supplied commands with optional arguments
2. A selection of typical builtin commands
3. Redirection of the standard output of commands to files
4. Composition of commands via piping 
5. Simple environment variables
## Commands and Specifications
### 1. Commands
This simple shell program supports the use of basic Linux/Unix operating system commands (e.g. `ls`, `cat`, `echo`). Here is a reference list of Unix commands: https://en.wikipedia.org/wiki/List_of_Unix_commands.

Limitations:
-   The maximum length of a command line never exceeds 512 characters.
-   A program has a maximum of 16 arguments.
-   The maximum length of individual tokens never exceeds 32 characters.

Example:
```
sshell@ucd$ echo Hello world
Hello world
+ completed 'echo Hello world' [0]
sshell@ucd$
```
### 2. Builtin commands
The commands `exit`, `cd`, and `pwd` are manually implemented in contrast to the regular commands previously mentioned that are ran externally. These builtin commands still function similar to how they are expected.

`set` is another builtin command used to set the value of a simple environment variable. Check the section "**5. Simple environment variables**" below for more information.

Example:
```
(base) chads-air:simple-shell chadhoang$ ./sshell
sshell@ucd$ exit
Bye...
+ completed 'exit' [0]
(base) chads-air:simple-shell chadhoang$
```

Example:
```
sshell@ucd$ pwd
/Users/chadhoang/Downloads/simple-shell
+ completed 'pwd' [0]
sshell@ucd$ cd ..
+ completed 'cd ..' [0]
sshell@ucd$ pwd
/Users/chadhoang/Downloads
+ completed 'pwd' [0]
sshell@ucd$
```
### 3. Output redirection
The standard output redirection is indicated by using the meta-character `>` followed by a file name. Such redirection implies that the command located right before `>` is to write its output to the specified file instead of the shell’s standard output (that is on the screen if the shell is run in a terminal).

Example:
```
sshell@ucd$ echo Hello world>file
+ completed 'echo Hello world>file' [0]
sshell@ucd$ cat file
Hello world
+ completed 'cat file' [0]
sshell@ucd$
```
### 4. Piping
The pipe sign is indicated by using the meta-character `|` and allows multiple commands to be connected to each other within the same command line. When the shell encounters a pipe sign, it indicates that the output of the command located before the pipe sign must be connected to the input of the command located after the pipe sign.

Limitations:
- Maximum number of pipe signs that can be included on the same command line to connect multiple commands to each other is 3.
- **Builtin commands** cannot be called as part of a pipeline.

Example:
```
sshell@ucd$ echo Hello world | grep Hello|wc -l
1
+ completed 'echo Hello world | grep Hello|wc -l' [0][0][0]
sshell@ucd$
```
### 5. Simple environment variables 
This feature introduces string variables, which can be used as part of a command. There can only be 26 variables, named `a` to `z`.

Variables are set using the builtin command `set`. The variable is unset if no string value is provided. A variable is replaced by the empty string `""` after being unset.

For a string variable to be used in a command, the variable must be prefixed by the special symbol `$`. The variable will be replaced by its equivalent string value upon use. 

Example:
```
sshell@ucd$ echo $p

+ completed 'echo $p' [0]
sshell@ucd$ set l ecs150
+ completed 'set l ecs150' [0]
sshell@ucd$ echo $p $l
 ecs150
+ completed 'echo $p $l' [0]
sshell@ucd$ set j echo
+ completed 'set j echo' [0]
sshell@ucd$ $j $p $l
 ecs150
+ completed '$j $p $l' [0]
sshell@ucd$
```
## How to use
This program requires GCC to be installed into your computer (link: https://gcc.gnu.org/install/). To run the program, enter the `simple-shell` directory in your computer's terminal, then enter `make` onto the command line to compile the files into an executable.
Example:
```
(base) chads-air:simple-shell-main chadhoang$ make
```
Afterwards, run the `sshell` executable to run the program.
Example:
```
(base) chads-air:simple-shell-main chadhoang$ ./sshell
sshell@ucd$
```
## Lessons Learned
This project taught me a lot about how shell programs work in great detail. I was introduced to the topic of processes in operating systems, such as how they are configured and launched, along with other operating system topics such as pipes and file manipulation. I learned how to work with the GNU C Library in order to implement the features of the shell. My familiarity with taking in command line arguments, and with the C language in general has also been greatly strengthened after working with pointers, struct data structures, and other parts of the syntax in order to complete this project.
