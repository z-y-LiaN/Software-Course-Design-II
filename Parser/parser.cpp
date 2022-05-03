#include "parser.h"

#include "path.h"

vector<Grammar>grammar; 
set<Item> Itemset[1000];
int itemSet_counter; 
set<char> VT; 
set<char> VN;
set<char> toEpsilon; 
map<char, set<char> > FirstSet;
bool lexical_wrong = false; 
string token = "";
vector<int> row; 
int ActionGoto[300][300]; 


/**
 * @brief  将词法分析器得到的token（串），映射为语法分析器所需的token（字符），方便语法分析
 * @param  string-str 词法分析的token
 * @param  string-type 词法分析的token类型
 * @return  类型为type的token的映射单词 符
 */
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

/**
 * @brief  将语法分析器的token（字符），映射为词法分析器的token（串）
 * @param char-c 语法分析器的token
 * @return   该语法分析器的token在 对应的词法分析中的token
 */
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

     
/**
 *  @brief   读取给定的文法文件
 *  @details 存非终结符、推出epsilon的非终结符、串到字符）映射后的token、判断词法分析器的结果是否有误
 */
void readGrammarFile() {
  vector<string> allGrammars;
  fstream file;
  file.open(GRAMMAR_FILE_PATH);
  char str_file[100];
  while (file.getline(str_file, 100)) {
    string str(str_file);
    allGrammars.push_back(str);
  }
  file.close();
  for (int i = 0; i < allGrammars.size(); i++) {
    VN.insert(allGrammars[i][0]);
    Grammar g;
    g.left = allGrammars[i][0];
    g.right = allGrammars[i].substr(3, allGrammars[i].size() - 3);
    grammar.push_back(g);
    if (g.right == "$") toEpsilon.insert(g.left);
    for (int j = 3; j < allGrammars[i].size(); j++) {
      if ((allGrammars[i][j] < 'A' || allGrammars[i][j] > 'Z') && allGrammars[i][j] != '$')
        VT.insert(allGrammars[i][j]);
    }
  }
  
  int last_cnt = -1;
  int VN_toEpsilon_cnt = toEpsilon.size();
  while (VN_toEpsilon_cnt != last_cnt) {
    for (int i = 0; i < grammar.size(); i++) {
      bool flag_isToEpsilon = true;
      for (int j = 0; j < grammar[i].right.size(); j++) {
        if (grammar[i].right[j] == '$' || toEpsilon.find(grammar[i].right[j]) != toEpsilon.end())
          continue;
        else
          flag_isToEpsilon = false;
      }
      if (flag_isToEpsilon) toEpsilon.insert(grammar[i].left);
    }
    last_cnt = VN_toEpsilon_cnt;
    VN_toEpsilon_cnt = toEpsilon.size();
  }
  // 判断词法分析器里是否有错误信息
  file.open(WRONG_FILE_PATH);
  char c = file.get();
  if (!file.eof()) {
    lexical_wrong = true;
  }
  file.close();
  // 读token表
  file.open(TOKEN_FILE_PATH);
  while (file.getline(str_file, 100)) {
    string token_line = str_file;
    string word = "";
    string type_of_word = "";
    bool flag_meetType = false;
    for (int i = 0; i < token_line.length(); i++) {
      if (token_line[i] == ' ') {
        flag_meetType = true;
        i += 9; 
      }
      if (!flag_meetType)
        word += token_line[i];
      else
        type_of_word += token_line[i];
    }
    token += token_from_lex_to_grammar(word, type_of_word);//映射
  }
  file.close();
  file.open(ROW_FILE_PATH);
  int row_count;
  while (file >> row_count) {
    row.push_back(row_count);
  }
  file.close();
}

/**
 *  @brief   求所有非终结符的First集，每轮统计FirstSet的集合中的所有元素总个数，如果不再改变时收敛
 *           (1)A->a 或 A->epsilon：a和epsilon是A的First集的元素
 *           (2)A->B：B的First集属于A的First集
 *           (3)A->aB..或A->ab....：a是A的First集的元素
 *           (4)A->XBCX...，分情况 存在B->epsilon和 不存在B->epsilon
 */
