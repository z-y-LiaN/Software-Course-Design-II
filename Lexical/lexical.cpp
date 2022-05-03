#include "lexical.h"

#include "path.h"

NFA nfa;              
char str_file[1000]; 

vector<char> INCHAR;  
vector<string> KEYWORDS;
vector<string> OPT;
vector<string> LIMITER;

DFA dfa;
bool Final[MAX_NODES];  

vector<vector<string>> sourceCode;   
vector<pair<string, string>> token;  
vector<pair<pair<int, int>, int>> wrong;  
vector<int> row;        

/**
 *  @brief   加载终结符
 */
void load_inchar() {
  fstream file;
  file.open(INCHAR_FILE_PATH);
  char tempChar;
  while (file >> tempChar) {
    if (tempChar != ',') INCHAR.push_back(tempChar);
  }
  file.close();
}

/**
 *  @brief   加载关键字
 */
void load_keywords() {
  fstream file;
  file.open(KEYWORDS_FILE_PATH);
  string str = "";
  char tempChar;
  while (file >> tempChar) {
    if (tempChar != ',')
      str += tempChar;
    else {
      KEYWORDS.push_back(str);
      str = "";
    }
  }
  file.close();
}

/**
 *  @brief   加载操作符
 */
void load_opt() {
  fstream file;
  file.open(OPERATOR_FILE_PATH);
  string str = "";
  char tempChar;
  while (file >> tempChar) {
    if (tempChar != ',')
      str += tempChar;
    else {
      OPT.push_back(str);
      str = "";
    }
  }
  file.close();
}

/**
 *  @brief   加载界符
 */
void load_limiter() {
  fstream file;
  file.open(LIMITER_FILE_PATH);
  string str = "";
  char tempChar;
  while (file >> tempChar) {
    if (tempChar != ',')
      str += tempChar;
    else {
      LIMITER.push_back(str);
      str = "";
    }
  }
  file.close();
}

/**
 *  @brief   初始化所有加载项
 */
void init() {
  load_inchar();
  load_keywords();
  load_opt();
  load_limiter();
}

/**
 * @brief  判断是否是终结符
 * @param   char-a 输入字符
 * @return  bool
 */
bool isVT(char a) {
  for (int i = 0; i < INCHAR.size(); i++) {
    if (a == INCHAR[i]) {
      return true;
    }
  }
  return false;
}

/**
 * @brief  根据读入的正规文法创建NFA，一个正规文法对应NFA中的一条边
 * @details 右线性，增加一个终态结点Y，开始符号对应的结点作为初态
 *          A→t 引一条从A到Y的弧，标记为t,A→tB 引一条从A到B的弧，标记为t
 */
void createNFA() {
  fstream file;
  file.open(GRAMMAR_FILE_PATH);
  if (!file.is_open()) {
    cout << "error: 正规文法文件无法打开" << endl;
    return;
  }
  while (file.getline(str_file, 10)) {
    Triad tempTriad;
    char *s = str_file;
    tempTriad.startPoint = *s++;
    s++;
    s++;  // s指向->后的第一个字符
    // A->epsilon,加入终态集
    if (*s == '$') {
      nfa.finalState.push_back(tempTriad.startPoint);
    }
    // A->B
    else if (*s >= 'A' && *s <= 'Z') {
      tempTriad.input = '$';
      tempTriad.endPoint = *s;
      nfa.f.push_back(tempTriad);

    } else if (isVT(*s)) {
      char tempChar = *s;
      s++;
      // A->a
      if ((int)*s == 0) {
        tempTriad.input = tempChar;
        tempTriad.endPoint = 'Y'; 
        nfa.f.push_back(tempTriad);
        int flag_exits=false;
        for(int i=0;i<nfa.finalState.size();i++){
          if(nfa.finalState[i]=='Y') flag_exits=true;
        }
        if(!flag_exits) nfa.finalState.push_back('Y'); 
      }
      // A->aB
      else {
        tempTriad.input = tempChar;
        tempTriad.endPoint = *s;
        nfa.f.push_back(tempTriad);
      }
    }
  }
  // 终态去重
  sort(nfa.finalState.begin(),nfa.finalState.end());
  nfa.finalState.erase(unique(nfa.finalState.begin(),nfa.finalState.end()),nfa.finalState.end());
  nfa.initialState='S';
  file.close();
}

