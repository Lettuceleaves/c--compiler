#include <iostream>
#include <stack>
#include <vector>
#include <queue>
#include <string>
#include <set>
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

// class declarations

struct token;
struct AST_Node;
union value;
struct var;
struct err_info;
struct value_area;

// function declarations

string get_word(string &line, int &index);
float string_to_float(string s);
int string_to_int(string s);
err_info insert_tokens(string word, int line_num, int index);
err_info lexer(ifstream &input_file);
err_info parser_start(AST_Node* &root);
err_info parser_sentence(AST_Node* &root);
err_info parser_func(AST_Node* &root);
err_info parser_ret(AST_Node* &root);
err_info parser_print(AST_Node* &root);
err_info parser_for(AST_Node* &root);
err_info parser_if(AST_Node* &root);
err_info parser_else(AST_Node* &root);
err_info parser_else_if(AST_Node* &root);
err_info parser_while(AST_Node* &root);
err_info parser_break(AST_Node* &root);
err_info parser_continue(AST_Node* &root);
err_info parser(AST_Node* &root);
err_info insert_word_in_sentence(AST_Node* &root);


string string_helper;

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

    EOF_, COMMA, SEMICOLON, BACK_BRACKET,                     // EOF , ; )]}
    RET, DOT, PRINT, START, BREAK, CONTINUE, EXPLAIN,     // return . ([{ printf
    FUNC
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
    {";", SEMICOLON},
    {")", BACK_BRACKET},
    {"]", BACK_BRACKET},
    {"}", BACK_BRACKET},
    {"return", RET},
    {".", DOT},
    {",", COMMA},
    {"printf", PRINT},
    {"START", START},
    {"break", BREAK},
    {"continue", CONTINUE},
    {"//", EXPLAIN},
    {"\"", STRING},
    {"'", CHAR}
};

union value{
    int int_val;
    float float_val;
    char char_val;
    AST_Node* ast_val;
    value_area* area_val;
    value() {};
    ~value() {};
    value(int i) : int_val(i) {};
    value(float f) : float_val(f) {};
    value(char c) : char_val(c) {};
    value(AST_Node* a) : ast_val(a) {};
    value(value_area* v) : area_val(v) {};
};

struct token {
    int line = -1;
    int index = -1;
    int val_type = -1; // -2 save for var
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

struct AST_Node {
    int token_index;
    vector<AST_Node*> children;
    AST_Node() {};
    ~AST_Node() {};
    AST_Node(int ti) : token_index(ti) {};
};

class value_area {
private:
    vector<value> values;
    unordered_map<string, pair<int, int>> vars;
    value_area* upper;

public:

    value_area() : upper(nullptr) {}

    value_area(value_area* u) : upper(u) {}

    bool insert_value(string name, int type) {
        if(vars.find(name) != vars.end()) return false;
        vars[name] = {values.size(), type};
        value val;
        if(type == INT) val = value(0);
        else if(type == FLOAT) val = value(0.0f);
        else if(type == CHAR) val = value('0');
        else if(type == STRING) val = value(0);
        else if(type == FUNC) val = value((AST_Node* )nullptr);
        else return false;
        values.push_back(val);
        return true;
    }

    void insert_circle(string name, AST_Node* continue_des, AST_Node* break_des){
        vars[name] = {-1, -1};
        values.push_back(continue_des);
        values.push_back(break_des);
    }

    bool check_type(string name, int &type){
        if(vars.find(name) == vars.end()){
            if (upper) return upper->check_type(name, type);
            return false;
        }
        type = vars[name].second;
        return true;
    }

    bool check_value(string name, value &val){
        if(vars.find(name) == vars.end() && !((name == "break" || name == "continue") && (vars.find("for") != vars.end() || vars.find("while") != vars.end()))){
            if (upper) return upper->check_value(name, val);
            return false;
        }
        if(name == "continue"){
            val = values[0];
            return true;
        }
        else if(name == "break"){
            val = values[1];
            return true;
        }
        else{
            val = values[vars[name].first];
            return true;
        }
    }

