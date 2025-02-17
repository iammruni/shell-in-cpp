#include <iostream>
#include <vector>
#include <sstream>
#include <unistd.h>
#include <filesystem>
#include <sys/wait.h>
#include <ctype.h>
#include <fcntl.h>
using namespace std;

// s: string to be split; del: delimiter char
vector<string> tokeniser(string s, char del)
{
  stringstream ss(s);
  string word;
  vector<string> tokens;

  while (getline(ss, word, del))
  {
    tokens.push_back(word);
  }

  return tokens;
}

// Function to split string into arguments while handling quotes
vector<string> parseInputWithQuotes(const string &input)
{
  vector<string> arguments;
  string currentArg;
  bool insideSingleQuote = false; // Whether we're inside single quotes
  bool insideDoubleQuote = false; // Whether we're inside double quotes
  bool escapeNextChar = false;    // Flag to check for backslash escape

  for (size_t i = 0; i < input.size(); ++i)
  {
    char currentChar = input[i];

    if (escapeNextChar)
    {
      // If we're escaping the next character, add it literally
      currentArg.push_back(currentChar);
      escapeNextChar = false;
      continue;
    }

    // Handle the case for single quotes (preserve all characters literally inside single quotes)
    if (currentChar == '\'' && !insideDoubleQuote)
    {
      insideSingleQuote = !insideSingleQuote; // Toggle the inside single quote flag
      continue;                               // Skip the quote itself
    }

    // Handle the case for double quotes (process escape sequences inside double quotes)
    if (currentChar == '"' && !insideSingleQuote)
    {
      insideDoubleQuote = !insideDoubleQuote; // Toggle the inside double quote flag
      continue;                               // Skip the quote itself
    }

    // If inside single quotes, add the character literally (including backslashes and spaces)
    if (insideSingleQuote)
    {
      currentArg.push_back(currentChar);
    }
    // If inside double quotes, handle escape sequences
    else if (insideDoubleQuote)
    {
      if (currentChar == '\\')
      {
        char nxtchar = input[i + 1];
        char prvchar = input[i - 1];
        // If a backslash, escape the next character (including quotes and backslashes)
        bool is_digit = isdigit(nxtchar) || isdigit(prvchar);
        bool is_single_quote = (nxtchar == '\'' && prvchar == '\'');

        if (is_digit || nxtchar == 'n' || is_single_quote)
        {
          currentArg.push_back(currentChar);
        }
        else
        {
          escapeNextChar = true;
        }
      }
      else
      {
        currentArg.push_back(currentChar);
      }
    }
    // If outside quotes, handle backslashes followed by space as literal space
    else if (currentChar == '\\' && i + 1 < input.size() && (input[i + 1] == ' ' || input[i + 1] == '\'' || input[i + 1] == '"'))
    {
      // If a backslash followed by a space, add a literal space and skip the backslash
      currentArg.push_back(input[i + 1]);
      i++; // Skip the next character
    }
    // If not inside any quotes, handle the character normally
    else if (currentChar != ' ' && currentChar != '\\')
    {
      currentArg.push_back(currentChar);
    }

    // When encountering a space outside of quotes, push the argument to the vector
    if (!insideSingleQuote && !insideDoubleQuote && currentChar == ' ' && !currentArg.empty())
    {
      arguments.push_back(currentArg);
      currentArg.clear();
    }
  }

  // Add the last argument if it exists
  if (!currentArg.empty())
  {
    arguments.push_back(currentArg);
  }

  return arguments;
}

string parseforCMD(string &input)
{
  char startswithquote = input.at(0);
  string cmd = "";
  if (startswithquote == '\"' || startswithquote == '\'')
  {
    for (size_t i = 0; i < input.size(); i++)
    {
      char currentChar = input[i];
      if (currentChar != startswithquote)
      {
        cmd.push_back(currentChar);
      }
      if (currentChar == startswithquote && i != 0)
      {
        return cmd;
      }
    }
    return cmd;
  }
  return input;
}

// F_OK only checks for existence of file.
// access() returns 0 for file exists and -1 otherwise
bool isExecutable(const string &path)
{
  return access(path.c_str(), F_OK) == 0;
}

string findExecutable(const string &cmd, const vector<string> &paths)
{
  for (const auto &dir : paths)
  {
    // Form a full path from the command passed
    // Eg.: '/usr/bin' + '/' + 'ls'
    string fullpath = dir + "/" + cmd;
    if (isExecutable(fullpath))
    {
      return fullpath;
    }
  }
  return "";
}

// void handleCmdEcho(const vector<string> &args)
// {
//   // Print each argument with a space between them
//   for (const auto &a : args)
//   {
//     cout << a << " ";
//   }
//   cout << endl;
// }