void getFirstSet() {
  int before_sum = -1;
  int next_sum = 0;
  while (next_sum != before_sum) {
    for (int i = 0; i < grammar.size(); i++) {
      string grammar_right = grammar[i].right;
      int right_size = grammar_right.size();

      if (right_size == 1) {
        if (grammar_right[0] == '$' ||
            VT.find(grammar_right[0]) != VT.end()) {
          FirstSet[grammar[i].left].insert(grammar_right[0]);
        } else {
          set<char> temp = FirstSet[grammar_right[0]];
          FirstSet[grammar[i].left].insert(temp.begin(), temp.end());
        }
      } else {
        for (int j = 0; j < grammar_right.size(); j++) {
          if (grammar_right[j] == '$')
            continue;
          else if (VT.find(grammar_right[j]) != VT.end()) {
            FirstSet[grammar[i].left].insert(grammar_right[j]);
            break;
          } else {
            // 
            set<char> temp = FirstSet[grammar_right[j]];
            set<char>::iterator it = temp.find('$');
            if (it != temp.end()) {
              if (j != grammar_right.size() - 1) temp.erase(it);
              FirstSet[grammar[i].left].insert(temp.begin(), temp.end());
            } else {
              FirstSet[grammar[i].left].insert(temp.begin(), temp.end());
              break;
            }
          }
        }
      }
    }
    int sum = 0;
    for (map<char, set<char> >::iterator it = FirstSet.begin();
         it != FirstSet.end(); it++) {
      sum += (*it).second.size();
    }
    before_sum = next_sum;
    next_sum = sum;
  }
}

/**
 *  @brief   求向前搜索符集合
 *  @param char-c c表示是形如A->a·B中B后的符号
 *  @param set<char>-forward 当前向前搜索集
 *  @return 返回向前搜索符集合
 */
set<char> getForward(char c, set<char> forward) {
  set<char> s;
  if (c == '$') { //A->α·B,forward
    return forward;
  } else if (VT.find(c) != VT.end()) { //A->α·Bc,forward ,c是终结符
    s.insert(c);  return s;
  } else {//A->α·BC,forward ,C是非终结符
    if (FirstSet[c].find('$') != FirstSet[c].end()) { //存在C->ε
      set<char> temp = FirstSet[c];
      temp.erase(temp.find('$'));
      temp.insert(forward.begin(), forward.end());
      return temp;
    } else {//不存在C->ε
      return FirstSet[c];
    }
  }
}

/**
 * @brief 求一个LR(1)项目集itemSet的闭包，如果项目集仍有新项目进入 则继续迭代，直至没有新项目进入
 * @param set<Item> itemset ：一个项目集
 * @return 该项目集的闭包
 */
set<Item> getClosure(set<Item> itemSet) {
   int XX=0,YY=0;
  if (itemSet.empty()) return itemSet;
  bool isChanging = true;
  while (isChanging) {
    isChanging = false;
    for (set<Item>::iterator it = itemSet.begin(); it != itemSet.end();it++) {
      Item item = *it;
      if (item.position != item.right.size()) {
        if (VN.find(item.right[item.position]) != VN.end()) {  //当前item为A-> α·Bβ，加入所有B->·γ
          char vn = item.right[item.position];
          for (int i = 0; i < grammar.size(); i++) {
            if (grammar[i].left == vn) {
              Item newItemAdded;
              newItemAdded.left = vn;
              newItemAdded.right = grammar[i].right;
              newItemAdded.position = 0;
              newItemAdded.index = i + 1;
              // for (int j = 0; j < grammar.size(); j++) {
              //   if (grammar[j].left == newItemAdded.left &&
              //       grammar[j].right == newItemAdded.right) {
              //     newItemAdded.index = j + 1;
              //   }
              // }
              if (item.position == item.right.size() - 1) {  // A-> α·B
                newItemAdded.forward = getForward('$', item.forward);
              } else {  // A-> α·Bβ
                newItemAdded.forward = getForward(item.right[item.position + 1], item.forward);
              }                
              if(itemSet.find(newItemAdded)==itemSet.end()){
                isChanging = true;
                itemSet.insert(newItemAdded);
              }       
              // set<Item>::iterator itt = itemSet.find(newItemAdded);    
              // if (itt != itemSet.end()) {
              //   set<char> C;
              //   set_union((itt->forward).begin(), (itt->forward).end(),newItemAdded.forward.begin(),newItemAdded.forward.end(), inserter(C, C.begin()));
              //   if (C != itt->forward) {
              //     XX++;
              //     flag1 = true;
              //     set<Item>::iterator itt_backup = itt;
              //     isChanging = true;
              //     Item temp_backup = *itt;
              //     it++;
              //     itemSet.erase(itt_backup);
              //     temp_backup.forward = C;
              //     itemSet.insert(temp_backup);
              //   }
              // } else {         
              //   YY++;     
              //   isChanging = true;
              //   itemSet.insert(newItemAdded);
              // }
            }
          }
        }
      }
    }
  }
  return itemSet;
}