    void set_upper(value_area* u){
        upper = u;
    }
};

#define AST_Pointer AST_Node*;

vector<token> tokens;

vector<value> values;

vector<var> vars;

vector<string> string_pool;

vector<pair<int, AST_Node*>> func_pool;

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
                if(word == "\"") {
                    index++;
                    while(line[index] != '\"') {
                        string_helper += line[index];
                        index++;
                        if(index >= line.size()) return "OUT_OF_BOUND";
                    }
                    index++;
                }
                else if(word == "'") {
                    index += 2;
                    if(index >= line.size()) return "OUT_OF_BOUND";
                    string_helper += line[index - 1];
                }
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
            if(key_word.word == "("){
                if(tokens.size() >= 1){
                    if(tokens[tokens.size() - 2].type == INT || tokens[tokens.size() - 2].type == FLOAT || tokens[tokens.size() - 2].type == CHAR || tokens[tokens.size() - 2].type == STRING){
                        tokens[tokens.size() - 1].type = FUNC;
                        func_pool.push_back({0, nullptr});
                    }
                    return {false, 0, 0, "", ""};
                }
            }
            else if(key_word.word == "'"){
                if(string_helper.size() != 1) return {true, line_num, index - word.size(), word, ""};
                value val = value((char)(string_helper[0]));
                values.push_back(val);
                return {false, 0, 0, "", ""};
            }
            else if(key_word.word == "\""){
                string_pool.push_back(string_helper);
                value val(int(string_pool.size() - 1));
                values.push_back(val);
                string_helper = "";
                return {false, 0, 0, "", ""};
            }
            return {false, 0, 0, "", ""};
        }
    }
    if(isdigit(word[0])) {
        if(word.find('.') != string::npos) {
            token new_token(line_num, index - word.size(), FLOAT, 0, CONST, word);
            tokens.push_back(new_token);
            values.push_back(string_to_float(word));
        }
        else {
            token new_token(line_num, index - word.size(), INT, 0, CONST, word);
            tokens.push_back(new_token);
            values.push_back(string_to_int(word));
        }
        return {false, 0, 0, "", ""};
    }
    else if(isalpha(word[0]) || word[0] == '_') {
        if(tokens.size() >= 1 && (tokens[tokens.size() - 1].lexeme == "int" || tokens[tokens.size() - 1].lexeme == "float" || tokens[tokens.size() - 1].lexeme == "char" || tokens[tokens.size() - 1].lexeme == "string")) {
            var new_var(word, tokens[tokens.size() - 1].type, 0);
            vars.push_back(new_var);
            token new_token(line_num, index - word.size(), 0, 0, -2, word);
            tokens.push_back(new_token);
            return {false, 0, 0, "", ""};
        }
        else{
            for(int i = vars.size() - 1; i >= 0; i--) {
                if(vars[i].name == word) {
                    token new_token(line_num, index - word.size(), 0, 0, -2, word);
                    tokens.push_back(new_token);
                    return {false, 0, 0, "", ""};
                }
            }
            return {true, line_num, index - word.size(), "lexer", word};
        }
    }
    return {true, line_num, index - word.size(), "lexer", word};
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
            if(cur == "OUT_OF_BOUND") return {true, line_num, index, "lexer", cur};
            if(cur == "//") break;
            err_info err = insert_tokens(cur, line_num, index);
            if(err.err) return err;
            if(cur.size() == 0) return {true, line_num, index, "lexer", cur};
        }
    }
    tokens.push_back(token(line_num, 0, -1, 0, EOF_, "EOF"));
    return {false, 0, 0, "", ""};
}

int parser_cur_index = 0;

AST_Node* AST_root;

set<int> sentence_elements = {-2, INT, FLOAT, CHAR, STRING, FRONT_BRACKET, CONST};

vector<value_area* > value_areas;

err_info parser_start(AST_Node* &root){
    tokens[root->token_index].val = value_areas.size();
    value_areas.push_back(new value_area());
    parser_cur_index++;
    while(tokens[parser_cur_index].type != EOF_){
        err_info err = parser(root);
        if(err.err) return err;
    }
    return {false, 0, 0, "", ""};
}

