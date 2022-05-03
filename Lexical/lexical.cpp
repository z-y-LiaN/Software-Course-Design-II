#include "lexical.h"

#include "path.h"

NFA nfa;              //保存正规文法 转换成 的NFA
char str_file[1000];  //保存文件里读出的内容

vector<char> INCHAR;  //保存终结符VT
vector<string> KEYWORDS;
vector<string> OPT;
vector<string> LIMITER;

DFA dfa;
bool Final[MAX_NODES];  //保存DFA状态集中哪些是终结符，便于O(1)查找

vector<vector<string>> sourceCode;   //保存源程序
vector<pair<string, string>> token;  //保存扫描源程序后得到的token表
vector<pair<pair<int, int>, int>> wrong;  //保存错误信息
vector<int> row;                          //保存每行有多少个token

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
 *  @brief  根据读入的正规文法创建NFA
 *  @details
 * 根据右线性正规文法的转换规则，对每一个正规文法进行处理,每个正规文法对应NFA中的一条边
 * 增加一个终态结点Y，开始符号对应的结点作为初态
 * 对形如 A→t 的规则，引一条从A到终态结点的弧，标记为t
 * 对形如 A→tB 的规则，引一条从A到B的弧，标记为t
 * 注：t 为 VT 或epsilon
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
        // 对形如 A→a 的规则，引一条从A到终态结点Y的弧，标记为a
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
        // 对形如 A→aB 的规则，引一条从A到B的弧，标记为a
        tempTriad.input = tempChar;
        tempTriad.endPoint = *s;
        nfa.f.push_back(tempTriad);
      }
    }
  }
  file.close();
}

// 显示NFA
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

/************************************   NFA → DFA  ****************************/

