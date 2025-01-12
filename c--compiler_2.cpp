#include <iostream>
#include <stack>
#include <vector>
#include <queue>
#include <string>
#include <algorithm>
#include <fstream>
#include <unordered_set>
#include <map>
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
    {"[", FRONT_BRACKET},
    {"{", FRONT_BRACKET},
    {"const", CONST},
    {".", DOT},
    {";", SEMICOLON},
    {")", BACK_BRACKET},
    {"]", BACK_BRACKET},
    {"}", BACK_BRACKET},
    {"return", RET},
    {".", POINT},
    {"printf", PRINT},
    {"START", START},
    {"break", BREAK},
    {"continue", CONTINUE},
    {"//", EXPLAIN}
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

struct token {
    int line = -1;
    int index = -1;
    int val_type = -1;
    int val = -1;
    int type = -1;
    string lexeme;
    token() {};
    ~token() {};
    token(int l, int i, int vt, int v, int t, string lex) : line(l), index(i), val_type(vt), val(v), type(t), lexeme(lex) {};
};

struct var {
    string name;
    int type;
    int val;
    var() {};
    ~var() {};
    var(string n, int t, int v) : name(n), type(t), val(v) {};
};

struct err_info{
    bool err;
    int line;
    int index;
    string part;
    string word;
};

vector<token> tokens;

vector<value> values;

vector<var> vars;

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

err_info insert_tokens(string word, int line_num, int index) {
    for(auto &key_word : key_words) {
        if(key_word.word == word) {
            token new_token(line_num, index - word.size(), -1, 0, key_word.type, word);
            tokens.push_back(new_token);
            return {false, 0, 0, "", ""};
        }
    }
    if(isdigit(word[0])) {
        if(word.find('.') != string::npos) {
            token new_token(line_num, index - word.size(), FLOAT, 0, FLOAT, word);
            values.push_back(string_to_float(word));
        }
        else {
            token new_token(line_num, index - word.size(), INT, 0, INT, word);
            values.push_back(string_to_int(word));
        }
        return {false, 0, 0, "", ""};
    }
    else if(isalpha(word[0]) || word[0] == '_') {
        if(tokens.size() >= 1){
            if(tokens[tokens.size() - 1].lexeme == "int" || tokens[tokens.size() - 1].lexeme == "float" || tokens[tokens.size() - 1].lexeme == "char" || tokens[tokens.size() - 1].lexeme == "string") {
                var new_var(word, tokens[tokens.size() - 1].type, 0);
                vars.push_back(new_var);
                map<string, int> type_map = {{"int", INT}, {"float", FLOAT}, {"char", CHAR}, {"string", STRING}};
                token new_token(line_num, index - word.size(), type_map[tokens[tokens.size() - 1].lexeme], 0, tokens[tokens.size() - 1].type, word);
                return {false, 0, 0, "", ""};
            }
            else{
                for(int i = vars.size() - 1; i >= 0; i--) {
                    if(vars[i].name == word) {
                        token new_token(line_num, index - word.size(), vars[i].type, 0, vars[i].type, word);
                        tokens.push_back(new_token);
                        return {false, 0, 0, "", ""};
                    }
                }
                return {true, line_num, index - word.size(), word, ""};
            }
        }
        else{
            for(int i = vars.size() - 1; i >= 0; i--) {
                if(vars[i].name == word) {
                    token new_token(line_num, index - word.size(), vars[i].type, 0, vars[i].type, word);
                    tokens.push_back(new_token);
                    return {false, 0, 0, "", ""};
                }
            }
            return {true, line_num, index - word.size(), word, ""};
        }
    }
    return {true, line_num, index - word.size(), word, ""};
}

err_info lexer(ifstream &input_file) {
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
            err_info err = insert_tokens(cur, line_num, index);
            if(err.err) return err;
            if(cur.size() == 0) return {true, line_num, index, "", ""};
        }
    }
    return {false, 0, 0, "", ""};
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

    if (mode_code) { string line; cout << endl; while (getline(input_file, line)) cout << line << endl; cout << endl; return 0;}

    // lexer

    err_info err = lexer(input_file);

    if(err.err) cout << "Error at line " << err.line << ", index " << err.index << ", " << err.part << ": " << err.word << endl;
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