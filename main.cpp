#include <iostream>
#include <stack>
#include <vector>
#include <queue>
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
IF, ELSE, ELSE_IF, WHILE, FOR,

ASSIGN,                                     // =
LOG_OR,                                     // ||
LOG_AND,                                    // &&
OR,                                         // |
XOR,                                        // ^
AND,                                        // &
EQUAL, NOT,                                 // == !=
LESS, LESS_EQUAL, GREATER, GREATER_EQUAL,   // < <= > >= 
LEFT_MOVE, RIGHT_MOVE,                      // << >>
ADD, SUB,                                   // + -
MUL, DIV, MOD,                              // * / %
LOG_NOT, OPPO,                              // ! ~
INT, FLOAT, CHAR, STRING, CONST,

EOF_, DOT, SEMICOLON, BACK_BRACKET,         // EOF , ; )]}
RET, POINT, FRONT_BRACKET, PRINT, START     // return . ([{ printf
};

int enum_size = START + 1;

// enum token_type {DATA, SCOP, NAME, KEYW, CALC, SYST};

unordered_map<int, int> operation_priority;

unordered_map<int, string> enum_name_list;

struct token{
    int line = 0;
    int type = -1;
    float float_val = 0;
    int int_val = 0;
    string lexeme;
};

class AST_node{
public: 
    int size = 0;
    token val;
    vector<AST_node* > nodes;
    AST_node() : size(0){
        nodes.reserve(size);
    }
    AST_node(int s) : size(s){
        nodes.reserve(size);
    }
    AST_node(int s, int start) : size(s){
        val.lexeme = "START";
        val.type = START;
        nodes.reserve(size);
    }
};

unordered_map<string, int> var_name;

vector<token> tokens;

AST_node* AST_Head;

int parser_index = -1;


void fill_int_enum_name_list(){
    for(int i = 0; i < enum_size; i++){
        switch(i){
            case INT: enum_name_list[i] = "INT"; break;
            case FLOAT: enum_name_list[i] = "FLOAT"; break;
            case CHAR: enum_name_list[i] = "CHAR"; break;
            case STRING: enum_name_list[i] = "STRING"; break;
            case CONST: enum_name_list[i] = "CONST"; break;
            case IF: enum_name_list[i] = "IF"; break;
            case ELSE: enum_name_list[i] = "ELSE"; break;
            case ELSE_IF: enum_name_list[i] = "ELSE_IF"; break;
            case WHILE: enum_name_list[i] = "WHILE"; break;
            case LOG_NOT: enum_name_list[i] = "LOG_NOT"; break;
            case OPPO: enum_name_list[i] = "OPPO"; break;
            case MUL: enum_name_list[i] = "MUL"; break;
            case DIV: enum_name_list[i] = "DIV"; break;
            case MOD: enum_name_list[i] = "MOD"; break;
            case ADD: enum_name_list[i] = "ADD"; break;
            case SUB: enum_name_list[i] = "SUB"; break;
            case LEFT_MOVE: enum_name_list[i] = "LEFT_MOVE"; break;
            case RIGHT_MOVE: enum_name_list[i] = "RIGHT_MOVE"; break;
            case LESS: enum_name_list[i] = "LESS"; break;
            case LESS_EQUAL: enum_name_list[i] = "LESS_EQUAL"; break;
            case GREATER: enum_name_list[i] = "GREATER"; break;
            case GREATER_EQUAL: enum_name_list[i] = "GREATER_EQUAL"; break;
            case EQUAL: enum_name_list[i] = "EQUAL"; break;
            case NOT: enum_name_list[i] = "NOT"; break;
            case AND: enum_name_list[i] = "AND"; break;
            case XOR: enum_name_list[i] = "XOR"; break;
            case OR: enum_name_list[i] = "OR"; break;
            case LOG_AND: enum_name_list[i] = "LOG_AND"; break;
            case LOG_OR: enum_name_list[i] = "LOG_OR"; break;
            case ASSIGN: enum_name_list[i] = "ASSIGN"; break;
            case EOF_: enum_name_list[i] = "EOF_"; break;
            case DOT: enum_name_list[i] = "DOT"; break;
            case SEMICOLON: enum_name_list[i] = "SEMICOLON"; break;
            case BACK_BRACKET: enum_name_list[i] = "BACK_BRACKET"; break;
            case RET: enum_name_list[i] = "RET"; break;
            case POINT: enum_name_list[i] = "POINT"; break;
            case FRONT_BRACKET: enum_name_list[i] = "FRONT_BRACKET"; break;
            case PRINT: enum_name_list[i] = "PRINT"; break;
            case START: enum_name_list[i] = "START"; break;
            default: cout << "error" << endl;
        }
    }
}

void operation_priority_init(){
    int level = 0;
    for(int i = LOG_NOT; i <= CONST; i++){
        if(i == MUL || i == ADD || i == LEFT_MOVE || i == LESS || i == EQUAL || i == AND || i == XOR || i == OR || i == LOG_AND || i == LOG_OR || i == ASSIGN || i == INT) level++;
        operation_priority[i] = level;
    }
}

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
    if(type == FOR) cout << "!!!!!!!!!!!!" << endl;
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

