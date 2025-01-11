#include <iostream>
#include <stack>
#include <vector>
#include <queue>
#include <string>
#include <algorithm>
#include <fstream>
#include <unordered_set>
#include <unordered_map>
#include <functional>
#include <math.h>
using namespace std;

// global variables
bool mode_token = false;
bool mode_ast = false;
bool mode_code = false;

enum token_type {
    IF, ELSE, ELSE_IF, WHILE, FOR,

    ASSIGN,                                                 // =
    LOG_OR,                                                 // ||
    LOG_AND,                                                // &&
    OR,                                                     // |
    XOR,                                                    // ^
    AND,                                                    // &
    EQUAL, NOT,                                             // == !=
    LESS, LESS_EQUAL, GREATER, GREATER_EQUAL,               // < <= > >= 
    LEFT_MOVE, RIGHT_MOVE,                                  // << >>
    ADD, SUB,                                               // + -
    MUL, DIV, MOD,                                          // * / %
    LOG_NOT, OPPO,                                          // ! ~
    INT, FLOAT, CHAR, STRING, FRONT_BRACKET, CONST,

    EOF_, DOT, SEMICOLON, BACK_BRACKET,                     // EOF , ; )]}
    RET, POINT, PRINT, START, BREAK, CONTINUE, EXPLAIN      // return . ([{ printf
};

struct key_word {
    string word;
    int type;
};

vector<key_word> key_words = {
    {"if", IF},
    {"else", ELSE},
    {"else if", ELSE_IF},
    {"while", WHILE},
    {"for", FOR},
    {"=", ASSIGN},
    {"||", LOG_OR},
    {"&&", LOG_AND},
    {"|", OR},
    {"^", XOR},
    {"&", AND},
    {"==", EQUAL},
    {"!=", NOT},
    {"<", LESS},
    {"<=", LESS_EQUAL},
    {">", GREATER},
    {">=", GREATER_EQUAL},
    {"<<", LEFT_MOVE},
    {">>", RIGHT_MOVE},
    {"+", ADD},
    {"-", SUB},
    {"*", MUL},
    {"/", DIV},
    {"%", MOD},
    {"!", LOG_NOT},
    {"~", OPPO},
    {"int", INT},
    {"float", FLOAT},
    {"char", CHAR},
    {"string", STRING},
    {"(", FRONT_BRACKET},
    {"const", CONST},
    {".", DOT},
    {";", SEMICOLON},
    {"}", BACK_BRACKET},
    {"return", RET},
    {".", POINT},
    {"printf", PRINT},
    {"START", START},
    {"break", BREAK},
    {"continue", CONTINUE},
    {"//", EXPLAIN}
};

string get_word(string &line, int &index) {
    string word;
    if(isalpha(line[index]) || line[index] == '_') {
        while(isalpha(line[index]) || line[index] == '_') {
            word += line[index];
            index++;
        }
    }
    else if(isdigit(line[index])) {
        while(isdigit(line[index]) || line[index] == '.') {
            word += line[index];
            index++;
        }
    }
    else if(line.size() - index >= 2) {
        string temp = line.substr(index, 2);
        for(auto &key_word : key_words) {
            if(key_word.word == temp) {
                word = temp;
                index += 2;
                return word;
            }
        }
        temp = line.substr(index, 1);
        for(auto &key_word : key_words) {
            if(key_word.word == temp) {
                word = temp;
                index++;
                return word;
            }
        }
    }
    else if(line.size() - index >= 1) {
        string temp = line.substr(index, 1);
        for(auto &key_word : key_words) {
            if(key_word.word == temp) {
                word = temp;
                index++;
                return word;
            }
        }
    }
    return word;
}

struct token {
    int line = -1;
    int index = -1;
    int val_type = -1;
    int val = -1;
    int type = -1;
};

union value{
    int int_val;
    float float_val;
    char char_val;
    value() {};
    ~value() {};
    value(int i) : int_val(i) {};
    value(float f) : float_val(f) {};
    value(char c) : char_val(c) {};
};

struct var {
    string name;
    int type;
    int val;
};

vector<token> tokens;

vector<value> values;

vector<var> vars;

float string_to_float(string s) {
    float res = 0;
    float dec = 0;
    bool is_dec = false;
    bool is_neg = false;
    for(int i = 0; i < s.size(); i++) {
        if(s[i] == '-') {
            if(i != 0) return 0.00f / 0.00f;
            is_neg = true;
            continue;
        }
        if(s[i] == '.') {
            if(i == 0 || i == s.size() - 1) return 0.00f / 0.00f;
            is_dec = true;
            continue;
        }
        if(is_dec) {
            dec = dec * 10 + s[i] - '0';
        }
        else {
            res = res * 10 + s[i] - '0';
        }
    }
    while(dec > 1) dec /= 10;
    res += dec;
    if(is_neg) res = -res;
    return res;
}

