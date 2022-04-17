#include <string.h>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>
#include <vector>

#define MAX_NODES 255

using namespace std;
// 定义三元组保存NFA的一条的信息
typedef struct Triad {
  // 边的起点，边的输入（权重），边的终点
  char startPoint, input, endPoint;
} Triad;

// 定义NFA的结构体
typedef struct NFA {
  // 初态、终态、边
  char initialState;
  vector<char> finalState;
  vector<Triad> f;
} NFA;

typedef struct DFA {
  char initialState;
  vector<char> finalState;
  int f[MAX_NODES][MAX_NODES];  //用char隐式转化int作数组下标 实现O(1)的查找
  // f[state][input] = nextState;
} DFA;

extern NFA nfa;                  //保存正规文法 转换成 的NFA
extern char str_file[1000];      //保存文件里读出的内容
extern vector<char> inchar;      //保存终结符VT
extern vector<string> keywords;  //保存关键字
extern vector<string> opt;       //保存操作符
extern vector<string> limiter;   //保存界符

extern DFA dfa;
extern bool Final[MAX_NODES];  //保存DFA状态集中哪些是终结符，便于O(1)查找

extern vector<vector<string> > sourceCode;  //保存源程序
extern vector<pair<string, string> > token;  //保存扫描源程序后得到的token表
extern vector<pair<pair<int, int>, int> > wrong;  //保存错误信息
extern vector<int> row;  //保存每行有多少个token

//初始化
void load_inchar();    //加载终结符
void load_keywords();  //加载关键字
void load_opt();       //加载操作符
void load_limiter();   //加载界符
void init();

// 正规文法→NFA
bool isVT(char a);
void createNFA();
void printNFA();

// NFA→DFA
// 1、set中的元素都是排好序的
// 2、set集合中没有重复的元素
set<char> e_closure(set<char> T);         //求闭包
set<char> move(set<char> I, char input);  //求move集
char change(int a);                       //状态重命名
void NFA_TO_DFA();                        // NFA转换成DFA
void printDFA();                          // 打印DFA

//源代码处理
void spilitSourceCode();
bool isKeywords(string str);
void scanSourceCode();
bool isLimiter(string str);