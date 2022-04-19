#include<iostream>
#include<string>
#include<vector>
#include<fstream>
#include<set>
#include<map>
using namespace std;

// 定义文法 产生式的数据结构
typedef struct Grammar{
    char left;
    string right;
}Grammar;

typedef struct Item{
	char left;               					  //产生式左边 
	string right;            					 //产生式右边 
	int position = 0;           				 //圆点位置 表示在right[position]的左边 
	int index;                					 //对应的文法在文法表中的位置，用于归约时查找 
	set<char> forward;      					 //向前搜索符 
	bool operator<(const struct Item & item) const{                //重载<使其能够放在set中进行去重排序的 
		if(this->left == item.left){
			if(this->right == item.right){
				if(this->position == item.position){
					return false;
				}else{
					return this->position < item.position;
				}
			}else{
				return this->right < item.right;
			}
		}else{
			return this->left < item.left;
		}
	}
	
	//和<不同，set中用于排序，去重的是用<来进行判等的 
	bool operator==(const struct Item & item) const{
		if(this->left == item.left && this->right == item.right && this->position == item.position && this->forward == item.forward )
			return true;
		else return false;
	} 

}Item;



extern vector<Grammar> grammar;//存放文法,文法中不得出现空行要不然会出现内存访问越界问题 
extern set<Item> Itemset[1000];               //存放LR(1)的项目集 
extern int totalNodes;                       //最终的状态集的个数 
extern set<char> VT;                         //存放文法中的终结符,其中不包括epsilon，epsilon单独处理 
extern set<char> VN;                      //存放文法中的非终结符 
extern set<char> toEpsilon;                  //存放能够推到epsilon的非终结符 
extern map<char,set<char> > FirstVT;             //存文法中的非终结符对应的First集 
extern bool is_wrong = false;                          //判断词法分析器是否有错 
extern string token = "";                       //存放从词法分析器里读来的token序列 
extern vector<int> row;                        //存放每行有多少个token 

//预处理 
char token_from_lex_to_grammar(string str,string type);
string token_from_grammar_to_lex(char c);
void readGrammarFile();

// 语法分析
void getFirstVT();
set<char> getForward(char c,set<char> forward);