/**
 *  @brief   求核的集合，当前itemSet项目集，输入文法符号c 转移
 *  @details 核：新状态的初始项目，即圆点移动后的项目
 *  @param   char-c 文法符号
 *  @param   set<Item>-itemSet  （转移前）LR(1)的项目集
 *  @return  由itemSet经c转移过来的核的集合
 */
set<Item> getKernel(char c, set<Item> itemSet) {
  set<Item> itemSet_Target;
  for (set<Item>::iterator it = itemSet.begin(); it != itemSet.end(); it++) {
    Item item = *it;
    if (item.position != item.right.size()) {
      if (item.right[item.position] == '$') {
        continue;
      } else if (item.right[item.position] == c) {
        Item temp;
        temp = item;
        temp.position++;
        itemSet_Target.insert(temp);
      }
    }
  }
  return itemSet_Target;
}

/**
 *   @brief   建立DFA项目集族以及Action-Goto表
 *   @details 初始化项目集I0，然后进行状态转移，求所有项目集并且状态转移 采用宽搜的方式,遍历所有终结符和非终结符
 *            ，归约在建立完之后统一遍历完成
 *   @param   char-left 增广文法的左部非终结符，Z
 *   @param   string-right 原文法的起始非终结符,S， Z->·S,#
 *   @return  void
 */
void create(char left, string right) {
  
  Item initItem;
  initItem.left = left;
  initItem.right = right;
  initItem.position = 0;
  initItem.index = 0;
  initItem.forward.insert('#');
  Itemset[0].insert(initItem);
  Itemset[0] = getClosure(Itemset[0]); // 初始项目集T0

  itemSet_counter = 1;
  queue<set<Item> > itemSet_queue;
  queue<int> q_index;
  itemSet_queue.push(Itemset[0]);
  q_index.push(0);

  while (!itemSet_queue.empty() && !q_index.empty()) {

    set<Item> itemSet_source = itemSet_queue.front();  
    itemSet_queue.pop();
    int itemSet_index = q_index.front();  
    q_index.pop();

    set<char> all_VN_VT;  
    set_union(VT.begin(), VT.end(), VN.begin(), VN.end(),inserter(all_VN_VT, all_VN_VT.begin()));

    for (set<char>::iterator it = all_VN_VT.begin(); it != all_VN_VT.end();it++) {
      set<Item> itemSet_target = getClosure(getKernel(*it, itemSet_source));
      if (!itemSet_target.empty()) {
        bool alreadyExits = false;
        for (int i = 0; i < itemSet_counter; i++) {
          if (itemSet_target == Itemset[i]) {  //重载后的==
            ActionGoto[itemSet_index][*it] = i;
            alreadyExits = true;
            break;
          }
        }
        if (!alreadyExits) {
          ActionGoto[itemSet_index][*it] = itemSet_counter;
          Itemset[itemSet_counter++] = itemSet_target;
          itemSet_queue.push(itemSet_target);
          q_index.push(itemSet_counter - 1);
        }
      } else
        ActionGoto[itemSet_index][*it] = -2;  //-2 标识没有此过程
    }
  }
  //归约or接受
  for (int i = 0; i < itemSet_counter; i++) {
    set<Item> itemset = Itemset[i];
    for (set<Item>::iterator itt = itemset.begin(); itt != itemset.end();itt++) {
      Item item = *itt;
      if (item.right.find('$') != item.right.npos || item.position == item.right.size()) {
        for (set<char>::iterator it = item.forward.begin();it != item.forward.end(); it++) {
          if (item.index == 0) {
            ActionGoto[i][*it] = -1;  //接受，标识acc
          } else {
            ActionGoto[i][*it] = item.index + 256;  //用+256区分是S还是r
            // cout << AG[i][*it] << endl;
          }
        }
      }
    }
  }
}

