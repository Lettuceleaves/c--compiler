#include <iostream>
#include <stack>
#include <vector>
#include <string>
#include <algorithm>
#include <fstream>
using namespace std;

bool mode_asm = false;
bool mode_code = false;

int main(int argc, char* argv[]) {

    // input check print

    for (int i = 0; i < argc; i++) {
        cout << "i: " << i << " argv: " << argv[i] << endl;
    }
    cout << endl;

    // save args in c++ string type

    string args[argc];

    for (int i = 0; i < argc; i++) {
        args[i] = argv[i];
    }

    // input err and mode control

    if (argc == 1) {
        cout << "Please input a filename" << endl;
        return 1;
    }
    else if (argc == 2) {}
    else if (argc == 3) {
        if (args[2] == "-asm") {
            mode_asm = true;
        }
        else if (args[2] == "-showcode") {
            mode_code = true;
        }
        else {
            cout << "Could not find " + args[2] + " mode" << endl;
            return 3;
        }
    }
    else if (argc == 4) {
        if (args[2] == "-asm" && args[3] == "-showcode"){
            mode_asm = true;
            mode_code = true;
        }
        else {
            cout << " Please enter in valid format" << endl;
        }
    }
    else {
        cout << "Please do not input more than three arguments" << endl;
        return 4;
    }

    ifstream input_file (args[1]);

    if (!input_file.is_open()) {
        cout << "Could not open the file" << endl;
        return 2;
    }
    else {
        cout << "Successfully opened the file" << endl << endl;
    }

    // cout code if choose the mode

    if (mode_code) {
        string line;
        while (getline(input_file, line)) {
            cout << line << endl;
        }
    }

    // safe exit

    input_file.close();
}