/**
 * @brief 打印NFA
 */
void printNFA() {
  cout << "****************NFA Show Start*******************" << endl;
  cout << "正规文法转换后的NFA:" << endl;
  cout << "初态：" << nfa.initialState << endl;
  cout << "终态：";
  for (int i = 0; i < nfa.finalState.size(); i++) {
    cout << nfa.finalState[i] << " ";
  }
  cout << endl;
  cout << "NFA边的数据: " << nfa.f.size() << endl;
  cout << "边：" << endl;
  for (int i = 0; i < nfa.f.size(); i++) {
    cout << nfa.f[i].startPoint << " -> " << nfa.f[i].input << " -> "
         << nfa.f[i].endPoint << endl;
  }
  cout << "****************NFA Show END*******************" << endl;
  cout << endl;
}

/**
 * @brief 求状态集T的闭包
 * @param set<chat>-T 输入状态集
 * @return 返回该状态集的闭包
 * @details 闭包：状态集T中的所有状态，及其 经过任意条ε弧所能到达的状态
 */
set<char> e_closure(set<char> T) {
  set<char> U = T;
  int previous_size, current_size;
  while (1) {
    for (set<char>::iterator it = U.begin(); it != U.end(); it++) {
      char tempChar = *it;
      // 经过任意条ε弧能到达的状态
      for (int k = 0; k < nfa.f.size(); k++) {
        if (nfa.f[k].startPoint == tempChar && nfa.f[k].input == '$') {
          U.insert(nfa.f[k].endPoint);
        }
      }
    }
    previous_size = current_size;
    current_size = U.size();
    // 当U中状态的数目不再发生增加时结束
    if (current_size == previous_size) break;
  }
  return U;
}

/**
 * @brief 求move集-所有可以从I中的某一状态 经一条input弧 所能到达的状态
 * @param set<char>-I 一个状态集
 * @param char-input 一条弧的标记
 * @return 状态集合
 */
set<char> move(set<char> I, char input) {
  set<char> U;
  for (set<char>::iterator it = I.begin(); it != I.end(); it++) {
    for (int k = 0; k < nfa.f.size(); k++) {
      if (nfa.f[k].startPoint == *it && nfa.f[k].input == input) {
        U.insert(nfa.f[k].endPoint);
      }
    }
  }
  return U;
}

/**
 * @brief 状态重命名
 * @param int-a 状态序号
 * @return char 序号到字符的映射：0-S; 1-A,2-B....依次类推
 */
char change(int a) {
  if (a == 0)
    return 'S';
  else
    return char(a + 64);
}

/**
 * @brief 子集法，将NFA转换成DFA；
 * @details 根据教材算法流程，最后再添加DFA终态集
 */
void NFA_TO_DFA() {
  nfa.initialState = 'S';
  dfa.initialState = 'S';

  set<char> C[MAX_NODES];  // DFA的状态集
  bool marked[MAX_NODES];  //标记是否已经求出该状态的闭包

  memset(dfa.f, -1, sizeof dfa.f);
  memset(marked, false, sizeof marked);

  set<char> T0;
  T0.insert(nfa.initialState);
  T0 = e_closure(T0);

  C[0] = T0;
  int node_counter = 1;  //初始化DFA状态数量
  int state_index = 0;
  // 当C中存在尚未被标记的子集T
  while (!marked[state_index] && state_index < node_counter) {
    marked[state_index] = true;
    for (int j = 0; j < INCHAR.size(); j++) {
      set<char> U = move(C[state_index], INCHAR[j]); 
      U = e_closure(U);  

      bool alreadyExitsInC = false;
      if (!U.empty()) {
        for (int k = 0; k < node_counter; k++) {
          // 在
          if (C[k] == U) {
            alreadyExitsInC = true;
            dfa.f[change(state_index)][INCHAR[j]] = change(k);
          }
        }
      }
      // 不在
      if (!alreadyExitsInC && !U.empty()) {
        C[node_counter] = U;
        dfa.f[change(state_index)][INCHAR[j]] = change(node_counter);
        node_counter++;
      }
    }
    state_index++;
  }
  // 添加DFA终态
  for (int i = 0; i < node_counter; i++) {
    bool isFinalState = false;
    for (int j = 0; j < nfa.finalState.size(); j++) {
      if (C[i].find(nfa.finalState[j]) != C[i].end()) {
        isFinalState = true;
        break;
      }
    }
    // 查重
    if (isFinalState) {
      bool inFinal = false;
      for (int j = 0; j < dfa.finalState.size(); j++) {
        if (dfa.finalState[j] == change(i)) {
          inFinal = true;
          break;
        }
      }
      if (!inFinal) {
        dfa.finalState.push_back(change(i));
      }
    }
  }
  // 将DFA终态保存在数组里面
  memset(Final, false, sizeof Final);
  for (int i = 0; i < dfa.finalState.size(); i++) {
    Final[dfa.finalState[i]] = true;
  }
}

