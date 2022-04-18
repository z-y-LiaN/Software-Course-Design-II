#include<iostream>
#include<string>
using namespace std;

// 定义文法 产生式的数据结构
typedef struct Grammar{
    char left;
    string right;
}Grammar;

//预处理 
char token_from_lex_to_grammar(string str,string type);
string token_from_grammar_to_lex(char c);
void readGrammarFile();