int operator_checker(token &t){
    int type = t.type;
    if(type == INT || type == FLOAT || type == CHAR || type == STRING || type == CONST) return -1;
    else if(type == IF || type == WHILE || type == ELSE_IF || type == WHILE || type == FOR) return -2;
    else if(type == ELSE) return -3;
    else if(type == ASSIGN || type == LOG_OR || type == LOG_AND || type == OR || type == XOR || type == AND && type == EQUAL || type == NOT || type == LESS || type == LESS_EQUAL || type == GREATER || type == GREATER_EQUAL || type == LEFT_MOVE || type == RIGHT_MOVE || type == ADD || type == SUB || type == MUL || type == DIV || type == MOD){
        return 2;
    }
    else if(type == FRONT_BRACKET && t.lexeme == "(") return 3;
    else if(type == FRONT_BRACKET && t.lexeme == "[") return 4;
    else if(type == LOG_NOT || type == OPPO){
        return 1;
    }
    else return 0;
}

int sentence(AST_node* start, char end){
    if()
}

int lexer(ifstream &file){
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
                else if(word == "for"){
                    add_token(line_num, FOR, 0, 0, word);
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
                    add_token(line_num, CONST, f, 0, "FLOAT");
                    cur += size - 1;
                }
                else{
                    add_token(line_num, INT, CONST, num, "INT");
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
                add_token(line_num, CONST, 0, (int)(line[cur + 1]), "CHAR");
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

int parser(AST_node* &cur_head){
    int check = operator_checker(tokens[parser_index]);
    if(parser_index == -1) {
        cur_head = (AST_node* )new AST_node(0, 1);
        parser_index++;
        parser(cur_head);
    }
    else if(check == -2){
        AST_node* new_node = new AST_node(0);
        new_node -> val = tokens[parser_index];
        cur_head -> nodes.push_back(new_node);
        cur_head -> size++;
        parser_index += 2;
        parser(new_node);
        parser_index++;
        if(tokens[parser_index].type == FOR){
            parser(new_node);
            parser(new_node);
        }
        if(tokens[parser_index + 1].lexeme[0] != '{'){
            parser_index++;
            int err = parser(new_node);
            if(err) return parser_index;
            return 0;
        }
        else{
            int save_index = parser_index + 1;
            parser_index += 2;
            while(tokens[parser_index].lexeme[0] != '}'){
                if(tokens[parser_index].type == EOF_) return save_index;
                int err = parser(new_node);
                if(err) return parser_index;
            }
            parser_index++;
            return 0;
        }
    }
    else if(check == -3){
        AST_node* new_node = new AST_node(0);
        new_node -> val = tokens[parser_index];
        cur_head -> nodes.push_back(new_node);
        cur_head -> size++;
        parser(new_node);
    }
    else if(check == -1 && check == 1 && check == 3){
        AST_node* new_node = new AST_node(0);
        new_node -> val = tokens[parser_index];
        cur_head -> nodes.push_back(new_node);
        cur_head -> size++;
        parser_index++;
        char end;
        if(cur_head -> val.type == WHILE || (cur_head -> val.type == FOR && cur_head -> size == 3) || cur_head -> val.type == IF || cur_head -> val.type == ELSE_IF || cur_head -> val.lexeme == "("){
            end = ')';
            if(new_node -> val.lexeme == ")") return parser_index - 1;
        }
        else if(cur_head -> val.lexeme == "["){
            end = ']';
            if(new_node -> val.lexeme == "]") return parser_index - 1;
        }
        else {
            end = ';'
            if(new_node -> val.type == SEMICOLON) return 0;
        }
        int err = sentence(new_node, end);
    }
    else{
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
        else if(args[2] == "-showtoken" && args[3] == "-showast"){
            mode_token = true;
            mode_ast = true;
        }
        else {cout << " Please enter in valid format" << endl; return 1;}
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

    fill_int_enum_name_list();
    if(mode_token) for(token t : tokens) cout << "token line: "<< t.line << "\ttoken type: " << t.type << "\ttoken float_val: " << t.float_val << "\ttoken int_val: " << t.int_val << "\ttoken lexeme: " << t.lexeme << "\t\ttoken type: " << enum_name_list[t.type] << endl;

    // build parser

    operation_priority_init();
    int parser_err = parser(AST_Head);

    if(parser_err) {cout << "Error happened in parser, when analyse the " << parser_err << "th node\nwhilch in " << tokens[parser_err].line << endl; return -1;}
    else cout << "Parser runs successfully" << endl << endl;

    // print ast tree if choose the mode

    if(mode_ast){
        queue<pair<AST_node*, int>> q;
        vector<pair<AST_node*, int>> Ast_Nodes;
        q.push({AST_Head, 0});
        while(!q.empty()){
            auto [cur, level] = q.front(); q.pop();
            Ast_Nodes.push_back({cur, level});
            cout << cur -> size << endl;
            for(int i = 0; i < cur -> size; i++){
                if(cur -> nodes[i]) q.push({cur -> nodes[i], level + 1});
            }
        }
        int cout_level = 0;
        for(auto [cur, level] : Ast_Nodes){
            if(level != cout_level){
                cout << endl;
                cout_level = level;
            }
            cout << "[" << enum_name_list[cur -> val.type] << ", " << cur -> val.lexeme << "] ";
        }
        cout << endl;
    }

    // safe exit

    input_file.close();
}