unordered_map<int, int> word_priority = {
    {ASSIGN, 1}, // =
    {LOG_OR, 2}, // ||
    {LOG_AND, 3}, // &&
    {OR, 4}, // |
    {XOR, 5}, // ^
    {AND, 6}, // &
    {EQUAL, 7}, {NOT, 7}, // == !=
    {LESS, 8}, {LESS_EQUAL, 8}, {GREATER, 8}, {GREATER_EQUAL, 8}, // < <= > >= 
    {LEFT_MOVE, 9}, {RIGHT_MOVE, 9}, // << >>
    {ADD, 10}, {SUB, 10}, // + -
    {MUL, 11}, {DIV, 11}, {MOD, 11}, // * / %
    {LOG_NOT, 12}, {OPPO, 12}, // ! ~
    {INT, 13}, {FLOAT, 13}, {CHAR, 13}, {STRING, 13}, {FRONT_BRACKET, 13}, {CONST, 13} // int float char string ( const
};

err_info insert_word_in_sentence(AST_Node* &root) {
    int cur_priority = word_priority[tokens[parser_cur_index].type];
    int root_priority;
    if(root == nullptr){
        root = new AST_Node(parser_cur_index);
        parser_cur_index++;
        return {false, 0, 0, "", ""};
    }
    else root_priority = word_priority[tokens[root->token_index].type];
    if(cur_priority <= root_priority){
        AST_Node* new_node = new AST_Node(parser_cur_index);
        new_node->children.push_back(root);
        root = new_node;
    }
    else{
        AST_Node* cur = root;
        while(cur->children.size() == 2 && cur_priority > word_priority[tokens[cur->children[cur->children.size() - 1]->token_index].type]){
            cur = cur->children[cur->children.size() - 1];
            if(cur == nullptr) return {true, tokens[parser_cur_index].line, tokens[parser_cur_index].index, "parser", tokens[parser_cur_index].lexeme};
        }
        AST_Node* new_node = new AST_Node(parser_cur_index);
        if(cur->children.size() == 2){
            AST_Node* temp = cur->children[1];
            cur->children[1] = new_node;
            new_node->children.push_back(temp);
        }
        else cur->children.push_back(new_node);
    }
    parser_cur_index++;
    return {false, 0, 0, "", ""};
}

err_info parser_sentence(AST_Node* &root){
    AST_Node* sentence_root = nullptr;
    int end_type = SEMICOLON;
    if(tokens[root->token_index].type == FRONT_BRACKET || tokens[root->token_index].type == WHILE || tokens[root->token_index].type == IF || tokens[root->token_index].type == ELSE_IF){
        end_type = BACK_BRACKET;
    }
    else if(tokens[root->token_index].type == FOR && root->children.size() == 2){
        end_type = BACK_BRACKET;
    }
    else if(tokens[root->token_index].type == FUNC){
        if(root->children.size() < values[tokens[parser_cur_index].val].int_val){
            end_type = COMMA;
        }
        else if(root->children.size() == values[tokens[parser_cur_index].val].int_val){
            end_type = BACK_BRACKET;
        }
    }
    while(tokens[parser_cur_index].type != end_type){
        if(tokens[parser_cur_index].type == INT || tokens[parser_cur_index].type == FLOAT || tokens[parser_cur_index].type == CHAR || tokens[parser_cur_index].type == STRING){
            if(tokens[parser_cur_index + 1].type != -2 && tokens[parser_cur_index + 1].type != FUNC) return {true, tokens[parser_cur_index].line, tokens[parser_cur_index].index, "parser", tokens[parser_cur_index].lexeme};
            if(!value_areas[tokens[root->token_index].val]->insert_value(tokens[parser_cur_index + 1].lexeme, tokens[parser_cur_index].type)){
                return {true, tokens[parser_cur_index].line, tokens[parser_cur_index].index, "parser", tokens[parser_cur_index].lexeme};
            }
            parser_cur_index++;
            continue;
        }
        else if(tokens[parser_cur_index].type == -2){
            int type;
            if(!value_areas[tokens[root->token_index].val]->check_type(tokens[parser_cur_index].lexeme, type)){
                return {true, tokens[parser_cur_index].line, tokens[parser_cur_index].index, "parser", tokens[parser_cur_index].lexeme};
            }
            tokens[parser_cur_index].val_type = type;
            tokens[parser_cur_index].type = type;
        }
        err_info err = insert_word_in_sentence(sentence_root);
        if(err.err) return err;
    }
    root->children.push_back(sentence_root);
    parser_cur_index++;
    return {false, 0, 0, "", ""};
}

