#include <iostream>
#include <vector>
#include <sstream>
using namespace std;

vector<string> tokeniser() {

}


int main() {
  // Flush after every cout / std:cerr
  cout << unitbuf;
  cerr << unitbuf;

  //Builtin commands
  string builtincmds[3] = {"exit", "echo", "type"};
  
  // Handle PATH variable
  const string PATH_ENV_NAME = "PATH";
  string ENV_PATHS = getenv(PATH_ENV_NAME.c_str());
  vector<string> PATH_DIRS;




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
          cout << arguments[0] << ": not found" << endl;
        }
      }
      continue;
    }

    cout << input << ": command not found" << endl;
  }
}
