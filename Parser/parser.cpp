#include "parser.h"

#include "path.h"

vector<Grammar>grammar;  //存放文法,文法中不得出现空行要不然会出现内存访问越界问题
set<Item> Itemset[1000];  //存放LR(1)的项目集
int totalNodes;           //最终的状态集的个数
set<char> VT;  //存放文法中的终结符,其中不包括epsilon，epsilon单独处理
set<char> VN;                   //存放文法中的非终结符
set<char> toEpsilon;            //存放能够推到epsilon的非终结符
map<char, set<char> > FirstSet;  //存文法中的非终结符对应的First集

bool is_wrong = false;  //判断词法分析器是否有错
string token = "";      //存放从词法分析器里读来的token序列
vector<int> row;        //存放每行有多少个token

int AG[300][300];                     //Action-GOTO表
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
string token_from_grammar_to_lex(char c) {
  if (c == 'a')
    return "include";
  else if (c == 'b')
    return "iostream";
  else if (c == 'c')
    return "CONST";
  else if (c == 'd')
    return "int";
  else if (c == 'e')
    return "main";
  else if (c == 'f')
    return "double";
  else if (c == 'g')
    return "float";
  else if (c == 'h')
    return "char";
  else if (c == 'i')
    return "ID";
  else if (c == 'j')
    return "case";
  else if (c == 'k')
    return "do";
  else if (c == 'l')
    return "default";
  else if (c == 'm')
    return "if";
  else if (c == 'o')
    return "OPERATOR";
  else if (c == 'n')
    return "else";
  else if (c == 'p')
    return "for";
  else if (c == 'r')
    return "switch";
  else if (c == 's')
    return "while";
  else if (c == 'w')
    return "+=";
  else if (c == 'x')
    return "-=";
  else if (c == 't')
    return "++";
  else if (c == 'u')
    return "--";
  else if (c == 'v')
    return "==";
  else if (c == '{')
    return "{";
  else if (c == '}')
    return "}";
  else if (c == '(')
    return "(";
  else if (c == ')')
    return ")";
  else if (c == ';')
    return ";";
  else if (c == ':')
    return ":";
  else if (c == '+')
    return "+";
  else if (c == '-')
    return "-";
  else if (c == '*')
    return "*";
  else if (c == '/')
    return "/";
  else if (c == '!')
    return "#";
  else if (c == '>')
    return ">";
  else if (c == '<')
    return "<";
  else if (c == '=')
    return "=";
}

// @brief:读取文法文件
//        存终结符、非终结符、推出epsilon的终结符、映射后的token
//        判断词法分析器的结果是否有有误
void readGrammarFile() {
  vector<string> temp;
  fstream file;
  file.open(GRAMMAR_FILE_PATH);
  char str_file[100];
  while (file.getline(str_file, 100)) {
    string str(str_file);
    temp.push_back(str);
  }
  for (int i = 0; i < temp.size(); i++) {
    VN.insert(temp[i][0]);  //存非终结符
    Grammar g;
    g.left = temp[i][0];
    g.right = temp[i].substr(3, temp[i].size() - 3);
    grammar.push_back(g);
    if (g.right == "$") toEpsilon.insert(g.left);
    // 存终结符
    for (int j = 3; j < temp[i].size(); j++) {
      if ((temp[i][j] < 'A' || temp[i][j] > 'Z') && temp[i][j] != '$')
        VT.insert(temp[i][j]);
    }
  }
  // 补充完toEpsilon的集合
  int last_cnt = -1;
  int VT_toEpsilon_cnt = toEpsilon.size();
  while (VT_toEpsilon_cnt != last_cnt) {
    for (int i = 0; i < grammar.size(); i++) {
      bool flag_isToEpsilon = true;
      for (int j = 0; j < grammar[i].right.size(); j++) {
        if (grammar[i].right[j] == '$' ||
            toEpsilon.find(grammar[i].right[j]) != toEpsilon.end())
          continue;
        else
          flag_isToEpsilon = false;
      }
      if (flag_isToEpsilon) toEpsilon.insert(grammar[i].left);
    }
    last_cnt = VT_toEpsilon_cnt;
    VT_toEpsilon_cnt = toEpsilon.size();
  }
  file.close();
  // 分析词法分析器里是否有错误信息
  file.open(WRONG_FILE_PATH);
  char c = file.get();
  if (!file.eof()) {
    is_wrong = true;
  }
  file.close();
  // 读token序列
  file.open(TOKEN_FILE_PATH);
  while (file.getline(str_file, 100)) {
    string s = str_file;
    string word = "";
    string type_of_word = "";
    int i = 0;
    bool flag = false;
    for (int i = 0; i < s.length(); i++) {
      if (s[i] == ' ') {
        flag = true;
        i += 9;//原token中word和type之间9个空格
      }
      if (!flag)
        word += s[i];
      else
        type_of_word += s[i];
    }
    token += token_from_lex_to_grammar(word, type_of_word);
  }
  file.close();
  // 读一行有多少个token
  file.open(ROW_FILE_PATH);
  int row_count;
  while (file >> row_count) {
    row.push_back(row_count);
  }
  file.close();
}