void handleCmdType(const vector<string> &arguments, const vector<string> &PATH_DIRS, const vector<string> &builtincmds)
{
  if (!arguments.empty())
  {
    bool found = false;
    for (const auto &builtincmds : builtincmds)
    {
      if (arguments[0] == builtincmds)
      {
        cout << arguments[0] << " is a shell builtin" << endl;
        found = true;
        break;
      }
    }
    if (!found)
    {
      string checkpaths = findExecutable(arguments[0], PATH_DIRS);
      if (!checkpaths.empty())
      {
        cout << arguments[0] << " is " << checkpaths << endl;
      }
      else
      {
        cout << arguments[0] << ": not found" << endl;
      }
    }
  }
}

void handleCmdCD(vector<string> &arguments)
{
  if (!arguments.empty())
  {
    if (arguments[0][0] == '~')
    {
      arguments[0] = arguments[0].replace(0, 1, getenv("HOME"));
    }
    if (chdir(arguments[0].c_str()) == -1)
    {
      perror(("cd: " + arguments[0]).c_str());
    }
  }
}

int main()
{
  // Flush after every cout / std:cerr
  cout << unitbuf;
  cerr << unitbuf;

  // Builtin commands
  vector<string> builtincmds = {"exit", "echo", "type", "pwd", "cd"};

  // Handle PATH variable
  const string PATH_ENV_NAME = "PATH";
  string ENV_PATHS = getenv(PATH_ENV_NAME.c_str());
  vector<string> PATH_DIRS;

  PATH_DIRS = tokeniser(ENV_PATHS, ':');

  // Uncomment this block to pass the first stage
  while (1)
  {
    cout << "$ ";

    string input;
    getline(cin, input);

    // Use stringstream to separate command and arguments
    string command;

    // Parse the input with quoted arguments handled properly
    vector<string> arguments = parseInputWithQuotes(input);

    if (arguments.empty())
    {
      continue; // Empty input, continue the loop
    }

    command = arguments[0];
    arguments.erase(arguments.begin());
    // cmd: EXIT
    if (command == "exit")
    {
      if (!arguments.empty() && arguments[0] == "0")
      {
        return 0;
      }
    }

    // cmd: ECHO
    // if (command == "echo")
    // {
    //   handleCmdEcho(arguments);
    //   continue;
    // }

    // cmd: TYPE
    if (command == "type")
    {
      handleCmdType(arguments, PATH_DIRS, builtincmds);
      continue;
    }

    // cmd: PWD
    if (command == "pwd")
    {
      cout << filesystem::current_path().c_str() << endl;
      continue;
    }

    // cmd: CD
    if (command == "cd")
    {
      handleCmdCD(arguments);
      continue;
    }
    // cmd: executable file
    if (input.starts_with("'") || input.starts_with("\""))
    {
      command = parseforCMD(input);
    }
    string checkpaths = findExecutable(command, PATH_DIRS);
    if (!checkpaths.empty())
    {
      // Bring args outside; easier to debug
      vector<char *> execArgs;
      string redirect_to_file;
      string redirect_type;

      execArgs.push_back(const_cast<char *>(command.c_str()));
      for (size_t i = 0; i < arguments.size(); ++i) {
        if (arguments[i] == ">" || arguments[i] == "1>" || arguments[i] == "2>") {
            redirect_type = arguments[i];
            // Handle the redirection
            if (i + 1 < arguments.size()) {
                redirect_to_file = arguments[i + 1];  // Set the file to redirect to
                break;  // No need to process further arguments
            }
        } else {
            execArgs.push_back(const_cast<char *>(arguments[i].c_str()));
        }
    }
      execArgs.push_back(nullptr);
      // Save the original stdout file descriptor
      int saved_stdout = dup(STDOUT_FILENO);
      int saved_stderr = dup(STDERR_FILENO);
      if (saved_stdout == -1 || saved_stderr == -1) {
          perror("Unable to save stdout or stderr");
          return 1;
      }
      // Execute the command with execvp
      if (!redirect_to_file.empty()) {
        // open file descriptor
        int fd = open(redirect_to_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1) {
          perror("Unable to open file");
          return 1;
        }
        // redirect stdout or stderr
        if (redirect_type == "1>" || redirect_type == ">") {
          if (dup2(fd, STDOUT_FILENO) == -1) {
            perror("Unable to copy file descriptor stdout");
            return 1;
          }
        } else if (redirect_type == "2>") {
          if (dup2(fd, STDERR_FILENO) == -1) {
            perror("Unable to copy file descriptor stderr");
            return 1;
          }
        }
        
        close(fd);
      }
      
      // Fork to create a new process
      pid_t pid = fork();
      if (pid == -1)
      {
        // Fork failed, print an error
        perror("fork failed");
        continue;
      }
      if (pid == 0)
      {
        execvp(command.c_str(), execArgs.data());
        // If execvp fails, print an error
        perror("execvp failed");
        return 1;
      }
      else
      {
        // In the parent process, wait for the child to finish
        waitpid(pid, nullptr, 0);
        if (dup2(saved_stdout, STDOUT_FILENO) == -1 || dup2(saved_stderr, STDERR_FILENO) == -1) {
          perror("Unable to restore stdout or stderr");
          return 1;
        }
        close(saved_stdout);
        close(saved_stderr);
      
      }
      continue;
    }

    cout << input << ": command not found" << endl;
  }
}
