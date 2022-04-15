#include<iostream>
#include<vector>
#include<fstream>
using namespace std;
// 定义三元组保存NFA的一条的信息
typedef struct Triad{
    // 边的起点，边的输入（权重），边的终点
    char startPoint,input,endPoint;
}Triad;

// 定义NFA的结构体
typedef struct NFA{
    // 初态、终态、边
    char initialState;
    vector<char> finalState;
    vector<Triad> f;
}NFA;

extern NFA nfa;//保存正规文法 转换成 的NFA
extern char str_file[1000]; 						//保存文件里读出的内容 
extern vector<char> inchar; 						//保存终结符VT
extern vector<string> keywords;                    //保存关键字
extern vector<string> opt;                         //保存操作符
extern vector<string> limiter;                     //保存界符 

void load_inchar(); //加载终结符
void load_keywords();//加载关键字
void load_opt();   //加载操作符
void load_limiter();//加载界符
void init();//初始化

bool isVT(char a);
void createNFA();
void printNFA();
void initial();