/* ****************** 语法分析*********************/
// @brief:  求First集（非终结符）
void getFirstSet() {
  int before_sum = -1;
  int next_sum = 0;
  while (next_sum != before_sum) {
    for (int i = 0; i < grammar.size(); i++) {
      string grammar_of_right = grammar[i].right;
      int size = grammar_of_right.size();

      if (size == 1) {
        // A->a 或 A->epsilon：a和epsilon是A的First集的元素
        if (grammar_of_right[0] == '$' || VT.find(grammar_of_right[0]) != VT.end()) {
          FirstSet[grammar[i].left].insert(grammar_of_right[0]);
        } else {
          // A -> B：B的First集 属于 A的First集
          set<char> temp = FirstSet[grammar_of_right[0]];
          FirstSet[grammar[i].left].insert(temp.begin(), temp.end());
        }
      } else {
        for (int j = 0; j < grammar_of_right.size(); j++) {
          if (grammar_of_right[j] == '$')
            continue;
          else if (VT.find(grammar_of_right[j]) != VT.end()) {
            // A->aB..或A->ab....：a是A的First集的元素
            FirstSet[grammar[i].left].insert(grammar_of_right[j]);
            break;
          } else {
            // A->BCX... 或A->BcX..
            set<char> temp = FirstSet[grammar_of_right[j]];
            set<char>::iterator it = temp.find('$');
            if (it != temp.end()) {
              //如果存在B->epsilon
              if (j != grammar_of_right.size() - 1) temp.erase(it);
              FirstSet[grammar[i].left].insert(temp.begin(), temp.end());
            } else {
              //如果不存在B->epsilon
              FirstSet[grammar[i].left].insert(temp.begin(), temp.end());
              break;
            }
          }
        }
      }
    }
    // 每轮统计First的集合中的元素总个数，如果不再改变时收敛
    int sum = 0;
    for (map<char, set<char> >::iterator it = FirstSet.begin();
         it != FirstSet.end(); it++) {
      sum += (*it).second.size();
    }
    before_sum = next_sum;
    next_sum = sum;
  }
}

// 向前搜索符集合
set<char> getForward(char c, set<char> forward) {
  set<char> s;
  if (c == '$') {
    // S->.S, #
    return forward;
  } else if (VT.find(c) != VT.end()) {
    // S->.Sa,#
    s.insert(c);
    return s;
  } else {
    //是否按照toEpsilon来判断无区别
    if (FirstSet[c].find('$') != FirstSet[c].end()) {
      // S->.SB,#      (epsilon belongs to B)
      set<char> temp = FirstSet[c];
      temp.erase(temp.find('$'));
      temp.insert(forward.begin(), forward.end());
      return temp;
    } else {
      // S->.SB,#     (epsilon doesn't belong to B)
      return FirstSet[c];
    }
  }
}

