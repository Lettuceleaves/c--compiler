#ifndef C_COMPILER_2_H
#define C_COMPILER_2_H

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

// class declarations

struct token;
struct AST_Node;
union value;
struct var;
struct err_info;
struct value_area;

bool mode_token = false;
bool mode_ast = false;
bool mode_code = false;
string string_helper;
int parser_cur_index = 0;
AST_Node* AST_root;

vector<pair<int, AST_Node*>> func_pool;
vector<string> string_pool; // record strings in tokens -- lexer
vector<token> tokens;
vector<value> values; // record values of const in tokens except for string -- lexer
vector<var> vars; // record vars in tokens to make pair -- lexer
enum token_type {
    IF = 0, ELSE = 1, ELSE_IF = 2, WHILE = 3, FOR = 4,

    ASSIGN = 5,                                                 // = 5
    LOG_OR = 6,                                                 // || 6
    LOG_AND = 7,                                                // && 7
    OR = 8,                                                     // | 8
    XOR = 9,                                                    // ^ 9
    AND = 10,                                                   // & 10
    EQUAL = 11, NOT = 12,                                       // == 11 != 12
    LESS = 13, LESS_EQUAL = 14, GREATER = 15, GREATER_EQUAL = 16, // < 13 <= 14 > 15 >= 16
    LEFT_MOVE = 17, RIGHT_MOVE = 18,                            // << 17 >> 18
    ADD = 19, SUB = 20,                                         // + 19 - 20
    MUL = 21, DIV = 22, MOD = 23,                               // * 21 / 22 % 23
    LOG_NOT = 24, OPPO = 25,                                    // ! 24 ~ 25
    INT = 26, FLOAT = 27, CHAR = 28, STRING = 29, FRONT_BRACKET = 30, CONST = 31,

    EOF_ = 32, COMMA = 33, SEMICOLON = 34, BACK_BRACKET = 35,   // EOF 32 , 33 ; 34 )] 35
    RET = 36, DOT = 37, PRINT = 38, START = 39, BREAK = 40, CONTINUE = 41, EXPLAIN = 42, // return 36 . 37 printf 38 START 39 break 40 continue 41 // 42
    FUNC = 43
};
set<int> sentence_elements = {-2, INT, FLOAT, CHAR, STRING, FRONT_BRACKET, CONST};
vector<value_area* > value_areas;
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
    {INT, 13}, {FLOAT, 13}, {CHAR, 13}, {STRING, 13}, {FRONT_BRACKET, 13}, {CONST, 13}, {FUNC, 13} // int float char string ( const
};

// function declarations

string get_word(string &line, int &index);
float string_to_float(string s);
int string_to_int(string s);
err_info insert_tokens(string word, int line_num, int index);
err_info lexer(ifstream &input_file);
err_info parser_start(AST_Node* &root);
err_info parser_sentence(AST_Node* &root, AST_Node* val_area_root);
err_info parser_func(AST_Node* &root);
err_info parser_ret(AST_Node* &root);
err_info parser_for(AST_Node* &root);
err_info parser_if(AST_Node* &root);
err_info parser_else(AST_Node* &root);
err_info parser_else_if(AST_Node* &root);
err_info parser_while(AST_Node* &root);
err_info parser_break(AST_Node* &root);
err_info parser_continue(AST_Node* &root);
err_info parser(AST_Node* &root, AST_Node* val_area_root = nullptr);
err_info insert_word_in_sentence(AST_Node* &root, AST_Node* &sentence_root);

#endif