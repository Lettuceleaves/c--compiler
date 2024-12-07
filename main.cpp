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

bool mode_asm = false;
bool mode_code = false;
bool mode_token = false;
bool mode_ast = false;

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
    AST_node& operator = (const AST_node& other){
        if(this != &other){
            val = other.val;
            size = other.size;
            nodes = other.nodes;
        }
    }
};

unordered_map<string, int> var_name;

vector<token> tokens;

AST_node* AST_Head;

int parser_index = -1;


bool fill_int_enum_name_list(){
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
            case FOR: enum_name_list[i] = "FOR"; break;
            default: cout << "error in function fill_int_enum_name_list()" << endl; return false;
        }
    }
    return true;
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

AST_node* check_priority_in_tree(AST_node* cur){
    int cur_priority;
    if(cur -> val.type == FRONT_BRACKET) cur_priority = INT_MAX;
    else cur_priority = operation_priority[cur -> val.type];
    int token_priority;
    if(tokens[parser_index].type == FRONT_BRACKET) token_priority = INT_MAX;
    else token_priority = operation_priority[tokens[parser_index].type];

    // if should replace cur_node

    if(token_priority < cur_priority){
        return nullptr;
    }

    // check wether cur is parent

    int size = cur -> size;
    if(size == 0) return cur;
    else if(size == 1){
        int left_prority = (cur -> nodes[0] -> val.type == FRONT_BRACKET) ? INT_MAX : operation_priority[cur -> nodes[0] -> val.type];
        if(left_prority > token_priority) return cur;
        else return check_priority_in_tree(cur -> nodes[0]);
    }
    else{
        int left_prority = (cur -> nodes[0] -> val.type == FRONT_BRACKET) ? INT_MAX : operation_priority[cur -> nodes[0] -> val.type];
        int right_prority = (cur -> nodes[1] -> val.type == FRONT_BRACKET) ? INT_MAX : operation_priority[cur -> nodes[1] -> val.type];
        if(left_prority <= token_priority) return check_priority_in_tree(cur -> nodes[0]);
        else if(right_prority <= token_priority) return check_priority_in_tree(cur -> nodes[1]);
        else return cur;
    }
    return cur;
}

