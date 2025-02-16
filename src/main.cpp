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

  int main() {
    // Flush after every cout / std:cerr
    cout << unitbuf;
    cerr << unitbuf;

    //Builtin commands
    string builtincmds[4] = {"exit", "echo", "type", "pwd"};
    
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
      stringstream ss(input);
      string command;
      vector<string> arguments;

      // Get the command (first word)
      ss >> command;

      // Get the arguments (remaining words)
      string arg;
      while (ss >> arg) {
        arguments.push_back(arg);
      }

      // cmd: EXIT
      if (command == "exit") {
        if (!arguments.empty() && arguments[0] == "0") {
          return 0;
        }
      }
      
      // cmd: ECHO
      if (command == "echo") {
        // Print each argument with a space between them
        for (const auto& a : arguments) {
          cout << a << " ";
        }
        cout << endl;
        continue;
      }

      //cmd: TYPE
      if (command == "type") {

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
        continue;
      }

       // cmd: PWD
       if (command == "pwd") {
        cout << filesystem::current_path().c_str() << endl;
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
