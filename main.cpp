/* main.cpp */

#include <iostream>
#include "Sessionizing.hpp"

using namespace std;

int main()
{
    string inputCommand = "";
    
    cout << "Welcome to my Sessionizing solution" << endl;
    
    while(inputCommand != "init") {
        cout << "Please enter CSV file name or 'init' when done" << endl;
        cin >> inputCommand;
        Sessionizing::GetInstance()->handleInputCommand(inputCommand);
    }
    
    while(1) {
        cout << "Please enter data query" << endl;
        cin >> inputCommand;
        Sessionizing::GetInstance()->handleInputQuery(inputCommand);
    }

    return 0;
}