err_info parser_func(AST_Node* &root){
    AST_Node* cur_ast = new AST_Node(parser_cur_index);
    parser_cur_index++;
    root->children.push_back(cur_ast);
    return {false, 0, 0, "", ""};
}
    
err_info parser_ret(AST_Node* &root){
    AST_Node* cur_ast = new AST_Node(parser_cur_index);
    parser_cur_index++;
    root->children.push_back(cur_ast);
    return {false, 0, 0, "", ""};
}

err_info parser_print(AST_Node* &root){
    AST_Node* cur_ast = new AST_Node(parser_cur_index);
    parser_cur_index++;
    root->children.push_back(cur_ast);
    return {false, 0, 0, "", ""};
}

err_info parser_for(AST_Node* &root){
    if(tokens[parser_cur_index + 1].type != FRONT_BRACKET) return {true, tokens[parser_cur_index].line, tokens[parser_cur_index].index, "parser", tokens[parser_cur_index].lexeme};
    if(value_areas[tokens[root->token_index].val] == nullptr) return {true, tokens[parser_cur_index].line, tokens[parser_cur_index].index, "parser", tokens[parser_cur_index].lexeme};
    tokens[parser_cur_index].val = value_areas.size();
    value_areas.push_back(new value_area(values[tokens[root->token_index].val].area_val));
    value_areas[value_areas.size() - 1]->set_upper(value_areas[tokens[root->token_index].val]);
    value_areas[value_areas.size() - 1]->insert_circle("for", root->children[root->children.size()], root->children[root->children.size() + 1]);
    AST_Node* cur_ast = new AST_Node(parser_cur_index);
    parser_cur_index += 2;
    for(int i = 0; i < 3; i++){
        err_info err = parser(cur_ast);
        if(err.err) return err;
    }
    if(tokens[parser_cur_index].lexeme == "{"){
        parser_cur_index++;
        while(tokens[parser_cur_index].lexeme != "}"){
            err_info err = parser(cur_ast);
            if(err.err) return err;
        }
        parser_cur_index++;
    }
    else{
        err_info err = parser(root);
        if(err.err) return err;
    }
    root->children.push_back(cur_ast);
    return {false, 0, 0, "", ""};
}

err_info parser_if(AST_Node* &root){
    if(tokens[parser_cur_index + 1].type != FRONT_BRACKET) return {true, tokens[parser_cur_index].line, tokens[parser_cur_index].index, "parser", tokens[parser_cur_index].lexeme};
    if(value_areas[tokens[root->token_index].val] == nullptr) return {true, tokens[parser_cur_index].line, tokens[parser_cur_index].index, "parser", tokens[parser_cur_index].lexeme};
    tokens[parser_cur_index].val = value_areas.size();
    value_areas.push_back(new value_area(values[tokens[root->token_index].val].area_val));
    value_areas[value_areas.size() - 1]->set_upper(value_areas[tokens[root->token_index].val]);
    AST_Node* cur_ast = new AST_Node(parser_cur_index);
    parser_cur_index += 2;
    err_info err = parser(cur_ast);
    if(err.err) return err;
    if(tokens[parser_cur_index].lexeme == "{"){
        parser_cur_index++;
        while(tokens[parser_cur_index].lexeme != "}"){
            err_info err = parser(cur_ast);
            if(err.err) return err;
        }
        parser_cur_index++;
    }
    else{
        err = parser(cur_ast);
        if(err.err) return err;
    }
    root->children.push_back(cur_ast);
    return {false, 0, 0, "", ""};
}

err_info parser_else(AST_Node* &root){
    if(tokens[parser_cur_index + 1].type != FRONT_BRACKET) return {true, tokens[parser_cur_index].line, tokens[parser_cur_index].index, "parser", tokens[parser_cur_index].lexeme};
    if(value_areas[tokens[root->token_index].val] == nullptr) return {true, tokens[parser_cur_index].line, tokens[parser_cur_index].index, "parser", tokens[parser_cur_index].lexeme};
    tokens[parser_cur_index].val = value_areas.size();
    value_areas.push_back(new value_area(values[tokens[root->token_index].val].area_val));
    value_areas[value_areas.size() - 1]->set_upper(value_areas[tokens[root->token_index].val]);
    AST_Node* cur_ast = new AST_Node(parser_cur_index);
    parser_cur_index += 2;
    if(tokens[parser_cur_index].lexeme == "{"){
        parser_cur_index++;
        while(tokens[parser_cur_index].lexeme != "}"){
            err_info err = parser(cur_ast);
            if(err.err) return err;
        }
        parser_cur_index++;
    }
    else{
        err_info err = parser(cur_ast);
        if(err.err) return err;
    }
    root->children.push_back(cur_ast);
    return {false, 0, 0, "", ""};
}

