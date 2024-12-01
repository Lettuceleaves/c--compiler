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
bool mode_ast = false;

// enum token_type {
//                     CONST, RET, EOF_, SEMICOLON, DOT, 
//                     SINGLE_QUOT, DOUBLE_QUOT, POINT,
//                     FRONT_BRACKET, BACK_BRACKET, PRINT};    // others

enum token_type {
INT, FLOAT, CHAR, STRING,
IF, ELSE, ELSE_IF, WHILE,

LOG_NOT, OPPO,                              // ! ~
MUL, DIV, MOD, ADD, SUB,                    // * / % + -
LEFT_MOVE, RIGHT_MOVE,                      // << >>
LESS, LESS_EQUAL, GREATER, GREATER_EQUAL,   // < <= > >= 
EQUAL, NOT,                                 // == !=
AND,                                        // &
XOR,                                        // ^
OR,                                         // |
LOG_AND,                                    // &&
LOG_OR,                                     // ||
ASSIGN,                                     // =

EOF_, DOT, SEMICOLON, BACK_BRACKET,        // EOF , ; )]}
RET, POINT, FRONT_BRACKET, PRINT            // return . ([{ printf
};

// enum token_type {DATA, SCOP, NAME, KEYW, CALC, SYST};

unordered_map<int, int> operation_priority;

void operation_priority_init(){
    int level = 0;
    for(int i = LOG_NOT; i <= OPPO; i++){
        if(i == MUL || i == LEFT_MOVE || i == LESS || i == EQUAL || i == AND || i == XOR || i == OR || i == LOG_AND || i == LOG_OR || i == ASSIGN) level++;
        operation_priority[i] = level;
        cout << i << " " << level << endl;
    }
}

struct token{
    int line = 0;
    int type = -1;
    float float_val = 0;
    int int_val = 0;
    string lexeme = "";
};

class AST_node{
    public: int size = 0;
    public: token val;
    public: vector<AST_node* > nodes;
    AST_node(){this -> size = 0;};
    AST_node(int s){
        this -> size = s;
        vector<AST_node* > t(0);
        this -> nodes = t;
    };
};

unordered_map<string, int> var_name;

vector<token> tokens;

AST_node* AST_Head;

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