// 求项目集itemSet的闭包
set<Item> e_closure(set<Item> itemSet) {
  if (itemSet.empty()) return itemSet;
  bool flag = true;
  //如果项目集仍有新项目进入或者项目集中仍有向前搜索符在发生变化继续迭代 
  while (flag) {
    flag=false;
    for(set<Item>::iterator it=itemSet.begin();it!=itemSet.end();){
      bool flag1=false;
      Item item=*it;
      //如果圆点不在最后可能继续扩充 
      if(item.position!=item.right.size()){
        //如果圆点后跟的是非终结符可以继续扩充
        if(VN.find(item.right[item.position])!=VN.end()){
          char vn=item.right[item.position];
          for(int i=0;i<grammar.size();i++){
            
            if(grammar[i].left==vn){
              Item temp;
              temp.left=vn;
              temp.right=grammar[i].right;
              temp.position=0;
              for(int j=0;j<grammar.size();j++){
                if(grammar[j].left==temp.left&&grammar[j].right==temp.right){
                  temp.index=j+1;
                }
              }
              // 判断vn后面是否还有符号
              if(item.position==item.right.size()-1){
                //形如S->A@B,# 
                temp.forward=getForward('$',item.forward);
              }else{
                //形如S->A@BC,# 
                temp.forward=getForward(item.right[item.position+1],item.forward);
              }

              set<Item>::iterator itt = itemSet.find(temp);
							//因为STL中find函数是默认用<号进行判等的，所以仍需要判断forward集是否相等 
              if(itt!=itemSet.end()){
                set<char> C;
                //如果itemset中有item，不一定两者forward相等
                set_union((itt->forward).begin(),(itt->forward).end(),temp.forward.begin(),temp.forward.end(),inserter( C , C.begin() ));
                //先求并集，如果并集等于原itemset中item的forward即不发生改变 
                if(C!=itt->forward){
                  flag1=true;
                  set<Item>::iterator itt_backup=itt;
                  flag=true;
                  Item temp_backup=*itt;
                  it++;
                  //先删除再添加，不然会报const出错，STL set中不能直接改变SET中内容，是const类型 
                  itemSet.erase(itt_backup);
                  temp_backup.forward=C;
                  itemSet.insert(temp_backup);
                }
              }else{
                flag=true;
                itemSet.insert(temp);
              }

            }
          }
        }
      }
      if(!flag1) it++;
    }
  }
  return itemSet;
}
// 求GO函数，实现状态转移
set<Item> Go(char c,set<Item> itemset){
  set<Item> itemset_new;
	set<Item>::iterator it;
	for(it = itemset.begin();it != itemset.end();it ++){
		Item item =  *it;
		if(item.position != item.right.size()){
			if(item.right[item.position] == '$'){
				continue;
			}else if(item.right[item.position] == c){
				Item temp;
				temp = item;
				temp.position ++;
				itemset_new.insert(temp);
			}
		}
	}
	//itemset_new = e_closure(itemset_new);
	return itemset_new;
}

// 建立DFA，创建ACTION—GOTO表
void create(char left,string right){
  //初始化I0 
	Item item;
	item.left = left;
	item.right = right;
	item.position = 0;
	item.index = 0;
	item.forward.insert('#');
	Itemset[0].insert(item);
	Itemset[0] = e_closure(Itemset[0]);
	totalNodes = 1;
	//进行状态转移
	queue<set<Item> > q;
	queue<int> q_index; 
	q.push(Itemset[0]);
	q_index.push(0);
	while(!q.empty() && !q_index.empty()){
		set<Item> s = q.front();
		q.pop();
		int index = q_index.front();
		q_index.pop();
		
		set<char>::iterator it;
		set<char> sum;                           //VT和VN 
		set_union(VT.begin(),VT.end(),VN.begin(),VN.end(),inserter( sum , sum.begin() )); 
		for(it = sum.begin();it != sum.end();it++){
			set<Item> s_new = e_closure(Go(*it,s));
			if(!s_new.empty()){
				//showItemset(s_new);
				//如果不空则和已有的项目集进行比较
				bool flag = false;                    				//标志和已有项目集无重复的 
				for(int i = 0;i < totalNodes;i++){
					//重载后的== 
					if(s_new == Itemset[i]){
						AG[index][*it] = i;
						flag = true;
						break;
					}
				}
				//如果没有相同的项目集则totalNodes++ 
				if(!flag){
					AG[index][*it] = totalNodes;
					Itemset[totalNodes++] = s_new;
					q.push(s_new);
					q_index.push(totalNodes-1);
				}
			}else AG[index][*it] = -2;                          //表示没有此过程 
		}		
	}
	//状态转移在建立DFA的时候完成，归约在建立完之后统一遍历完成 
	for(int i = 0;i<totalNodes;i++){
		set<Item> itemset = Itemset[i];
		set<Item>::iterator itt;
		for(itt = itemset.begin();itt != itemset.end();itt++){
			Item item = *itt;
			//如果圆点在最后或者A->.epsilon这种情况就可以归约了 
			if(item.right.find('$') != item.right.npos || item.position == item.right.size()){
				set<char>::iterator it;
				for(it = item.forward.begin();it != item.forward.end();it++){
					if(item.index == 0){
						AG[i][*it] = -1;                       //标识acc 
						//cout << AG[i][*it] << endl;
					} 
					else{
						AG[i][*it] = item.index + 256;                              //用+256区分是S还是r 
						//cout << AG[i][*it] << endl; 
					}
				}
			}
		}
	}
}
/*********************源程序处理****************/
// 展示部分结果

