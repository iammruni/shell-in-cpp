  #include <iostream>
  #include <vector>
  #include <sstream>
  #include <unistd.h>
  #include <filesystem>
  using namespace std;

  // s: string to be split; del: delimiter char
  vector<string> tokeniser(string s, char del) {
    stringstream ss(s);
    string word;
    vector<string> tokens;

    while (getline(ss, word, del)) {
      tokens.push_back(word);
    }

    return tokens;
  }

  // Function to split string into arguments while handling quotes
  vector<string> parseInputWithQuotes(const string& input) {
    vector<string> arguments;
    string currentArg;
    bool insideQuote = false;
    char quoteChar = '\0';
    bool escapeNextChar = false; // Flag to check for backslash escape

    for (size_t i = 0; i < input.size(); ++i) {
        char currentChar = input[i];

        // Handle backslash escape
        if (escapeNextChar) {
            currentArg.push_back(currentChar);
            escapeNextChar = false; // Reset escape flag
            continue;
        }

        // Handle backslash (escape character)
        if (currentChar == '\\') {
            escapeNextChar = true; // Set flag to escape next character
            continue;
        }

        // Toggle inside quote state on encountering a quote mark
        if ((currentChar == '"' || currentChar == '\'') && !insideQuote) {
            insideQuote = true;
            quoteChar = currentChar; // Set the type of quote encountered
            continue; // Skip the quote
        } else if (currentChar == quoteChar && insideQuote) {
            insideQuote = false; // Close the quote
            continue; // Skip the closing quote
        }

        // If inside a quote, append characters to current argument
        if (insideQuote || currentChar != ' ') {
            currentArg.push_back(currentChar);
        }

        // When encountering a space outside of quotes, push the argument to the vector
        if (!insideQuote && currentChar == ' ' && !currentArg.empty()) {
            arguments.push_back(currentArg);
            currentArg.clear();
        }
    }

    // Add the last argument if it exists
    if (!currentArg.empty()) {
        arguments.push_back(currentArg);
    }

    return arguments;
}

  // F_OK only checks for existence of file.
  // access() returns 0 for file exists and -1 otherwise
  bool isExecutable(const string& path) {
    return access(path.c_str(), F_OK) == 0;
  }

  string findExecutable(const string& cmd, const vector<string>& paths) {
    for (const auto& dir: paths){
      // Form a full path from the command passed
      // Eg.: '/usr/bin' + '/' + 'ls'
      string fullpath = dir + "/" + cmd;
      if (isExecutable(fullpath)) {
        return fullpath;
      }
    }
    return "";
  }

  void handleCmdEcho (const vector<string>& args) {
    // Print each argument with a space between them
    for (const auto& a : args) {
      cout << a << " ";
    }
    cout << endl;
  }

  void handleCmdType (const vector<string>& arguments, const vector<string>& PATH_DIRS, const vector<string>& builtincmds) {
    if (!arguments.empty()) {
      bool found = false;
      for (const auto& builtincmds: builtincmds) {
        if (arguments[0] == builtincmds) {
          cout << arguments[0] << " is a shell builtin" << endl;
          found = true;
          break;
        }
      }
      if (!found) {
        string checkpaths = findExecutable(arguments[0], PATH_DIRS);
        if (!checkpaths.empty()){
          cout << arguments[0] << " is " << checkpaths << endl;
        } else {
          cout << arguments[0] << ": not found" << endl;
        }
      }
    }
  }

  void handleCmdCD (vector<string>& arguments) {
    if (!arguments.empty()) {
      if (arguments[0][0] == '~') {
        arguments[0] = arguments[0].replace(0, 1, getenv("HOME"));
      }
      if (chdir(arguments[0].c_str()) == -1) {
        perror(("cd: " + arguments[0]).c_str());
      }
    }
  }

  int main() {
    // Flush after every cout / std:cerr
    cout << unitbuf;
    cerr << unitbuf;

    //Builtin commands
    vector<string> builtincmds = {"exit", "echo", "type", "pwd", "cd"};
    
    // Handle PATH variable
    const string PATH_ENV_NAME = "PATH";
    string ENV_PATHS = getenv(PATH_ENV_NAME.c_str());
    vector<string> PATH_DIRS;

    PATH_DIRS = tokeniser(ENV_PATHS, ':');

    // Uncomment this block to pass the first stage
    while(1){
      cout << "$ ";

      string input;
      getline(cin, input);

      // Use stringstream to separate command and arguments
      string command;

      // Parse the input with quoted arguments handled properly
      vector<string> arguments = parseInputWithQuotes(input);

      if (arguments.empty()) {
          continue; // Empty input, continue the loop
      }

      command = arguments[0];
      arguments.erase(arguments.begin());
      // cmd: EXIT
      if (command == "exit") {
        if (!arguments.empty() && arguments[0] == "0") {
          return 0;
        }
      }
      
      // cmd: ECHO
      if (command == "echo") {
        handleCmdEcho(arguments);
        continue;
      }

      //cmd: TYPE
      if (command == "type") {
        handleCmdType(arguments, PATH_DIRS, builtincmds);
        continue;
      }

       // cmd: PWD
       if (command == "pwd") {
        cout << filesystem::current_path().c_str() << endl;
        continue;
      }

      //cmd: CD
      if (command == "cd") {
        handleCmdCD(arguments);
        continue;
      }
      // cmd: executable file
      string checkpaths = findExecutable(command, PATH_DIRS);
      if (!checkpaths.empty()) {
        system(input.c_str());
        continue;
      }

      cout << input << ": command not found" << endl;
    }
  }