/**
 * @brief 打印DFA 
 */
void printDFA() {
  cout << "****************DFA Show Start*******************" << endl;

  cout << "初始状态：" << dfa.initialState << endl;
  cout << "终止状态：";
  for (int i = 0; i < dfa.finalState.size(); i++) {
    cout << dfa.finalState[i] << " ";
  }
  cout << endl;
  cout << "状态转移函数：" << endl;
  for (int i = 0; i < MAX_NODES; i++) {
    for (int j = 0; j < MAX_NODES; j++) {
      if (dfa.f[i][j] != -1)
        cout << char(i) << " -> " << char(j) << " -> "<< char(dfa.f[i][j]) << endl;
    }
  }
  cout << "****************DFA Show END*******************" << endl;
  cout << endl;
}


/**
 * @brief 过滤源程序的tab和换行，保存在数组里面:sourceCode
 */
void spilitSourceCode() {
  fstream file;
  file.open(SOURCE_FILE_PATH);
  char temp[100];
  while (file.getline(temp, 100)) {
    vector<string> vec;
    string str = "";
    for (int i = 0; i < strlen(temp); i++) {
      if ((temp[i] == ' ' || temp[i] == '\t') && str == "")
        continue;
      else if ((temp[i] == ' ' || temp[i] == '\t') && str != "") {
        vec.push_back(str);
        str = "";
      } else {
        str += temp[i];
      }
    }
    if (str != "") vec.push_back(str);
    sourceCode.push_back(vec);
  }
  file.close();
  // for(int i = 0;i<sourceCode.size();i++){
  // 	for(int j = 0;j<sourceCode[i].size();j++){
  // 		cout << sourceCode[i][j] << " ";
  // 	}
  // 	cout << endl;
  // }
}

bool isKeywords(string str) {
  for (int i = 0; i < KEYWORDS.size(); i++) {
    if (str == KEYWORDS[i]) return true;
  }
  return false;
}
bool isLimiter(string str) {
  for (int i = 0; i < LIMITER.size(); i++) {
    if (str == LIMITER[i]) return true;
  }
  return false;
}
bool isOperator(string str) {
  for (int i = 0; i < OPT.size(); i++) {
    if (str == OPT[i]) return true;
  }
  return false;
}

/**
 * @brief 扫描源程序进行分析，生成token序列
 */