// 扫描词法分析器的token序列进行分析
void scan(string str){
	ofstream output;
	output.open(ANALYSIS_FILE_PATH);
	if(is_wrong){
		cout << "词法分析器有错，请先校正源程序的构词方式再进行语法分析" << endl;
		return;
	}
	str += '#';                                         //补充最后一个# 
	stack<int> state;                                   //状态栈 
	stack<char> symbol;									//符号栈
	//初始化两个栈 
	state.push(0);
	symbol.push('#');
	output << "分析过程：" << endl; 
	output <<std::left<< setw(10) << "步骤" <<std::left<< setw(150) << "状态栈"<<std::left << setw(150) << "符号栈" <<std::left<< setw(200) <<"输入串" <<std::left<< setw(150)<< "ACTION" <<std::left<< setw(150) << "GOTO" << endl;
	int count = 1;                   //步骤数 
	int i = 0;					//输入token的下标 
	output <<std::left<< setw(10) << "1" <<std::left<< setw(150) << "0" <<std::left<< setw(150) << "#"<<std::left << setw(200) << str ; 
	while(1){
		count++;
		int Action = -256;
		int Goto = -256;
		string action = "";
		string goTo = "";
		
		int temp_state = state.top();
		int temp_symbol = symbol.top();
		if(AG[temp_state][str[i]] == -1){
			action = "acc";
			output << setw(150) << action << setw(150) << goTo << endl;;
			break; 
		}
		else if(AG[temp_state][str[i]] == -2){
			/*
			if(i < str.length() - 1) output << "在第"<< i+1 << "个token发生错误" << endl;
			else output << "在最后一个token发生错误" << endl;*/
			//cout << "第" << i+1 << "个token出错" << endl;
			int temp_i = i+1;
			if(temp_i < str.length() - 1){
				for(int j = 0;j<row.size();j++){
					if(temp_i > row[j]) {
						//cout << "减前:" << temp_i << endl;
						temp_i -= row[j];
						//cout << "减去" << j+1 <<"行" << row[j] << "后:" << temp_i << endl; 
					}
					else{
						cout << "在第" << j+1 << "行,第" << temp_i << "个token:"<< token_from_grammar_to_lex(str[i]) << " 发生错误" << endl;
						break;
					}	 
				}	
			}else{
				cout << "最后一个token发生错误" << endl;
			} 
			cout << "未能找到期待的token:" << endl;
			set<char>::iterator it;
			for(it = VT.begin();it != VT.end();it++){
				if(AG[temp_state][*it] != -2) cout << token_from_grammar_to_lex(*it) << " ";
			} 
			cout << endl;
			return ;
		}else if(AG[temp_state][str[i]] < 256){
			Action = AG[temp_state][str[i]];
			state.push(Action);
			symbol.push(str[i]);
			i++;
			action = "S" + to_string(Action);
		}else{
			Action = AG[temp_state][str[i]] - 256;
			//准备进行归约
			char left = grammar[Action-1].left;
			int right_size = grammar[Action-1].right.size();
			//如果右边是epsilon，就不需要退栈只需要将左部加入到符号栈即可 
			if(grammar[Action-1].right != "$"){
				for(int j = 0;j<right_size;j++){
					state.pop();
					symbol.pop();
				}	 	
			}
			symbol.push(left);
			temp_state = state.top();
			Goto = AG[temp_state][left];
			if(Goto > 0 && Goto <= 256){
				state.push(Goto);
				goTo = to_string(Goto);
			}
			action = "r" + to_string(Action);
		}
		
		
		//打印分析过程
		
		string state_str = "";
		string symbol_str = "";
		vector<int> state_v;
		vector<char> symbol_v;
		stack<int> state_temp = state;
		stack<char> symbol_temp = symbol;
		while(!state_temp.empty()){
			state_v.push_back(state_temp.top());
			state_temp.pop();
		}
		while(!symbol_temp.empty()){
			symbol_v.push_back(symbol_temp.top());
			symbol_temp.pop();
		}
		for(int i = state_v.size()-1;i>=0;i--){
			state_str += to_string(state_v[i]);
			if(i) state_str += " ";
		}
		for(int i = symbol_v.size()-1;i>=0;i--){
			symbol_str += symbol_v[i];
		}
		string input = str.substr(i,str.length()-i);
		output <<std::left<<  setw(150) << action <<std::left<< setw(150) << goTo<<endl;	
		output << setw(10) << count <<std::left<< setw(150) << state_str <<std::left<< setw(150) << symbol_str <<std::left << setw(200) << input ;
	} 
	cout << "正确" << endl;
}

