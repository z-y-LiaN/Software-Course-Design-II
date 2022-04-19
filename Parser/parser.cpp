#include "parser.h"

#include "path.h"


vector<Grammar> grammar;               //存放文法,文法中不得出现空行要不然会出现内存访问越界问题 
set<Item> Itemset[1000];               //存放LR(1)的项目集 
int totalNodes;                       //最终的状态集的个数 
set<char> VT;                         //存放文法中的终结符,其中不包括epsilon，epsilon单独处理 
set<char> VN;                      //存放文法中的非终结符 
set<char> toEpsilon;                  //存放能够推到epsilon的非终结符 
map<char,set<char> > FirstVT;             //存文法中的非终结符对应的First集 

bool is_wrong = false;                          //判断词法分析器是否有错 
string token = "";                       //存放从词法分析器里读来的token序列 
vector<int> row;                        //存放每行有多少个token 
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
  // 暂存文法，便于多次遍历
    vector<string>temp;
    fstream file;
    file.open(GRAMMAR_FILE_PATH);
    char str_file[100];;
    while (file.getline(str_file, 100)) {
        string str(str_file);
        temp.push_back(str);
    }
    for(int i=0;i<temp.size();i++){
      VN.insert(temp[i][0]);//左边第一个字符为非终结符
      Grammar g;
      g.left=temp[i][0];
      g.right=temp[i].substr(3,temp[i].size()-3);
      grammar.push_back(g);
      if(g.right=="$") toEpsilon.insert(g.left);
      // 读终结符
      for(int j=3;j<temp[i].size();j++){
        if((temp[i][j]<'A'||temp[i][j]>'Z')&&temp[i][j]!='$')
          VT.insert(temp[i][j]);
      }
    }
    // 补充完toEpsilon的集合
    int before_count=-1;
    int next_count=toEpsilon.size();
    while(next_count!=before_count){
      for(int i=0;i<grammar.size();i++){
        bool flag=true;
        for(int j=0;j<grammar[i].right.size();j++){
          if(grammar[i].right[j]=='$'||toEpsilon.find(grammar[i].right[j])!=toEpsilon.end())
            continue;
          else flag=false;
        }
        if(flag) toEpsilon.insert(grammar[i].left);
      }
      before_count=next_count;
      next_count=toEpsilon.size();
    }
    file.close();
    // 分析词法分析器里是否有错误信息
    file.open(WRONG_FILE_PATH);
    char c=file.get();
    if(!file.eof()){
      is_wrong=true;
    }
    file.close();
    // 读token序列
    file.open(TOKEN_FILE_PATH);
    while(file.getline(str_file,100)){
      string s=str_file;
      string str="";
      string type="";
      int i=0;
      bool flag =false;
      for(int i=0;i<s.length();i++){
        if(s[i]==' '){
          flag =true;
          i+=9;
        }
        if(!flag) str+=s[i];
        else type+=s[i];
      }
      token+=token_from_lex_to_grammar(str,type);
    }
    file.close();
    // 读一行有多少个token
    file.open(ROW_FILE_PATH);
    int row_count;
    while(file>>row_count){
      row.push_back(row_count);
    }
    file.close();
}


/* ****************** 语法分析*********************/
// 求非终结符的First集
void getFirstVT(){
  int before_sum=-1;
  int next_sum=0;
  while(next_sum!=before_sum){
    for(int i=0;i<grammar.size();i++){
      string str=grammar[i].right;
      int size=str.size();

      if(size==1){
        // 终结符 或epsilon
        if(str[0]=='$'|| VT.find(str[0])!=VT.end()){
          FirstVT[grammar[i].left].insert(str[0]);
        }else{
          // A -> B
          set<char> temp =FirstVT[str[0]];
          FirstVT[grammar[i].left].insert(temp.begin(),temp.end());
        }
      }else{
        for(int j=0;j<str.size();j++){
          if(str[j]=='$') continue;
          else if(VT.find(str[j])!=VT.end()){
            // A->aB....
            FirstVT[grammar[i].left].insert(str[j]);
            break;
          }else{
            // A->BCD
						set<char> temp = FirstVT[str[j]];
						set<char>::iterator it = temp.find('$');
						if(it != temp.end()){
							//如果B可以推到epsilon 
							if(j != str.size()-1) temp.erase(it);
							FirstVT[grammar[i].left].insert(temp.begin(),temp.end());
						}else{
							//如果B不能推到epsilon
							FirstVT[grammar[i].left].insert(temp.begin(),temp.end());
							break; 
						}
          }
        }

      }

    }
    // 每轮统计FirstVT的集合总个数，如果不再改变时收敛
    int sum=0;
    for(map<char,set<char> >::iterator it=FirstVT.begin();it!=FirstVT.end();it++){
      sum+=(*it).second.size();
    }
    before_sum=next_sum;
    next_sum=sum;
  }
}

// 向前搜索符集合
set<char> getForward(char c,set<char> forward){
  set<char> s;
	if(c == '$'){
		//S->.S, # 
		return forward;
	}else if(VT.find(c) != VT.end()){
		//S->.Sa,#
		s.insert(c);
		return s;
	}else{
		//是否按照toEpsilon来判断无区别 
		if(FirstVT[c].find('$') != FirstVT[c].end()){
			//S->.SB,#      (epsilon belongs to B)
			set<char> temp = FirstVT[c];
			temp.erase(temp.find('$'));
			temp.insert(forward.begin(),forward.end());
			return temp;
		}else{
			//S->.SB,#     (epsilon doesn't belong to B)
			return FirstVT[c];
		}
	}
}

// 求项目集的闭包
// 求GO函数，实现状态转移
// 建立DFA，创建ACTION—GOTO表


/*********************源程序处理****************/
// 展示部分结果
// 扫描词法分析器的token序列进行分析