//  @brief: 求状态集T的闭包
//  @param: set<char> T
//  @ret: 返回状态集T的一个闭包；
//        闭包：状态集T中的任何状态，及其 经过任意条ε弧所能到达的状态
//        闭包也是一个集合,所以用set,就不会有重复元素
set<char> e_closure(set<char> T) {
  // 闭包首先包含原始状态集的所有状态
  set<char> U = T;
  int previous_size, current_size;
  while (1) {
    for (set<char>::iterator it = U.begin(); it != U.end(); it++) {
      char tempChar = *it;
      // 再包含经过任意条ε弧能到达的状态
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

// @brief: 求move集-所有可以从I中的某一状态 经一条input弧 所能到达的状态
// @param:  set<char> I
//          char input
// @ret:  状态的集合
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

// 状态重命名:0-S;1-A,2-B....依次类推
char change(int a) {
  if (a == 0)
    return 'S';
  else
    return char(a + 64);
}

// @brief:  子集法，将NFA转换成DFA；
//            假定所构造的DFA子集族 为 C = (T1, T2,,... Ti)，
//            其中T1, T2,,... Ti为状态 K 的子集。
//            1）开始，令ε -closure(K0) 为C中唯一成员，并且它是未被标记的。
//            2）While(C中存在尚未被标记的子集T) do
//               {	标记T；
//                  for(每个 输入字母 a) do
//                  {	U:= ε-closure(move(T,a))；
//                  if（U不在C中）  then
//                  {	将U作为未标记的子集加在C中；}
//                }
// @param:  void
// @ret:    void
void NFA_TO_DFA() {
  nfa.initialState = 'S';
  dfa.initialState = 'S';

  set<char> C[MAX_NODES];  // DFA的状态集
  bool marked[MAX_NODES];  //标记状态，用于记录是否已经求出该状态的闭包

  memset(dfa.f, -1, sizeof dfa.f);
  memset(marked, false, sizeof marked);

  set<char> T0;
  // 1）开始，令ε -closure(T0) 为C中唯一成员，并且它是未被标记的。
  T0.insert(nfa.initialState);
  T0 = e_closure(T0);

  C[0] = T0;
  int node_count = 1;  //初始化DFA状态数量
  int i = 0;
  // 当C中存在尚未被标记的子集T
  while (!marked[i] && i < node_count) {
    // 标记
    marked[i] = true;
    // 对于每个输入字母inchar
    for (int j = 0; j < INCHAR.size(); j++) {
      set<char> U = move(C[i], INCHAR[j]);  //求move集
      U = e_closure(U);                     //求闭包

      bool inC = false;
      if (!U.empty()) {
        for (int k = 0; k < node_count; k++) {
          // 新产生的状态在原来的状态集中
          if (C[k] == U) {
            inC = true;
            dfa.f[change(i)][INCHAR[j]] = change(k);
          }
        }
      }
      // 新产生的状态不在原来的状态集中
      if (!inC && !U.empty()) {
        C[node_count] = U;
        dfa.f[change(i)][INCHAR[j]] = change(node_count);
        node_count++;
      }
    }
    i++;
  }
  // 添加DFA的终态：包含NFA终态的状态为DFA的终态
  for (int i = 0; i < node_count; i++) {
    bool isFinalState = false;
    for (int j = 0; j < nfa.finalState.size(); j++) {
      if (C[i].find(nfa.finalState[j]) != C[i].end()) {
        isFinalState = true;
        break;
      }
    }
    // 查重，vector存的，可能有重复的，其实也可以改成set用
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

// 打印DFA
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
        cout << char(i) << " -> " << char(j) << char(dfa.f[i][j]) << endl;
    }
  }
  cout << "****************DFA Show END*******************" << endl;
  cout << endl;
}

/************************************  扫描识别用户输入的源代码
 * ****************************/
// @brief:  读入源程序 ,过滤原来的空格、tab，保存在数组里面:sourceCode
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
// @brief:  读入源程序进行分析，生成token序列
void scanSourceCode() {
  spilitSourceCode();
  for (int i = 0; i < sourceCode.size(); i++) {
    int row_count = 0;
    // 读入一行
    for (int j = 0; j < sourceCode[i].size(); j++) {
      string str =
          sourceCode[i][j];  //第i行的第j个串儿，这个串可能包含多个单词(token)
      // cout<<str<<endl;
      string result = "";  //存放token
      int next = dfa.initialState;
      int now;
      // 存放一个二元组：token内容+类别
      vector<pair<string, string>> token_temp;
      // 先对这个串判断错误情况
      for (int k = 0; k < str.length(); k++) {
        // 错误情况：字符本身不属于非终结符集合（即：不是Σ里面的字符）
        if (!isVT(str[k])) {
          pair<int, int> position = make_pair(i + 1, j + 1);
          pair<pair<int, int>, int> position_errorChar =
              make_pair(position, int(str[k]));
          // vector存的，对错误情况去重
          bool flag = false;
          for (int k = 0; k < wrong.size(); k++) {
            if (wrong[k].first.first == i + 1 &&
                wrong[k].first.second == j + 1 && wrong[k].second == 0) {
              flag = true;
              break;
            }
          }
          if (!flag) wrong.push_back(position_errorChar);
          continue;
        }

        next = dfa.f[next][str[k]];
        if (next == -1) {  //识别出来了一个单词
          token_temp.push_back(make_pair(result, "    "));
          result.clear();
          // 如果是已经到达终态，那么就处理的字符是下一格token的开始
          result += str[k];
          next = dfa.f[dfa.initialState][str[k]];
        } else {
          // 下一个状态合法，则添加到result中
          result += str[k];
        }
      }

      // 处理最后一个可能的单词
      if (!result.empty()) {
        token_temp.push_back(make_pair(result, "    "));
      }

      // 这一行处理出来的 单词进行token分类
      bool flag = false;
      for (int k = 0; k < token_temp.size(); k++) {
        if (isKeywords(token_temp[k].first)) {  // 关键字
          token_temp[k].second = "KEYWORDS";
          token.push_back(token_temp[k]);
        } else if (isLimiter(token_temp[k].first)) {  // 界符
          token_temp[k].second = "LIMITER";
          token.push_back(token_temp[k]);
        } else if (isOperator(token_temp[k].first)) {  // 操作符
          token_temp[k].second = "OPERATOR";
          token.push_back(token_temp[k]);
        } else if (token_temp[k].first[0] >= 'a' &&
                   token_temp[k].first[0] <= 'z') {
          // 检查标识符的合法性:首字符是数字——非法
          if (k && token_temp[k - 1].second == "CONST") {
            pair<int, int> postion = make_pair(i + 1, j + 1);
            pair<pair<int, int>, int> position_errorChar =
                make_pair(postion, 0);
            wrong.push_back(position_errorChar);
          }
          token_temp[k].second = "ID";
          token.push_back(token_temp[k]);
        } else if (token_temp[k].first[0] >= '0' &&
                   token_temp[k].first[0] <= '9') {  // 当前串以数字开头

          int index = token.size() - 1;
          // 复数常量,这里规定复数的形式都是Ai+B的形式
          if ((token[index].first == "+" || token[index].first == "-") &&
              token[index - 1].second == "CONST" &&
              token[index - 1].first.find("i") != token[index - 1].first.npos) {
            flag = true;
            token[index - 1].first += token[index].first;
            token[index - 1].first += token_temp[k].first;
            token.erase(token.end());
          } else {
            token_temp[k].second = "CONST";
            token.push_back(token_temp[k]);
          }
        }
      }
      if (flag)
        row_count += token_temp.size() - 2;
      else
        row_count += token_temp.size();
    }
    row.push_back(row_count);  //保存该行的token数量，用于后面的语法分析
  }
  ofstream outfile;
  outfile.open(WRONG_FILE_PATH);
  for (int i = 0; i < wrong.size(); i++) {
    if (wrong[i].second != 0) {
      cout << "词法错误信息：在第" << wrong[i].first.first << "行，第"
           << wrong[i].first.second << "个单词出现未知符号"
           << (char)wrong[i].second << endl;
      outfile << "在第" << wrong[i].first.first << "行，第"
              << wrong[i].first.second << "个单词出现未知符号"
              << (char)wrong[i].second << endl;
    }
    if (wrong[i].second == 0) {
      cout << "词法错误信息：在第" << wrong[i].first.first << "行，第"
           << wrong[i].first.second << "个单词变量命名错误" << endl;
      outfile << "在第" << wrong[i].first.first << "行，第"
              << wrong[i].first.second << "个单词变量命名错误" << endl;
    }
  }
  outfile.close();
  // 保存token(二元组,方便用于后续的语法分析)
  outfile.open(TOKEN_FILE_PATH);
  for (int i = 0; i < token.size(); i++) {
    outfile << token[i].first << "         " << token[i].second << endl;
  }
  outfile.close();

  // 保存token三元组(行号,单词,类型);行号从1开始计数
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

  // 保存源代码每行的token数量,方便用于后续的语法分析
  outfile.open(ROW_FILE_PATH);
  for (int i = 0; i < row.size(); i++) {
    outfile << row[i] << endl;
  }

  if (wrong.size() == 0) {
    cout << "词法分析完成，没有错误" << endl;
  }
}