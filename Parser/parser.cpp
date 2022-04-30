#include "parser.h"

#include "path.h"

vector<Grammar>grammar;  //����ķ�,�ķ��в��ó��ֿ���Ҫ��Ȼ������ڴ����Խ������
set<Item> Itemset[1000];  //���LR(1)����Ŀ��
int totalNodes;           //���յ�״̬���ĸ���
set<char> VT;  //����ķ��е��ս��,���в�����epsilon��epsilon��������
set<char> VN;                   //����ķ��еķ��ս��
set<char> toEpsilon;            //����ܹ��Ƶ�epsilon�ķ��ս��
map<char, set<char> > FirstSet;  //���ķ��еķ��ս����Ӧ��First��

bool is_wrong = false;  //�жϴʷ��������Ƿ��д�
string token = "";      //��ŴӴʷ��������������token����
vector<int> row;        //���ÿ���ж��ٸ�token

int AG[300][300];                     //Action-GOTO��
/******************* Ԥ����  **********************/
// ���ʷ��������õ���tokenת��Ϊ�﷨�����������token

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

// ��ת��
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

// @brief:��ȡ�ķ��ļ�
//        ���ս�������ս�����Ƴ�epsilon���ս����ӳ����token
//        �жϴʷ��������Ľ���Ƿ�������
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
    VN.insert(temp[i][0]);  //����ս��
    Grammar g;
    g.left = temp[i][0];
    g.right = temp[i].substr(3, temp[i].size() - 3);
    grammar.push_back(g);
    if (g.right == "$") toEpsilon.insert(g.left);
    // ���ս��
    for (int j = 3; j < temp[i].size(); j++) {
      if ((temp[i][j] < 'A' || temp[i][j] > 'Z') && temp[i][j] != '$')
        VT.insert(temp[i][j]);
    }
  }
  // ������toEpsilon�ļ���
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
  // �����ʷ����������Ƿ��д�����Ϣ
  file.open(WRONG_FILE_PATH);
  char c = file.get();
  if (!file.eof()) {
    is_wrong = true;
  }
  file.close();
  // ��token����
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
        i += 9;//ԭtoken��word��type֮��9���ո�
      }
      if (!flag)
        word += s[i];
      else
        type_of_word += s[i];
    }
    token += token_from_lex_to_grammar(word, type_of_word);
  }
  file.close();
  // ��һ���ж��ٸ�token
  file.open(ROW_FILE_PATH);
  int row_count;
  while (file >> row_count) {
    row.push_back(row_count);
  }
  file.close();
}

/* ****************** �﷨����*********************/
// @brief:  ��First�������ս����
void getFirstSet() {
  int before_sum = -1;
  int next_sum = 0;
  while (next_sum != before_sum) {
    for (int i = 0; i < grammar.size(); i++) {
      string grammar_of_right = grammar[i].right;
      int size = grammar_of_right.size();

      if (size == 1) {
        // A->a �� A->epsilon��a��epsilon��A��First����Ԫ��
        if (grammar_of_right[0] == '$' || VT.find(grammar_of_right[0]) != VT.end()) {
          FirstSet[grammar[i].left].insert(grammar_of_right[0]);
        } else {
          // A -> B��B��First�� ���� A��First��
          set<char> temp = FirstSet[grammar_of_right[0]];
          FirstSet[grammar[i].left].insert(temp.begin(), temp.end());
        }
      } else {
        for (int j = 0; j < grammar_of_right.size(); j++) {
          if (grammar_of_right[j] == '$')
            continue;
          else if (VT.find(grammar_of_right[j]) != VT.end()) {
            // A->aB..��A->ab....��a��A��First����Ԫ��
            FirstSet[grammar[i].left].insert(grammar_of_right[j]);
            break;
          } else {
            // A->BCX... ��A->BcX..
            set<char> temp = FirstSet[grammar_of_right[j]];
            set<char>::iterator it = temp.find('$');
            if (it != temp.end()) {
              //�������B->epsilon
              if (j != grammar_of_right.size() - 1) temp.erase(it);
              FirstSet[grammar[i].left].insert(temp.begin(), temp.end());
            } else {
              //���������B->epsilon
              FirstSet[grammar[i].left].insert(temp.begin(), temp.end());
              break;
            }
          }
        }
      }
    }
    // ÿ��ͳ��First�ļ����е�Ԫ���ܸ�����������ٸı�ʱ����
    int sum = 0;
    for (map<char, set<char> >::iterator it = FirstSet.begin();
         it != FirstSet.end(); it++) {
      sum += (*it).second.size();
    }
    before_sum = next_sum;
    next_sum = sum;
  }
}

// ��ǰ����������
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
    //�Ƿ���toEpsilon���ж�������
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