/**
 *  @brief  扫描词法分析器的token序列进行LR(1)语法分析
 *          （1）检查词法分析器是否报错
 *
 *  @param  string-token_str 之前词法分析器获得的token序列
 */
void scanSourceToken(string token_str) {
  ofstream outputAnalysis;
  outputAnalysis.open(ANALYSIS_FILE_PATH);
  if (lexical_wrong) {
    cout << "ERROR_INFO: 词法分析有误，请先检查源程序!"<< endl;
    return;
  }
  token_str += '#'; 
  stack<int> stateStack;
  stack<char> symbolStack;
  stateStack.push(0);
  symbolStack.push('#');
  outputAnalysis << "Analysis Proccess Below:" << endl;
  outputAnalysis << std::left << setw(10) << "step" << std::left << setw(150)
         << "stateStack" << std::left << setw(150) << "symbolStack" << std::left
         << setw(200) << "inputString" << std::left << setw(150) << "ACTION"
         << std::left << setw(150) << "GOTO" << endl;
  int step_counter = 1;
  int token_index = 0;
  outputAnalysis << std::left << setw(10) << "1" << std::left << setw(150) << "0"
         << std::left << setw(150) << "#" << std::left << setw(200)
         << token_str;
  while (1) {
    step_counter++;
    int Action = -256;
    int Goto = -256;
    string action = "";
    string goTo = "";

    int temp_state = stateStack.top();
    int temp_symbol = symbolStack.top();
    if (ActionGoto[temp_state][token_str[token_index]] == -1) {
      action = "acc";
      outputAnalysis << setw(150) << action << setw(150) << goTo << endl;
      break;
    } else if (ActionGoto[temp_state][token_str[token_index]] == -2) {
      int temp_index = token_index + 1;
      if (temp_index < token_str.length() - 1) {
        for (int j = 0; j < row.size(); j++) {
          if (temp_index > row[j]) {
            temp_index -= row[j];
          } else {
            cout << "在第" << j + 1 << "行,第" << temp_index
                 << "个token:" << token_from_grammar_to_lex(token_str[token_index])
                 << " 发生错误" << endl;
            break;
          }
        }
      } else {
        cout << "最后一个token发生错误" << endl;
      }
      cout << "原因是: 未能找到下列所期待的token:" << endl;
      for (set<char>::iterator it = VT.begin(); it != VT.end(); it++) {
        if (ActionGoto[temp_state][*it] != -2)
          cout << token_from_grammar_to_lex(*it) << " ";
      }
      cout << endl;
      return;
    } else if (ActionGoto[temp_state][token_str[token_index]] < 256) {//移进
      Action = ActionGoto[temp_state][token_str[token_index]];
      stateStack.push(Action);
      symbolStack.push(token_str[token_index]);
      token_index++;
      action = "S" + to_string(Action);
    } else {
      Action = ActionGoto[temp_state][token_str[token_index]] - 256;//归约
      char left = grammar[Action - 1].left;
      int right_size = grammar[Action - 1].right.size();
      //如果右边是epsilon，就不需要退栈只需要将左部加入到符号栈即可
      if (grammar[Action - 1].right != "$") {
        for (int j = 0; j < right_size; j++) {
          stateStack.pop();
          symbolStack.pop();
        }
      }
      symbolStack.push(left);
      temp_state = stateStack.top();
      Goto = ActionGoto[temp_state][left];
      if (Goto > 0 && Goto <= 256) {
        stateStack.push(Goto);
        goTo = to_string(Goto);
      }
      action = "r" + to_string(Action);
    }

    //打印分析过程
    string state_str = "";
    string symbol_str = "";
    vector<int> state_v;
    vector<char> symbol_v;
    stack<int> state_temp = stateStack;
    stack<char> symbol_temp = symbolStack;
    while (!state_temp.empty()) {
      state_v.push_back(state_temp.top());
      state_temp.pop();
    }
    while (!symbol_temp.empty()) {
      symbol_v.push_back(symbol_temp.top());
      symbol_temp.pop();
    }
    for (int i = state_v.size() - 1; i >= 0; i--) {
      state_str += to_string(state_v[i]);
      if (i) state_str += " ";
    }
    for (int i = symbol_v.size() - 1; i >= 0; i--) {
      symbol_str += symbol_v[i];
    }
    string input = token_str.substr(token_index, token_str.length() - token_index);
    outputAnalysis << std::left << setw(150) << action << std::left << setw(150) << goTo
           << endl;
    outputAnalysis << setw(10) << step_counter << std::left << setw(150) << state_str
           << std::left << setw(150) << symbol_str << std::left << setw(200)
           << input;
  }
  cout << "正确" << endl;
}

