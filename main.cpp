#include <iostream>
#include <stack>
#include <vector>
#include <string>
#include <algorithm>
#include <fstream>
#include <unordered_set>
#include <unordered_map>
#include <math.h>
using namespace std;

bool mode_asm = false;
bool mode_code = false;
bool mode_token = false;

// enum token_type {INT, FLOAT, CHAR, STRING,   // data type
//             IF, ELSE, ELSE_IF, WHILE, EQUAL, GRATER, GRATER_EQUAL, LESS, LESS_EQUAL, LOG_AND, LOG_OR, LOG_NOT_EQUAL, LOG_NOT,    // logic control
//             AND, OR, XOR, LEFT_MOVE, RIGHT_MOVE, ADD, SUB, ASSIGN, DIV, MUL, MOD, QUES, OPPO, CONST, // culculate
//             RET, EOF_, SEMICOLON, DOT, SINGLE_QUOT, DOUBLE_QUOT, POINT, FRONT_BRACKET, BACK_BRACKET, PRINT};    // others

enum token_type {DATA, SCOP, NAME, KEYW, CALC, SYST};

struct token{
    int line = 0;
    int type = -1;
    float float_val = 0;
    int int_val = 0;
    string lexeme = "";
};

unordered_map<string, int> var_name;

vector<token> tokens;

string get_this_word(string &s, int cur){
    int len = s.size();
    if(cur >= len) return "OUTRANGE";
    string word = "";
    for(int i = cur; i < len; i++){
        if(isalpha(s[i]) || s[i] == '_'){
            word += s[i];
        }
        else break;
    }
    return word;
}

void add_token(int line, int type, int float_val, int int_val, string lexeme){
    token t;
    t.line = line;
    t.type = type;
    t.float_val = float_val;
    t.int_val = int_val;
    t.lexeme = lexeme;
    tokens.push_back(t);
}

int get_next_num(float& f, string line, int cur, int& size){
    int len = line.size();
    string num = "";
    int i = cur;
    bool dot = true;
    for(; i < len; i++){
        if(line[i] == ' ') continue;
        else break;
    }
    for(; i < len; i++){
        if(isdigit(line[i])){
            num += line[i];
        }
        else if(line[i] == '.' && dot){
            num += '.';
            dot = false;
        }
        else if(line[i] == '.' && !dot){
            cout << "Number Error" << endl;
            f = 0.00f / 0.00f;
            return 0;
        }
        else break;
    }
    size = i - cur;
    if(dot){
        return stoi(num);
    }
    else{
        f = stof(num);
        return 0;
    }
}

