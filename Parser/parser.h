#include <string.h>

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <vector>
using namespace std;

// 定义文法 产生式的数据结构
typedef struct Grammar {
  char left;
  string right;
} Grammar;
// 定义项目集的中的一条 数据结构
typedef struct Item {
  char left;         //产生式左边
  string right;      //产生式右边
  int position = 0;  //圆点位置 表示在right[position]的左边
  int index;  //对应的文法在文法表中的位置，用于归约时查找
  set<char> forward;  //向前搜索符
  bool operator<(const struct Item& item)
      const {  //重载<，使其能够放在set中进行去重，！！！不进行向前搜素符的比较
    if (this->left == item.left) {
      if (this->right == item.right) {
        if (this->position == item.position) {
          return false;
        } else {
          return this->position < item.position;
        }
      } else {
        return this->right < item.right;
      }
    } else {
      return this->left < item.left;
    }
  }

  //需要比较向前搜素符
  bool operator==(const struct Item& item) const {
    if (this->left == item.left && this->right == item.right &&
        this->position == item.position && this->forward == item.forward)
      return true;
    else
      return false;
  }
} Item;

extern vector<Grammar>
    grammar;  //存放文法,文法中不得出现空行要不然会出现内存访问越界问题
extern set<Item> Itemset[1000];  //项目集族：存放LR(1)的项目集
extern int itemSet_counter;      //最终的状态集的个数
extern set<char> VT;  //存放文法中的终结符,其中不包括epsilon，epsilon单独处理
extern set<char> VN;         //存放文法中的非终结符
extern set<char> toEpsilon;  //存放能够推到epsilon的非终结符
extern map<char, set<char>> FirstSet;  //存文法中的First集
extern bool is_wrong;                  //判断词法分析器是否有错
extern string token;     //存放从词法分析器里读来的token序列
extern vector<int> row;  //存放每行有多少个token
extern int ActionGoto[300][300];  // Action-GOTO表，-1标识接受，-2表示不存在这个状态转移,x表示移进项，x+256表示规约项
//预处理
char token_from_lex_to_grammar(string str, string type);
string token_from_grammar_to_lex(char c);
void readGrammarFile();

// 语法分析
void getFirstSet();
set<char> getForward(char c, set<char> forward);  //前向搜索集
set<Item> getClosureOfItemSet(set<Item> itemSet);
set<Item> Go(char c, set<Item> itemset);  // Go(I,X)
void create(char left, string right);

void scan(string str);
void show();