// ����Ŀ��itemSet�ıհ�
set<Item> e_closure(set<Item> itemSet) {
  if (itemSet.empty()) return itemSet;
  bool flag = true;
  //�����Ŀ����������Ŀ���������Ŀ����������ǰ�������ڷ����仯�������� 
  while (flag) {
    flag=false;
    for(set<Item>::iterator it=itemSet.begin();it!=itemSet.end();){
      bool flag1=false;
      Item item=*it;
      //���Բ�㲻�������ܼ������� 
      if(item.position!=item.right.size()){
        //���Բ�������Ƿ��ս�����Լ�������
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
              // �ж�vn�����Ƿ��з���
              if(item.position==item.right.size()-1){
                //����S->A@B,# 
                temp.forward=getForward('$',item.forward);
              }else{
                //����S->A@BC,# 
                temp.forward=getForward(item.right[item.position+1],item.forward);
              }

              set<Item>::iterator itt = itemSet.find(temp);
							//��ΪSTL��find������Ĭ����<�Ž����еȵģ���������Ҫ�ж�forward���Ƿ���� 
              if(itt!=itemSet.end()){
                set<char> C;
                //���itemset����item����һ������forward���
                set_union((itt->forward).begin(),(itt->forward).end(),temp.forward.begin(),temp.forward.end(),inserter( C , C.begin() ));
                //���󲢼��������������ԭitemset��item��forward���������ı� 
                if(C!=itt->forward){
                  flag1=true;
                  set<Item>::iterator itt_backup=itt;
                  flag=true;
                  Item temp_backup=*itt;
                  it++;
                  //��ɾ������ӣ���Ȼ�ᱨconst����STL set�в���ֱ�Ӹı�SET�����ݣ���const���� 
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
// ��GO������ʵ��״̬ת��
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

// ����DFA������ACTION��GOTO��
void create(char left,string right){
  //��ʼ��I0 
	Item item;
	item.left = left;
	item.right = right;
	item.position = 0;
	item.index = 0;
	item.forward.insert('#');
	Itemset[0].insert(item);
	Itemset[0] = e_closure(Itemset[0]);
	totalNodes = 1;
	//����״̬ת��
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
		set<char> sum;                           //VT��VN 
		set_union(VT.begin(),VT.end(),VN.begin(),VN.end(),inserter( sum , sum.begin() )); 
		for(it = sum.begin();it != sum.end();it++){
			set<Item> s_new = e_closure(Go(*it,s));
			if(!s_new.empty()){
				//showItemset(s_new);
				//�������������е���Ŀ�����бȽ�
				bool flag = false;                    				//��־��������Ŀ�����ظ��� 
				for(int i = 0;i < totalNodes;i++){
					//���غ��== 
					if(s_new == Itemset[i]){
						AG[index][*it] = i;
						flag = true;
						break;
					}
				}
				//���û����ͬ����Ŀ����totalNodes++ 
				if(!flag){
					AG[index][*it] = totalNodes;
					Itemset[totalNodes++] = s_new;
					q.push(s_new);
					q_index.push(totalNodes-1);
				}
			}else AG[index][*it] = -2;                          //��ʾû�д˹��� 
		}		
	}
	//״̬ת���ڽ���DFA��ʱ����ɣ���Լ�ڽ�����֮��ͳһ������� 
	for(int i = 0;i<totalNodes;i++){
		set<Item> itemset = Itemset[i];
		set<Item>::iterator itt;
		for(itt = itemset.begin();itt != itemset.end();itt++){
			Item item = *itt;
			//���Բ����������A->.epsilon��������Ϳ��Թ�Լ�� 
			if(item.right.find('$') != item.right.npos || item.position == item.right.size()){
				set<char>::iterator it;
				for(it = item.forward.begin();it != item.forward.end();it++){
					if(item.index == 0){
						AG[i][*it] = -1;                       //��ʶacc 
						//cout << AG[i][*it] << endl;
					} 
					else{
						AG[i][*it] = item.index + 256;                              //��+256������S����r 
						//cout << AG[i][*it] << endl; 
					}
				}
			}
		}
	}
}
/*********************Դ������****************/
// չʾ���ֽ��

// ɨ��ʷ���������token���н��з���
void scan(string str){
	ofstream output;
	output.open(ANALYSIS_FILE_PATH);
	if(is_wrong){
		cout << "�ʷ��������д�����У��Դ����Ĺ��ʷ�ʽ�ٽ����﷨����" << endl;
		return;
	}
	str += '#';                                         //�������һ��# 
	stack<int> state;                                   //״̬ջ 
	stack<char> symbol;									//����ջ
	//��ʼ������ջ 
	state.push(0);
	symbol.push('#');
	output << "�������̣�" << endl; 
	output <<std::left<< setw(10) << "����" <<std::left<< setw(150) << "״̬ջ"<<std::left << setw(150) << "����ջ" <<std::left<< setw(200) <<"���봮" <<std::left<< setw(150)<< "ACTION" <<std::left<< setw(150) << "GOTO" << endl;
	int count = 1;                   //������ 
	int i = 0;					//����token���±� 
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
			if(i < str.length() - 1) output << "�ڵ�"<< i+1 << "��token��������" << endl;
			else output << "�����һ��token��������" << endl;*/
			//cout << "��" << i+1 << "��token����" << endl;
			int temp_i = i+1;
			if(temp_i < str.length() - 1){
				for(int j = 0;j<row.size();j++){
					if(temp_i > row[j]) {
						//cout << "��ǰ:" << temp_i << endl;
						temp_i -= row[j];
						//cout << "��ȥ" << j+1 <<"��" << row[j] << "��:" << temp_i << endl; 
					}
					else{
						cout << "�ڵ�" << j+1 << "��,��" << temp_i << "��token:"<< token_from_grammar_to_lex(str[i]) << " ��������" << endl;
						break;
					}	 
				}	
			}else{
				cout << "���һ��token��������" << endl;
			} 
			cout << "δ���ҵ��ڴ���token:" << endl;
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
			//׼�����й�Լ
			char left = grammar[Action-1].left;
			int right_size = grammar[Action-1].right.size();
			//����ұ���epsilon���Ͳ���Ҫ��ջֻ��Ҫ���󲿼��뵽����ջ���� 
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
		
		
		//��ӡ��������
		
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
	cout << "��ȷ" << endl;
}

//չʾ���ֽ�� 
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
	output << "ACTION-GOTO��:" << endl;
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