int parser(ifstream &file){
    string line;
    int line_num = 1;
    bool next_line = false;
    while(getline(file, line)){
        int len = line.size();
        for(int cur = 0; cur < len; cur++){
            if(next_line){
                next_line = false;
                break;
            }
            if(line[cur] == ' ') continue;
            else if(isalpha(line[cur])){
                string word = get_this_word(line, cur);
                if(var_name.find(word) != var_name.end()){
                    add_token(line_num, var_name[word], 0, 0, word);
                    cur += word.size() - 1;
                }
                else if(word == "while"){
                    add_token(line_num, KEYW, 0, 0, word);
                    cur += word.size() - 1;
                }
                else if(word == "printf"){
                    add_token(line_num, SYST, 0, 0, word);
                    cur += word.size() - 1;
                }
                else if(word == "if"){
                    add_token(line_num, KEYW, 0, 0, word);
                    cur += word.size() - 1;
                }
                else if(word == "else" && get_this_word(line, cur + 5) == "if"){
                    add_token(line_num, KEYW, 0, 0, "else_if");
                    cur += 6;
                }
                else if(word == "else"){
                    add_token(line_num, KEYW, 0, 0, word);
                    cur += word.size() - 1;
                }
                else if(word == "return"){
                    add_token(line_num, KEYW, 0, 0, word);
                    cur += word.size() - 1;
                }
                else if(word == "int" || word == "char" || word == "float"){
                    for(int next = cur + word.size() + 1; next <= len; next++){
                        if(next == len){
                            cout << "Error happened in char: " << cur << endl;
                            return line_num;
                        }
                        if(line[next] == ' ') continue;
                        else if(isalpha(line[next])){
                            string next_word = get_this_word(line, next);
                            if(next_word == "OUTRANGE"){
                                cout << "Error happened in char: " << cur << endl;
                                return line_num;
                            }
                            int type = 0;
                            if(word == "int"){
                                add_token(line_num, type, 0, 0, next_word);
                                type = NAME;
                            }
                            else if(word == "char"){
                                add_token(line_num, type, (0 / 0), 0, next_word);
                                type = NAME;
                            }
                            else{
                                add_token(line_num, type, 1, 0, next_word);
                                type = NAME;
                            }
                            var_name[next_word] = type;
                            cur = next + next_word.size() - 1;
                            break;
                        }
                        else{
                            cout << "Error happened in char: " << cur << endl;
                            return line_num;
                        }
                    }
                }
            }
            else if(isdigit(line[cur])){
                float f = 0;
                int size = 0;
                int num = get_next_num(f, line, cur, size);
                if(isnan(f)){
                    cout << "Error happened in char: " << cur << endl;
                    return line_num;
                }
                else if(f > 0){
                    add_token(line_num, DATA, f, 0, "FLOAT");
                    cur += size - 1;
                }
                else{
                    add_token(line_num, DATA, 0, num, "INT");
                    cur += size - 1;
                }
            }
            else if(line[cur] == '#'){
                next_line = true;
                continue;
            }
            else if(cur + 1 < len && line[cur + 1] == '/' && line[cur] == '/'){
                next_line = true;
                continue;
            }
            else if(line[cur] == '/') add_token(line_num, CALC, 0, 0, "/");
            else if(cur + 1 < len && line[cur + 1] == '=' && line[cur] == '=') {add_token(line_num, CALC, 0, 0, "=="); cur++;}
            else if(line[cur] == '=') add_token(line_num, CALC, 0, 0, "=");
            else if(cur + 1 < len && line[cur + 1] == '=' && line[cur] == '!') {add_token(line_num, CALC, 0, 0, "!="); cur++;}
            else if(line[cur] == '!') add_token(line_num, CALC, 0, 0, "!");
            else if(cur + 1 < len && line[cur + 1] == '&' && line[cur] == '&') {add_token(line_num, CALC, 0, 0, "&&"); cur++;}
            else if(line[cur] == '&') add_token(line_num, CALC, 0, 0, "&");
            else if(cur + 1 < len && line[cur + 1] == '|' && line[cur] == '|') {add_token(line_num, CALC, 0, 0, "||"); cur++;}
            else if(line[cur] == '|') add_token(line_num, CALC, 0, 0, "|");
            else if(cur + 1 < len && line[cur + 1] == '=' && line[cur] == '<') {add_token(line_num, CALC, 0, 0, "<="); cur++;}
            else if(cur + 1 < len && line[cur + 1] == '<' && line[cur] == '<') {add_token(line_num, CALC, 0, 0, "<<"); cur++;}
            else if(line[cur] == '<') add_token(line_num, CALC, 0, 0, "<");
            else if(cur + 1 < len && line[cur + 1] == '=' && line[cur] == '>') {add_token(line_num, CALC, 0, 0, ">="); cur++;}
            else if(cur + 1 < len && line[cur + 1] == '>' && line[cur] == '>') {add_token(line_num, CALC, 0, 0, ">>"); cur++;}
            else if(line[cur] == '>') add_token(line_num, CALC, 0, 0, ">");
            else if(line[cur] == '\"') {
                for(int i = cur + 2; i < len - 1; i++){
                    if(line[i] == '\"'){ 
                        add_token(line_num, DATA, 0, -1, line.substr(cur + 1, i - cur - 1));
                        cur = i;
                        break;
                    }
                }
            }
            else if(line[cur] == '\''){
                add_token(line_num, DATA, 0, (int)(line[cur + 1]), "CHAR");
                cur += 2;
            }
            else if(line[cur] == '(') add_token(line_num, SCOP, 0, 0, "(");
            else if(line[cur] == '[') add_token(line_num, SCOP, 0, 1, "[");
            else if(line[cur] == '{') add_token(line_num, SCOP, 0, 2, "{");
            else if(line[cur] == ')') add_token(line_num, SCOP, 0, 0, ")");
            else if(line[cur] == ']') add_token(line_num, SCOP, 0, 1, "]");
            else if(line[cur] == '}') add_token(line_num, SCOP, 0, 2, "}");
            else if(line[cur] == ',') add_token(line_num, SCOP, 0, 0, ",");
            else if(line[cur] == '+') add_token(line_num, CALC, 0, 0, "+");
            else if(line[cur] == '-') add_token(line_num, CALC, 0, 0, "-");
            else if(line[cur] == '*') add_token(line_num, CALC, 0, 0, "*");
            else if(line[cur] == '^') add_token(line_num, CALC, 0, 0, "^");
            else if(line[cur] == '~') add_token(line_num, CALC, 0, 0, "~");
            else if(line[cur] == '%') add_token(line_num, CALC, 0, 0, "%");
            else if(line[cur] == '?') add_token(line_num, CALC, 0, 0, "?");
            else if(line[cur] == ';') add_token(line_num, SCOP, 0, 0, ";");
            else{
                cout << "Error happened in char:\t" << cur << endl;
                return line_num;
            }
        }
        line_num++;
    }
    add_token(line_num, SYST, 0, 0, "EOF");
    return 0;
}

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
        else if(args[2] == "-showtoken"){
            mode_token = true;
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
        else if(args[2] == "-asm" && args[3] == "-showtoken"){
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

    // parser

    int parser_err = parser(input_file);

    if(parser_err != 0){
        cout << " Error happened in line: " << parser_err << endl;
        return -1;
    }
    else{
        cout << "Parser runs successfully" << endl;
    }

    // print tokens if choose the mode

    if(mode_token){
        for(token t : tokens){
            cout << "token line: "<< t.line << "\ttoken type: " << t.type << "\ttoken float_val: " << t.float_val << "\ttoken int_val: " << t.int_val << "\ttoken lexeme: " << t.lexeme << endl;
        }
    }

    // safe exit

    input_file.close();
}