err_info parser_else_if(AST_Node* &root){
    if(tokens[parser_cur_index + 1].type != FRONT_BRACKET) return {true, tokens[parser_cur_index].line, tokens[parser_cur_index].index, "parser", tokens[parser_cur_index].lexeme};
    if(value_areas[tokens[root->token_index].val] == nullptr) return {true, tokens[parser_cur_index].line, tokens[parser_cur_index].index, "parser", tokens[parser_cur_index].lexeme};
    tokens[parser_cur_index].val = value_areas.size();
    value_areas.push_back(new value_area(values[tokens[root->token_index].val].area_val));
    value_areas[value_areas.size() - 1]->set_upper(value_areas[tokens[root->token_index].val]);
    AST_Node* cur_ast = new AST_Node(parser_cur_index);
    parser_cur_index += 2;
    err_info err = parser(cur_ast);
    if(err.err) return err;
    if(tokens[parser_cur_index].lexeme == "{"){
        parser_cur_index++;
        while(tokens[parser_cur_index].lexeme != "}"){
            err_info err = parser(cur_ast);
            if(err.err) return err;
        }
        parser_cur_index++;
    }
    else{
        err = parser(root);
        if(err.err) return err;
    }
    root->children.push_back(cur_ast);
    return {false, 0, 0, "", ""};
}

err_info parser_while(AST_Node* &root){
    if(tokens[parser_cur_index + 1].type != FRONT_BRACKET) return {true, tokens[parser_cur_index].line, tokens[parser_cur_index].index, "parser", tokens[parser_cur_index].lexeme};
    if(value_areas[tokens[root->token_index].val] == nullptr) return {true, tokens[parser_cur_index].line, tokens[parser_cur_index].index, "parser", tokens[parser_cur_index].lexeme};
    tokens[parser_cur_index].val = value_areas.size();
    value_areas.push_back(new value_area(values[tokens[root->token_index].val].area_val));
    value_areas[value_areas.size() - 1]->set_upper(value_areas[tokens[root->token_index].val]);
    value_areas[value_areas.size() - 1]->insert_circle("while", root->children[root->children.size()], root->children[root->children.size() + 1]);
    AST_Node* cur_ast = new AST_Node(parser_cur_index);
    parser_cur_index += 2;
    err_info err = parser(cur_ast);
    if(err.err) return err;
    if(tokens[parser_cur_index].lexeme == "{"){
        parser_cur_index++;
        while(tokens[parser_cur_index].lexeme != "}"){
            err_info err = parser(cur_ast);
            if(err.err) return err;
        }
        parser_cur_index++;
    }
    else{
        err = parser(cur_ast);
        if(err.err) return err;
    }
    root->children.push_back(cur_ast);
    return {false, 0, 0, "", ""};
}

err_info parser_break(AST_Node* &root){
    AST_Node* cur_ast = new AST_Node(parser_cur_index);
    if(tokens[parser_cur_index + 1].type != SEMICOLON) return {true, tokens[parser_cur_index].line, tokens[parser_cur_index].index, "parser", tokens[parser_cur_index].lexeme};
    parser_cur_index += 2;
    root->children.push_back(cur_ast);
    value val;
    if(!value_areas[tokens[root->token_index].val]->check_value("break", val)){
        return {true, tokens[parser_cur_index].line, tokens[parser_cur_index].index, "parser", tokens[parser_cur_index].lexeme};
    }
    AST_Node* break_root = val.ast_val;
    root->children.push_back(break_root);
    return {false, 0, 0, "", ""};
}