int lexer(ifstream &file){
    
    string line;
    int line_num = 1;
    bool next_line = false;
    add_token(0, EOF_, 0, 0, "");
    while(getline(file, line)){
        int len = line.size();
        for(int cur = 0; cur < len; cur++){
            if(next_line){
                next_line = false;
                break;
            }
            if(line[cur] == ' ') continue;
            else if(line[cur] == '\t') continue;
            else if(isalpha(line[cur])){
                string word = get_this_word(line, cur);
                if(var_name.find(word) != var_name.end()){
                    add_token(line_num, var_name[word], 0, 0, word);
                    cur += word.size() - 1;
                }
                else if(word == "while"){
                    add_token(line_num, WHILE, 0, 0, word);
                    cur += word.size() - 1;
                }
                else if(word == "printf"){
                    add_token(line_num, PRINT, 0, 0, word);
                    cur += word.size() - 1;
                }
                else if(word == "if"){
                    add_token(line_num, IF, 0, 0, word);
                    cur += word.size() - 1;
                }
                else if(word == "else" && get_this_word(line, cur + 5) == "if"){
                    add_token(line_num, ELSE_IF, 0, 0, "else_if");
                    cur += 6;
                }
                else if(word == "else"){
                    add_token(line_num, ELSE, 0, 0, word);
                    cur += word.size() - 1;
                }
                else if(word == "return"){
                    add_token(line_num, RET, 0, 0, word);
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
                            if(word == "int") type = INT;
                            else if(word == "char") type = CHAR;
                            else type = FLOAT;
                            add_token(line_num, type, 0, 0, next_word);
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
                    add_token(line_num, FLOAT, f, 0, "$");
                    cur += size - 1;
                }
                else{
                    add_token(line_num, INT, 0, num, "$");
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
            else if(line[cur] == '/') add_token(line_num, DIV, 0, 0, "/");
            else if(cur + 1 < len && line[cur + 1] == '=' && line[cur] == '=') {add_token(line_num, EQUAL, 0, 0, "=="); cur++;}
            else if(line[cur] == '=') add_token(line_num, ASSIGN, 0, 0, "=");
            else if(cur + 1 < len && line[cur + 1] == '=' && line[cur] == '!') {add_token(line_num, LOG_NOT, 0, 0, "!="); cur++;}
            else if(line[cur] == '!') add_token(line_num, NOT, 0, 0, "!");
            else if(cur + 1 < len && line[cur + 1] == '&' && line[cur] == '&') {add_token(line_num, LOG_AND, 0, 0, "&&"); cur++;}
            else if(line[cur] == '&') add_token(line_num, AND, 0, 0, "&");
            else if(cur + 1 < len && line[cur + 1] == '|' && line[cur] == '|') {add_token(line_num, LOG_OR, 0, 0, "||"); cur++;}
            else if(line[cur] == '|') add_token(line_num, OR, 0, 0, "|");
            else if(cur + 1 < len && line[cur + 1] == '=' && line[cur] == '<') {add_token(line_num, LESS_EQUAL, 0, 0, "<="); cur++;}
            else if(cur + 1 < len && line[cur + 1] == '<' && line[cur] == '<') {add_token(line_num, LEFT_MOVE, 0, 0, "<<"); cur++;}
            else if(line[cur] == '<') add_token(line_num, LESS, 0, 0, "<");
            else if(cur + 1 < len && line[cur + 1] == '=' && line[cur] == '>') {add_token(line_num, GREATER_EQUAL, 0, 0, ">="); cur++;}
            else if(cur + 1 < len && line[cur + 1] == '>' && line[cur] == '>') {add_token(line_num, RIGHT_MOVE, 0, 0, ">>"); cur++;}
            else if(line[cur] == '>') add_token(line_num, GREATER, 0, 0, ">");
            else if(line[cur] == '\"') {
                for(int i = cur + 2; i < len - 1; i++){
                    if(line[i] == '\"'){ 
                        add_token(line_num, STRING, 0, -1, line.substr(cur + 1, i - cur - 1));
                        cur = i;
                        break;
                    }
                }
            }
            else if(line[cur] == '\''){
                add_token(line_num, CHAR, 0, (int)(line[cur + 1]), "CHAR");
                cur += 2;
            }
            else if(line[cur] == '(') add_token(line_num, FRONT_BRACKET, 0, 0, "(");
            else if(line[cur] == '[') add_token(line_num, FRONT_BRACKET, 0, 1, "[");
            else if(line[cur] == '{') add_token(line_num, FRONT_BRACKET, 0, 2, "{");
            else if(line[cur] == ')') add_token(line_num, BACK_BRACKET, 0, 0, ")");
            else if(line[cur] == ']') add_token(line_num, BACK_BRACKET, 0, 1, "]");
            else if(line[cur] == '}') add_token(line_num, BACK_BRACKET, 0, 2, "}");
            else if(line[cur] == ',') add_token(line_num, DOT, 0, 0, ",");
            else if(line[cur] == '+') add_token(line_num, ADD, 0, 0, "+");
            else if(line[cur] == '-') add_token(line_num, SUB, 0, 0, "-");
            else if(line[cur] == '*') add_token(line_num, MUL, 0, 0, "*");
            else if(line[cur] == '^') add_token(line_num, XOR, 0, 0, "^");
            else if(line[cur] == '~') add_token(line_num, OPPO, 0, 0, "~");
            else if(line[cur] == '%') add_token(line_num, MOD, 0, 0, "%");
            else if(line[cur] == ';') add_token(line_num, SEMICOLON, 0, 0, ";");
            else{
                cout << "Error happened in char:\t" << cur << endl;
                return line_num;
            }
        }
        line_num++;
    }
    add_token(line_num, EOF_, 0, 0, "EOF");
    return 0;
}

int parser(int index, AST_node* cur){
    if(tokens[index].line == 0) {cur = (AST_node* )new AST_node(1); parser(++index, cur -> nodes[0]);}
    else if((tokens[index].type == INT || tokens[index].type == FLOAT) && tokens[index].lexeme != "$"){
        static AST_node* new_node = new AST_node(0);
        new_node -> val = tokens[index];
        cur -> nodes.push_back(new_node);
        cur -> size++;
        int re = parser(index++, cur -> nodes[0]);
    }
    else if(tokens[index].type == ASSIGN){
        static AST_node* new_node = new AST_node(0);
        new_node -> val = tokens[index];
        cur -> nodes.push_back(new_node);
        cur -> size++;
        int re = parser(index++, cur -> nodes[0]);
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
    else if (argc == 2) {}
    else if (argc == 3) {
        if (args[2] == "-asm") mode_asm = true;
        else if (args[2] == "-showcode") mode_code = true;
        else if (args[2] == "-showtoken") mode_token = true;
        else if (args[2] == "-showast") mode_ast = true;
        else { cout << "Could not find " + args[2] + " mode" << endl; return 3;}
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
        else cout << " Please enter in valid format" << endl;
    }
    else {
        cout << "Please do not input more than three arguments" << endl;
        return 4;
    }

    ifstream input_file (args[1]);

    if (!input_file.is_open()) { cout << "Could not open the file" << endl; return 2;}
    else cout << "Successfully opened the file" << endl << endl;

    // cout code if choose the mode

    if (mode_code) { string line; while (getline(input_file, line)) cout << line << endl;}

    // lexer

    int lexer_err = lexer(input_file);

    if(lexer_err != 0){ cout << " Error happened in line: " << lexer_err << endl; return -1;}
    else cout << "Lexer runs successfully" << endl << endl;

    // print tokens if choose the mode

    if(mode_token) for(token t : tokens) cout << "token line: "<< t.line << "\ttoken type: " << t.type << "\ttoken float_val: " << t.float_val << "\ttoken int_val: " << t.int_val << "\ttoken lexeme: " << t.lexeme << endl;

    // build parser

    operation_priority_init();
    int parser_err = parser(0, AST_Head);

    if(parser_err) return -1;
    else cout << "Parser runs successfully" << endl << endl;

    // print ast tree if choose the mode

    if(mode_ast) cout << "unsupport" << endl << endl;

    // safe exit

    input_file.close();
}