//展示部分结果 
void show(){
	
	cout << "Grammar:" << endl;
	for(int i = 0;i< grammar.size();i++){
		cout << grammar[i].left << "->" << grammar[i].right << endl; 
	}
	
	cout << "toEpsilon:" << endl;
	set<char>::iterator it;
	for(it = toEpsilon.begin();it != toEpsilon.end();it++){
		cout << *it << " ";
	}
	cout << endl << "VT:" << endl;
	for(it = VT.begin();it != VT.end();it++){
		cout << *it << " ";
	}
	cout << endl << "VN:" << endl;
	for(it = VN.begin();it != VN.end();it++){
		cout << *it << " " ;
	}
	cout << endl << "FirstVT:" << endl;
	map<char,set<char> >::iterator it_map;
	for(it_map = FirstSet.begin();it_map != FirstSet.end();it_map++){
		cout << (*it_map).first << ":" ;
		set<char> temp = (*it_map).second;
		for(it = temp.begin();it != temp.end();it++){
			cout << *it << " ";
		}
		cout << endl;
	}
	cout << endl;

	for(int i = 0;i<totalNodes;i++){
		cout << "I" << i << ":" << endl;
		set<Item>::iterator it_set;
		set<Item> itemset = Itemset[i];
		for(it_set = itemset.begin();it_set != itemset.end();it_set ++){
			Item item = *it_set;
			cout << item.left << "->";
			for(int j = 0;j<item.right.size();j++){
				if(j == item.position) cout << '@';
				cout << item.right[j];
			}
			if(item.position == item.right.size()) cout << '@';
			cout << ", ";
			int count = 0;
			for(it = item.forward.begin();it != item.forward.end();it++){
				count ++;
				if(count != item.forward.size()) cout << *it << "/";
				else cout << *it;
			}
			cout << "    index:" << item.index << endl;
		}
		cout << endl;
	}
	
	ofstream output;
	output.open(AG_PATH);
	output << "ACTION-GOTO表:" << endl;
	output << setw(7) << "";
	VT.insert('#');
	for(it = VT.begin();it != VT.end();it++){
		output <<setw(7)<< *it ;
	}
	for(it = VN.begin();it != VN.end();it++){
		output << setw(7)<< *it ;
	}
	output << endl;
	for(int i = 0;i<totalNodes;i++){
		output << setw(7)<< i ;	
		for(it  = VT.begin();it != VT.end();it++){
			int temp = AG[i][*it];
			if(temp == -1){
				output << setw(7)<< "acc" ;
			}else if(temp < 256 && temp > 0){
				output <<setw(7)<< "S"+ to_string(temp) ;
			}else if(temp >= 256){
				output <<setw(7)<< "r" +to_string(temp - 256)   ;
			}else{
				output <<setw(7) << "";
			}
		}
		for(it  = VN.begin();it != VN.end();it++){
			int temp = AG[i][*it];
			if(temp == -1){
				output << setw(7)<< "acc" ;
			}else if(temp < 256 && temp > 0){
				output <<setw(7)<< to_string(temp) ;
			}else if(temp >= 256){
				output <<setw(7)<< "r" +to_string(temp - 256)   ;
			}else{
				output <<setw(7) << "";
			}
		}
		output << endl;
	}
	VT.erase('#');
	output.close();
}