/**
 * @brief 展示部分结果
 */
void showProccessing() {
  cout << "Grammar:" << endl;
  for (int i = 0; i < grammar.size(); i++) {
    cout << grammar[i].left << "->" << grammar[i].right << endl;
  }

  cout << "toEpsilon:" << endl;
  set<char>::iterator it;
  for (it = toEpsilon.begin(); it != toEpsilon.end(); it++) {
    cout << *it << " ";
  }
  cout << endl << "VT:" << endl;
  for (it = VT.begin(); it != VT.end(); it++) {
    cout << *it << " ";
  }
  cout << endl << "VN:" << endl;
  for (it = VN.begin(); it != VN.end(); it++) {
    cout << *it << " ";
  }
  cout << endl << "FirstVT:" << endl;
  map<char, set<char> >::iterator it_map;
  for (it_map = FirstSet.begin(); it_map != FirstSet.end(); it_map++) {
    cout << (*it_map).first << ":";
    set<char> temp = (*it_map).second;
    for (it = temp.begin(); it != temp.end(); it++) {
      cout << *it << " ";
    }
    cout << endl;
  }
  cout << endl;
  ofstream outputItemSet;
  outputItemSet.open(ITEMSET_PATH);
  for (int i = 0; i < itemSet_counter; i++) {
    cout << "ItemSet-" << i << ":" << endl; 
    outputItemSet <<"ItemSet-" << i << ":" << endl;
    set<Item>::iterator it_set;
    set<Item> itemset = Itemset[i];
    for (it_set = itemset.begin(); it_set != itemset.end(); it_set++) {
      Item item = *it_set;
      cout << item.left << "->";  
      outputItemSet << item.left << "->";
      for (int j = 0; j < item.right.size(); j++) {
        if (j == item.position) cout << '@';
        cout << item.right[j];
        outputItemSet << item.right[j];
      }
      if (item.position == item.right.size()) cout << '@';
      cout << ", ";
      outputItemSet << ", ";
      int count = 0;
      for (it = item.forward.begin(); it != item.forward.end(); it++) {
        count++;
        if (count != item.forward.size()){
            cout << *it << "/";
            outputItemSet << *it << "/";
        }
        else{
            cout << *it;
            outputItemSet << *it;
        }
          
      }
      cout<<"     point position:"<<item.position;
      outputItemSet<<"     point position:"<<item.position;
      cout << "    index:" << item.index << endl;
      outputItemSet << "    index:" << item.index << endl;
    }
    cout << endl;
    outputItemSet << endl;
  }
  outputItemSet.close();

  ofstream output;
  output.open(AG_PATH);
  output << "ACTION-GOTO Table:" << endl;
  output << setw(7) << "";
  VT.insert('#');
  for (it = VT.begin(); it != VT.end(); it++) {
    output << setw(7) << *it;
  }
  for (it = VN.begin(); it != VN.end(); it++) {
    output << setw(7) << *it;
  }
  output << endl;

  for (int i = 0; i < itemSet_counter; i++) {
    output << setw(7) << i;
    for (it = VT.begin(); it != VT.end(); it++) {
      int temp = ActionGoto[i][*it];
      if (temp == -1) {
        output << setw(7) << "acc";
      } else if (temp < 256 && temp > 0) {
        output << setw(7) << "S" + to_string(temp);
      } else if (temp >= 256) {
        output << setw(7) << "r" + to_string(temp - 256);
      } else {
        output << setw(7) << "";
      }
    }
    for (it = VN.begin(); it != VN.end(); it++) {
      int temp = ActionGoto[i][*it];
      if (temp == -1) {
        output << setw(7) << "acc";
      } else if (temp < 256 && temp > 0) {
        output << setw(7) << to_string(temp);
      } else if (temp >= 256) {
        output << setw(7) << "r" + to_string(temp - 256);
      } else {
        output << setw(7) << "";
      }
    }
    output << endl;
  }
  VT.erase('#');
  output.close();
}