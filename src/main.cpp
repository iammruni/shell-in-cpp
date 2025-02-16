#include <iostream>
#include <vector>
#include <sstream>

int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  //Builtin commands
  std::string builtincmds[2] = {"exit", "echo"};

  // Uncomment this block to pass the first stage
  while(1){
    std::cout << "$ ";

    std::string input;
    std::getline(std::cin, input);

    // Use stringstream to separate command and arguments
    std::stringstream ss(input);
    std::string command;
    std::vector<std::string> arguments;

    // Get the command (first word)
    ss >> command;

    // Get the arguments (remaining words)
    std::string arg;
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
        std::cout << a << " ";
      }
      std::cout << std::endl;
      continue;
    }

    //cmd: TYPE
    if (command == "type") {

      if (!arguments.empty()) {
        bool found = false;
        for (const auto& builtincmds: builtincmds) {
          if (arguments[0] == builtincmds) {
            std::cout << arguments[0] << " is a shell builtin" << std::endl;
            found = true;
            break;
          }
        }
        if (!found) {
          std::cout << arguments[0] << ": not found" << std::endl;
        }
      }
      continue;
    }

    std::cout << input << ": command not found" << std::endl;
  }
}