void scanSourceCode() {
  spilitSourceCode();
  for (int i = 0; i < sourceCode.size(); i++) {
    int token_counter = 0;
    for (int j = 0; j < sourceCode[i].size(); j++) {
      string str = sourceCode[i][j]; // cout<<str<<endl;
      string result = "";  //存放单个word
      int next = dfa.initialState;
      int now;
      vector<pair<string, string>> token_temp; //word+type
      for (int k = 0; k < str.length(); k++) {
        // error：字符本身不属于VN
        if (!isVT(str[k])) {
          pair<int, int> position = make_pair(i + 1, j + 1);
          pair<pair<int, int>, int> position_errorChar = make_pair(position, int(str[k]));
          // error去重
          bool flag = false;
          for (int k = 0; k < wrong.size(); k++) {
            if (wrong[k].first.first == i + 1 && wrong[k].first.second == j + 1 && wrong[k].second == 0) {
              flag = true;
              break;
            }
          }
          if (!flag) wrong.push_back(position_errorChar);
          continue;
        }

        next = dfa.f[next][str[k]];
        if (next == -1) {  
          token_temp.push_back(make_pair(result, "    "));
          result.clear();
          result += str[k];
          next = dfa.f[dfa.initialState][str[k]];
        } else {
          result += str[k];
        }
      }
      if (!result.empty()) {
        token_temp.push_back(make_pair(result, "    "));
      }

      // 该行word分类
      bool isComplexNumber = false;
      for (int token_index = 0; token_index < token_temp.size(); token_index++) {
        if (isKeywords(token_temp[token_index].first)) { 
          token_temp[token_index].second = "KEYWORDS";
          token.push_back(token_temp[token_index]);
        } else if (isLimiter(token_temp[token_index].first)) { 
          token_temp[token_index].second = "LIMITER";
          token.push_back(token_temp[token_index]);
        } else if (isOperator(token_temp[token_index].first)) {
          token_temp[token_index].second = "OPERATOR";
          token.push_back(token_temp[token_index]);
        } else if (token_temp[token_index].first[0] >= 'a' &&
                   token_temp[token_index].first[0] <= 'z') {
          // 检查标识符的合法性
          if (token_index && token_temp[token_index - 1].second == "CONST") {
            pair<int, int> postion = make_pair(i + 1, j + 1);
            pair<pair<int, int>, int> position_errorChar = make_pair(postion, 0);
            wrong.push_back(position_errorChar);
          }
          token_temp[token_index].second = "ID";
          token.push_back(token_temp[token_index]);
        } else if (token_temp[token_index].first[0] >= '0' &&
                   token_temp[token_index].first[0] <= '9') {  
          int index = token.size() - 1;
          // Ai+B
          if ((token[index].first == "+" || token[index].first == "-") &&
              token[index - 1].second == "CONST" &&
              token[index - 1].first.find("i") != token[index - 1].first.npos) {
            isComplexNumber = true;
            token[index - 1].first += token[index].first;
            token[index - 1].first += token_temp[token_index].first;
            token.erase(token.end());
          } else {
            token_temp[token_index].second = "CONST";
            token.push_back(token_temp[token_index]);
          }
        }
      }
      if (isComplexNumber)
        token_counter += token_temp.size() - 2;
      else
        token_counter += token_temp.size();
    }
    row.push_back(token_counter); 
  }
  ofstream outfile;
  outfile.open(WRONG_FILE_PATH);
  for (int i = 0; i < wrong.size(); i++) {
    if (wrong[i].second != 0) {
      cout<<endl;
      cout << "=======ERROR_INFO！！！！======="<<endl;
      cout<<" 程序第 " << wrong[i].first.first << " 行, 第 "
           << wrong[i].first.second << "  个单词出错，出错原因：出现未知符号"
           << (char)wrong[i].second << endl;
      cout<<endl;
      outfile << "error-info: in line: " << wrong[i].first.first << " , in word "
              << wrong[i].first.second << "  : unknown symbol appears"
              << (char)wrong[i].second << endl;
    }
    if (wrong[i].second == 0) {
      cout<<endl;
      cout << "=======ERROR_INFO！！！！======="<<endl;
      cout<<" 程序第 " << wrong[i].first.first <<  " 行, 第 "
           << wrong[i].first.second << "  个单词出错，出错原因：变量命名错误" << endl;
      cout<<endl;
      outfile << "error-info: in line" << wrong[i].first.first << " , in word "
              << wrong[i].first.second << " : variable naming error"<< endl;
    }
  }
  outfile.close();

  outfile.open(TOKEN_FILE_PATH);
  for (int i = 0; i < token.size(); i++) {
    outfile << token[i].first << "         " << token[i].second << endl;
  }
  outfile.close();
  outfile.open(TOKEN_TRIAD_FILE_PATH);
  int counter = 0;
  for (int i = 0; i < row.size(); i++) {
    for (int j = counter; j < row[i] + counter; j++) {
      outfile << setiosflags(ios::left) << setw(8) << i + 1 << setw(12)
              << token[j].first << setw(8) << token[j].second << endl;
    }
    counter += row[i];
  }
  outfile.close();

  // 保存源代码每行的token数量,用于语法分析报错指定位置
  outfile.open(ROW_FILE_PATH);
  for (int i = 0; i < row.size(); i++) {
    outfile << row[i] << endl;
  }

  if (wrong.size() == 0) {
    cout<<"****************************************************************"<<endl;
    cout<<"*                                                              *"<<endl;
    cout <<"*              恭喜!  词法分析完成，没有错误！                 *" << endl;
    cout<<"*                                                              *"<<endl;
    cout<<"****************************************************************"<<endl;
  }
}