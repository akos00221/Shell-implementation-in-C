# Shell implementation in C
*A non-portable POSIX dependent shell implementation written in C.*

## Goal:
The main goals include:
- Deepen my knowledge of C
- Gain an understanding of how a shell works
- Have a large enough codebase or complex enough problem where I can apply a non-builtin data structure.

## Capabilities:
- **Command Parsing (AST):** Parses user input into an Abstract Syntax Tree for structured execution (`myshell_ast_tree`).
- **Job Control & Concurrency:** Supports background execution and job tracking (`myshell_jobs`).
- **I/O Redirection:** Handles standard input/output redirection (`myshell_redir`).
- **Pipe (|) control operator:**  Supports piping the output of one executable/command into another (`myshell_ctrl_ops`).
- **Command History:** Tracks previously executed commands (`myshell_history`).
- **Variable Management:** Allows declaring and managing variables (`myshell_declare`).
- **Shell Built-ins:** Includes standard shell built-in commands (`myshell_shell_builtin`). (e.g. cd, pwd, type, command completion)
- **Readline Utility:** Enhances input handling for command typing (`myshell_readline_util`)
## Prerequisites:
To compile and run this shell you will need:
- A POSIX-compliant operating system (Linux, MacOS, etc.)
- GCC or Clang compiler
- the `readline.h` library:

**Ubuntu/Debian**
> sudo apt update && sudo apt install libreadline-dev

**Fedora**
> sudo dnf install readline-devel

**Arch Linux**
> sudo pacman -Syu readline

**MacOS**
> brew install readline

*Note: The `-lreadline` flag is needed for compilation and on MacOS systems gcc/Clang does not automatically find the default install location.*

## Installation & Build:
Clone the repository and compile the source code using `gcc`:

**Clone the repository**
> git clone https://github.com/akos00221/Shell-implementation-in-C  
> cd Shell-implementation-in-C

**Compile the source files**:  
> gcc -Iinclude -g src/*.c -o myshell -lreadline

**Execute the program**:
> ./myshell

## Project insights:
Here I will go over in what capacity I managed to achieve the aforementioned goals.
- **Deepen my knowledge of C**: C is infamous for its poor string handling out of the box and this wasn't any different in my project. Input parsing (`myshell_input_parse`) had to be handled early on and implementing the different input states was the first bigger roadblock I encountered. I decided to use a **state machine** to switch between the parsing modes simply because I had experience with them from before. I also implemented a lookup table (`char_lookup[]`) for the special characters which are escaped depending on the parsing state (the int values are needed because the ones with values of 2 are the only ones escaped if within double quotes). The array size also allows for additional escapable characters to be implemented if the user desires.  

    C is also famous for allowing the programmer a lot of control over memory. I made a point of making sure that my code does not leak memory and also to try and minimize trips to the memory (essentially mallocs) as these are the biggest bottlenecks of a program. In the first I succeeded as running the program through valgrind reveals leaks only from the `readline.h` library. In the second I partially succeeded. Early on I made the decision to `strdup()` each tokenized input from `char *input` and store it in `char **input_tok`. This was technical debt I realised only later on but I did not make the same mistake in my `create_ast_tree` where I only stored the pointers to the tokens.   
    *note: I will expand on how I would have done things differently in a later section.*
- **Gain an understanding of how a shell works**: This is a passive side effect of completing the project. Before shells were a black box to me but now I can fully appreciate just how powerful they can be if someone knows the basics of its features. The most challenging parts were the system calls (like `dup()`, `dup2()`, `fork()`) and implementing concurrency (`jobs()`, `piping()`).
- **Have a large enough codebase or complex enough problem where I can apply a non-builtin data structure**: I came up with the vague idea of using an [Abstract Syntax Tree](https://en.wikipedia.org/wiki/Abstract_syntax_tree) on my own and after some searching I found the official name for it. When I was implementing the redirection functionality (`myshell_redir`) I had to allow for the redirection operator(s) (>, >>, 1>> etc.) to appear in various positions relative to the source and destination. This was a problem as before this all commands or executables had their parameters in a fixed position and could be easily fetched using simple indexes. But here both `"echo A > a.txt"` and `"> a.txt echo A"` would be correct.  
So at this point I knew shells use different symbols (control operators) to control the flow of data and set precedence (in order of execution). Naturally I wanted to break up the input string along these operators. I also knew this had to be recursive, as shells allow chaining and even multiple standalone inputs separated by ";". The idea for the tree structure came from the redirection operator's structure which only guarantees that to its right will be the filename (which would become the right leaf here). From here the leap to the AST tree wasn't that big. Implementing it was although the most fun I had it also came with a huge restructuring of the code I had implemented that far.
### What I would do differently:
- **Avoid using `\0` terminated strings**: Storing strings in a simple structure like below would have saved a lot of `strlen()` checks and string manipulation would have been easier.
    ```C
    typedef struct {
        size_t len, size;
        char *data;
    }string;
    ```  
- **Don't `strdup()` in `create_token()` instead store the references**: As alluded to earlier originally I wanted to minimize memory trips and `strdup()` calls `malloc()` in the background which although is convenient and makes things simpler it also causes unnecessary slowdown. The members of `**input_tok` would be simple references to the original input string (which would be altered in place) if I had to start over.
- **Map out all functionalities and plan in account of them**: When I started this project I learned all shell functionalities along the way and the code reflected this. To be fair it was fine until about the halfway point (the redirection) but then I had to rethink the whole execution and parsing logic. If I had to start over I would try to gain a near complete picture of what a shell is and how it roughly works.
- **Write informative comments**: Not really a major issue that came up as the code size is still pretty small sometimes I had to rediscover concepts after not touching a project for some time.

## AI usage:
As this is a learning project I tried to minimze using AI. Here is a list of where it was used:
- It wrote 2 functions: `print_ast` and `print_ast_helper`. These were quick debug functions they don't contribute to the overall logic of the project.
- When I googled the AI summary inevitably came up and at times (in simple questions) I only needed the short version.
- Grammar and structure check for this document.
    