int sentence(AST_node* start, int end){
    while(tokens[parser_index].type != EOF_){
        if(tokens[parser_index].type == SEMICOLON && end == 2) return 0;
        else if(tokens[parser_index].lexeme == "]" && end == 1) return 0;
        else if(tokens[parser_index].lexeme == ")" && end == 0) return 0;
        else if(tokens[parser_index].type < ASSIGN || tokens[parser_index].type > CONST) return parser_index;
        if(tokens[parser_index].lexeme == "(" || tokens[parser_index].lexeme == "["){
            AST_node* insert_parent_node = check_priority_in_tree(start);
            AST_node* new_node = new AST_node(0);
            new_node -> val = tokens[parser_index];
            parser_index++;
            if(tokens[parser_index].lexeme == ")" || tokens[parser_index].lexeme == "]"){
                return parser_index;
            }
            int new_end;
            if(tokens[parser_index].lexeme == "(") new_end = 0;
            else new_end = 1;
            int err = sentence(new_node, new_end);
            if(err) return parser_index;
            int parent_size = insert_parent_node -> size;
            if(parent_size < 2){
                insert_parent_node -> nodes.push_back(new_node);
                insert_parent_node -> size++;
            }
            else if(parent_size == 2){
                AST_node* tmp = new AST_node(0);
                *tmp = *insert_parent_node;
                *insert_parent_node = *new_node;
                new_node -> nodes.push_back(tmp);
                new_node -> size++;
            }
            else return parser_index;
        }
        else{
            AST_node* insert_parent_node = check_priority_in_tree(start); ////////////////////////////////////////////////////////////////////////////////左树低且右树空时应该插入而不是push
            AST_node* new_node = new AST_node(0);
            new_node -> val = tokens[parser_index];
            if(insert_parent_node){
                int parent_size = insert_parent_node -> size;
                if(parent_size < 2){
                    insert_parent_node -> nodes.push_back(new_node);
                    insert_parent_node -> size++;
                }
                else if(parent_size == 2){
                    AST_node* tmp = new AST_node(0);
                    *tmp = *insert_parent_node;
                    *insert_parent_node = *new_node;
                    new_node -> nodes.push_back(tmp);
                    new_node -> size++;
                }
                else return parser_index;
            }
            else{
                AST_node* tmp = new AST_node(0);
                *tmp = *start;
                *start = *new_node;
                new_node -> nodes.push_back(tmp);
                new_node -> size++;
            }
        }
        parser_index++;
    }
    return parser_index;
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

int debug_counter = 0;

int parser(AST_node* &cur_head){
    cout << parser_index << endl;
    debug_counter++;
    if(debug_counter >= 100) return -1;
    if(parser_index >= (int)tokens.size()) return parser_index;
    int check = operator_checker(tokens[parser_index]);
    if(parser_index == -1) {
        cur_head = (AST_node* )new AST_node(0, 1);
        parser_index++;
        while(tokens[parser_index].type != EOF_){
            int err = parser(cur_head);
            // debug

            // if(AST_Head -> size > 0) cout << "debug: " << AST_Head -> nodes[1] -> val.lexeme << endl << endl;
            if(err == tokens.size()) return 0;

            if(err){
                cout << "start error: " << err << endl;
                return parser_index;
            }
        }
        return 0;
    }
    else if(check == -2){
        AST_node* new_node = new AST_node(0);
        new_node -> val = tokens[parser_index];
        cur_head -> nodes.push_back(new_node);
        cur_head -> size++;
        parser_index += 2;
        int err = parser(new_node);
        if(err) return parser_index;
        parser_index++;
        if(tokens[parser_index].type == FOR){
            int err1 = parser(new_node);
            if(err1) return parser_index;
            int err2 = parser(new_node);
            if(err2) return parser_index;
        }
        if(tokens[parser_index + 1].lexeme[0] != '{'){
            parser_index++;
            err = parser(new_node);
            if(err) return parser_index;
            return 0;
        }
        else{
            int save_index = parser_index + 1;
            parser_index += 2;
            while(tokens[parser_index].lexeme[0] != '}'){
                if(tokens[parser_index].type == EOF_) return save_index;
                err = parser(new_node);
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
    else if(check == -1 || check == 1 || check == 3){
        AST_node* new_node = new AST_node(0);
        new_node -> val = tokens[parser_index];
        cur_head -> nodes.push_back(new_node);
        cur_head -> size++;
        parser_index++;
        int end;
        if(cur_head -> val.type == WHILE || (cur_head -> val.type == FOR && cur_head -> size == 3) || cur_head -> val.type == IF || cur_head -> val.type == ELSE_IF || cur_head -> val.lexeme == "("){
            end = 0;
            if(new_node -> val.lexeme == ")") return parser_index - 1;
        }
        else if(cur_head -> val.lexeme == "["){
            end = 1;
            if(new_node -> val.lexeme == "]") return parser_index - 1;
        }
        else {
            end = 2;
            if(new_node -> val.type == SEMICOLON) return 0;
        }
        cout << "operation end: " << end << endl;
        int err = sentence(new_node, end);
        if(err) return err;
        parser_index++;
    }
    else if(cur_head != AST_Head && tokens[parser_index].type == EOF_) return parser_index;
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

    if (mode_code) { string line; while (getline(input_file, line)) cout << line << endl; return 0;}

    // lexer

    int lexer_err = lexer(input_file);

    if(lexer_err != 0){ cout << " Error happened in line: " << lexer_err << endl; return -1;}
    else cout << "Lexer runs successfully" << endl << endl;

    // print tokens if choose the mode

    if(fill_int_enum_name_list() == false) return 1;
    if(mode_token) for(int i = 0; i < tokens.size(); i++) cout << i << "\ttoken line: "<< tokens[i].line << "\ttoken type: " << tokens[i].type << "\ttoken float_val: " << tokens[i].float_val << "\ttoken int_val: " << tokens[i].int_val << "\ttoken lexeme: " << tokens[i].lexeme << "\t\ttoken type: " << enum_name_list[tokens[i].type] << endl;

    // build parser

    operation_priority_init();
    int parser_err = parser(AST_Head);

    if(parser_err) {cout << "Error happened in parser, when analyse the " << parser_err << "th node\nwhilch in " << tokens[parser_err].line << endl; return -1;}
    else cout << "Parser runs successfully" << endl << endl;

    // print ast tree if choose the mode

    // if(mode_ast){
    //     queue<pair<AST_node*, int>> q;
    //     vector<pair<AST_node*, int>> Ast_Nodes;
    //     q.push({AST_Head, 0});
    //     int level_max = 0;
    //     while(!q.empty()){
    //         auto [cur, level] = q.front(); q.pop();
    //         level_max = max(level_max, level);
    //         Ast_Nodes.push_back({cur, level});
    //         for(int i = 0; i < cur -> size; i++){
    //             if(cur -> nodes[i]) q.push({cur -> nodes[i], level + 1});
    //         }
    //     }
    //     int cout_level = 0;
    //     unordered_map<int, int> level_count;
    //     for(auto [cur, level] : Ast_Nodes){
    //         level_count[level]++;
    //         if(level != cout_level){
    //             cout << endl;
    //             cout_level = level;
    //         }
    //         cout << "[" << enum_name_list[cur -> val.type] << ", " << cur -> val.lexeme << "] ";
    //     }
    //     cout << endl;
    // }

    if(mode_ast){
        function<void(AST_node*, int)> dfs = [&](AST_node* cur, int level){
            string space = "    ";
            for(int i = 0; i < level; i++) cout << space;
            cout << cur -> val.lexeme << " " << cur -> size << endl;
            for(int i = 0; i < cur -> size; i++){
                dfs(cur -> nodes[i], level + 1);
            }
            return;
        };
        dfs(AST_Head, 0);
    }

    // safe exit

    input_file.close();
}