err_info parser_continue(AST_Node* &root){
    AST_Node* cur_ast = new AST_Node(parser_cur_index);
    if(tokens[parser_cur_index + 1].type != SEMICOLON) return {true, tokens[parser_cur_index].line, tokens[parser_cur_index].index, "parser", tokens[parser_cur_index].lexeme};
    parser_cur_index += 2;
    root->children.push_back(cur_ast);
    value val;
    if(!value_areas[tokens[root->token_index].val]->check_value("continue", val)){
        return {true, tokens[parser_cur_index].line, tokens[parser_cur_index].index, "parser", tokens[parser_cur_index].lexeme};
    }
    AST_Node* continue_root = val.ast_val;
    root->children.push_back(continue_root);
    return {false, 0, 0, "", ""};
}

err_info parser(AST_Node* &root) {
    if(tokens[parser_cur_index].type == START){
        err_info err = parser_start(root);
        if(err.err) return err;
    }
    else if(tokens[parser_cur_index].type == FUNC){
        err_info err = parser_func(root);
        if(err.err) return err;
    }
    else if(tokens[parser_cur_index].type == RET){
        err_info err = parser_ret(root);
        if(err.err) return err;
    }
    else if(tokens[parser_cur_index].type == PRINT){
        err_info err = parser_print(root);
        if(err.err) return err;
    }
    else if(tokens[parser_cur_index].type == BREAK){
        err_info err = parser_break(root);
        if(err.err) return err;
    }
    else if(tokens[parser_cur_index].type == CONTINUE){
        err_info err = parser_continue(root);
        if(err.err) return err;
    }
    else if((sentence_elements.find(tokens[parser_cur_index].type) != sentence_elements.end() && (tokens[parser_cur_index + 1].type != FUNC)) || tokens[parser_cur_index].type == INT || tokens[parser_cur_index].type == FLOAT || tokens[parser_cur_index].type == CHAR || tokens[parser_cur_index].type == STRING){
        err_info err = parser_sentence(root);
        if(err.err) return err;
    }
    else if(tokens[parser_cur_index].type == IF){
        err_info err = parser_if(root);
        if(err.err) return err;
    }
    else if(tokens[parser_cur_index].type == ELSE){
        err_info err = parser_else(root);
        if(err.err) return err;
    }
    else if(tokens[parser_cur_index].type == ELSE_IF){
        err_info err = parser_else_if(root);
        if(err.err) return err;
    }
    else if(tokens[parser_cur_index].type == WHILE){
        err_info err = parser_while(root);
        if(err.err) return err;
    }
    else if(tokens[parser_cur_index].type == FOR){
        err_info err = parser_for(root);
        if(err.err) return err;
    }
    else if(tokens[parser_cur_index].type == EOF_){
        return {false, 0, 0, "", ""};
    }
    else{
        return {true, tokens[parser_cur_index].line, tokens[parser_cur_index].index, "parser", tokens[parser_cur_index].lexeme};
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

    err_info err;

    // lexer
    tokens.push_back(token(0, 0, -1, 0, START, "START"));
    err = lexer(input_file);

    if(err.err){
        cout << "Error at line " << err.line << ", index " << err.index << ", " << err.part << ": " << err.word << endl;
        return 1;
    }
    else cout << "Lexer runs successfully" << endl << endl;

    if(mode_token){
        cout << "Tokens:" << endl;
        for (const auto& tok : tokens) {
            cout << "Line: " << tok.line << ", Index: " << tok.index << ", Type: " << tok.type << ", Val Type: " << tok.val_type << ", Val: " << tok.val << ", Lexeme: " << tok.lexeme << endl;
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

    // parser
    AST_root = new AST_Node(0);
    err = parser(AST_root);

    if(err.err){
        cout << "Error at line " << err.line << ", index " << err.index << ", " << err.part << ": " << err.word << endl;
        return 1;
    }
    else cout << "Parser runs successfully" << endl << endl;

    if(mode_ast){
        function<void(AST_Node*, int)> print_ast = [&](AST_Node* node, int depth) {
            if (!node) return;
            for (int i = 0; i < depth; ++i) cout << "  ";
            cout << tokens[node->token_index].lexeme << endl;
            for (auto child : node->children) {
                print_ast(child, depth + 1);
            }
        };

        print_ast(AST_root, 0);
    }

    // safe exit

    input_file.close();
}