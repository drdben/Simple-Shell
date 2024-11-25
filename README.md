# Simple-Shell
## Functionality
- Basically: A simple shell built using fork() and execvp().
- Command History: Maintains a global array to store executed commands, including their PID and execution time.
- Signal Handling: Custom handler for SIGINT (Ctrl+C) to display process details (history) before exiting.
- Piping Support: Implements pipes (|) to chain commands, using dup2() and an array of pipes.
- Shell Scripts: Executes commands from .sh files.

## Limitations
1. cd Command:
   cd alters the parent process directory, but fork() creates a child process. Hence, changes in the child don't reflect in the parent.
2. ls ~ and Shell-Specific Expansions:
   Expansions like ~ or $HOME are not recognized since they are interpreted by the shell, not the exec family.
3. Environment Variables:
   Variables like $PATH or custom functions within the shell are unsupported outside this implementation.
4. Command Length:
   Commands are limited to 1000 characters due to code constraints.

## Implementation Details
### Shell Loop:
- Handles user input, command parsing, and command execution.
- Exits on status failure (1) from the launch function.
### Command Parsing:
Commands are split by:
- Pipes (|) for piping functionality.
- Spaces ( ) for individual command arguments.
### Command Execution:
Executed using fork() and execvp().
For pipes:
- pipe() creates an array for each command.
- dup2() redirects stdin and stdout to enable communication between commands.
History:
- Stores executed commands and details like PID and execution time in global arrays.
- Accessible at any time during shell usage.

Error Handling: Checked at every stage, especially for execvp() failures.
SIGINT Handler: Displays command history and exits gracefully when Ctrl+C is pressed.
## Contributions
- Divyanshi, 2023209
- Arnav Gupta, 2023125
