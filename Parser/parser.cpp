#include "parser.h"

#include "path.h"

/******************* 预处理  **********************/
// 将词法分析器得到的token转换为语法分析器所需的token

char token_from_lex_to_grammar(string str, string type) {
  if (type == "KEYWORDS") {
    if (str == "include")
      return 'a';
    else if (str == "iostream")
      return 'b';
    else if (str == "int")
      return 'd';
    else if (str == "main")
      return 'e';
    else if (str == "double")
      return 'f';
    else if (str == "float")
      return 'g';
    else if (str == "char")
      return 'h';
    else if (str == "case")
      return 'j';
    else if (str == "do")
      return 'k';
    else if (str == "default")
      return 'l';
    else if (str == "if")
      return 'm';
    else if (str == "else")
      return 'n';
    else if (str == "for")
      return 'p';
    else if (str == "switch")
      return 'r';
    else if (str == "while")
      return 's';
  } else if (type == "ID") {
    return 'i';
  } else if (type == "CONST") {
    return 'c';
  } else if (type == "LIMITER") {
    if (str == "#")
      return '!';
    else if (str == "{")
      return '{';
    else if (str == "}")
      return '}';
    else if (str == "(")
      return '(';
    else if (str == ")")
      return ')';
    else if (str == ";")
      return ';';
    else if (str == ":")
      return ':';
  } else if (type == "OPERATOR") {
    if (str == "+")
      return '+';
    else if (str == "-")
      return '-';
    else if (str == "*")
      return '*';
    else if (str == "/")
      return '/';
    else if (str == "=")
      return '=';
    else if (str == "++")
      return 't';
    else if (str == "--")
      return 'u';
    else if (str == ">")
      return '>';
    else if (str == "<")
      return '<';
    else if (str == "==")
      return 'v';
    else if (str == "+=")
      return 'w';
    else if (str == "-=")
      return 'x';
  }
}

// 反转换
string token_from_grammar_to_lex(char c){

    if(c == 'a') return "include";
	else if(c == 'b') return "iostream";
	else if(c == 'c') return "CONST";
	else if(c == 'd') return "int";
	else if(c == 'e') return "main";
	else if(c == 'f') return "double";
	else if(c == 'g') return "float";
	else if(c == 'h') return "char";
	else if(c == 'i') return "ID";
	else if(c == 'j') return "case";
	else if(c == 'k') return "do";
	else if(c == 'l') return "default";
	else if(c == 'm') return "if";
	else if(c == 'o') return "OPERATOR";
	else if(c == 'n') return "else";
	else if(c == 'p') return "for";
	else if(c == 'r') return "switch";
	else if(c == 's') return "while";
	else if(c == 'w') return "+=";
	else if(c == 'x') return "-=";
	else if(c == 't') return "++";
	else if(c == 'u') return "--";
	else if(c == 'v') return "==";
	else if(c == '{') return "{";
	else if(c == '}') return "}";
	else if(c == '(') return "(";
	else if(c == ')') return ")";
	else if(c == ';') return ";";
	else if(c == ':') return ":";
	else if(c == '+') return "+";
	else if(c == '-') return "-";
	else if(c == '*') return "*";
	else if(c == '/') return "/";
	else if(c == '!') return "#";
	else if(c == '>') return ">";
	else if(c == '<') return "<";
	else if(c == '=') return "=";
}

// 读取文法文件
void readGrammarFile(){
    
}