int string_to_int(string s) {
    int res = 0;
    bool is_neg = false;
    for(int i = 0; i < s.size(); i++) {
        if(s[i] == '-') {
            if(i != 0) return 0;
            is_neg = true;
            continue;
        }
        res = res * 10 + s[i] - '0';
    }
    if(is_neg) res = -res;
    return res;
}

void insert_tokens(string &word, int line_num, int index) {
    if(word == "int" || word == "char" || word == "float" || word == "string") {
        return;
    }
    for(auto &key_word : key_words) {
        if(key_word.word == word) {
            token new_token;
            new_token.line = line_num;
            new_token.index = index;
            new_token.type = key_word.type;
            tokens.push_back(new_token);
            return;
        }
    }
    token new_token;
    new_token.line = line_num;
    new_token.index = index;
    if(isdigit(word[0])) {
        if(word.find('.') != string::npos) {
            new_token.type = FLOAT;
            new_token.val_type = FLOAT;
            new_token.val = values.size();
            values.push_back(string_to_float(word));
        }
        else {
            new_token.type = INT;
            new_token.val_type = INT;
            new_token.val = values.size();
            values.push_back(string_to_int(word));
        }
    }
    else if(isalpha(word[0]) || word[0] == '_') {
        new_token.type = CONST;
        if(tokens.size() >= 2){
            if(tokens[tokens.size() - 2].type == INT || tokens[tokens.size() - 2].type == FLOAT || tokens[tokens.size() - 2].type == CHAR || tokens[tokens.size() - 2].type == STRING) {
                var new_var;
                new_var.name = word;
                new_var.type = tokens[tokens.size() - 2].type;
                new_var.val = values.size();
                vars.push_back(new_var);
            }
        }
        else{
            for(auto &var : vars) {
                if(var.name == word) {
                    new_token.val_type = var.type;
                    new_token.val = var.val;
                    break;
                }
            }
        }
    }
    tokens.push_back(new_token);
    return;
}

int lexer(ifstream &input_file) {
    string line;
    int line_num = 0;
    while (getline(input_file, line)) {
        line_num++;
        int len = line.size();
        for (int index = 0; index < len;) {
            if(line[index] == ' ' || line[index] == '\t'){
                index++;
                continue;
            }
            string cur = get_word(line, index);
            cout << cur << " ";
            insert_tokens(cur, line_num, index);
            if(index == len - 1) index++;
        }
    }
    return 0;
}

int main(int argc, char* argv[]) {

    // input check print

    for (int i = 0; i < argc; i++) {
        cout << endl << "i: " << i << " argv: " << argv[i] << endl;
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
    else if(argc > 4){
        cout << "Too many arguments" << endl;
        return 1;
    }
    for(int i = 0; i < argc; i++){
        if(args[i] == "-t") mode_token = true;
        if(args[i] == "-a") mode_ast = true;
        if(args[i] == "-c") mode_code = true;
    }

    ifstream input_file (args[1]);

    if (!input_file.is_open()) { cout << "Could not open the file" << endl; return 2;}
    else cout << "Successfully opened the file" << endl << endl;

    // cout code if choose the mode

    if (mode_code) { string line; while (getline(input_file, line)) cout << line << endl; return 0;}

    // lexer

    int lexer_err = lexer(input_file);

    if(lexer_err != 0){ cout << " Error happened in line: " << lexer_err << endl; return -1;}
    else cout << "Lexer runs successfully" << endl << endl;

    if(mode_token){
        cout << "Tokens:" << endl;
        for (const auto& tok : tokens) {
            cout << "Line: " << tok.line << ", Index: " << tok.index << ", Type: " << tok.type << ", Val Type: " << tok.val_type << ", Val: " << tok.val << endl;
        }

        cout << "Values:" << endl;
        for (const auto& val : values) {
            if (val.int_val) {
                cout << "Int: " << val.int_val << endl;
            } else if (val.float_val) {
                cout << "Float: " << val.float_val << endl;
            } else if (val.char_val) {
                cout << "Char: " << val.char_val << endl;
            }
        }

        cout << "Vars:" << endl;
        for (const auto& var : vars) {
            cout << "Name: " << var.name << ", Type: " << var.type << ", Val: " << var.val << endl;
        }
    }

    // safe exit

    input_file.close();
}