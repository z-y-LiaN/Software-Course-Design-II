#include "lexical.h"
#include "route.h"
#include<string.h>
NFA nfa;             //保存正规文法 转换成 的NFA
char str_file[1000]; //保存文件里读出的内容
vector<char> inchar; //保存终结符VT
vector<string> keywords;
vector<string> opt;
vector<string> limiter;

DFA dfa;
bool Final[MAX_NODES];						//保存DFA状态集中哪些是终结符，便于O(1)查找  

vector<vector<string> > sourceCode; 	            //保存源程序 

/************************************  初始化涉及的操作   *********************************/
//  1.加载VT终结符
void load_inchar()
{
    fstream file;
    file.open(INCHAR_FILE_PATH);
    char tempChar;
    while (file >> tempChar)
    {
        if (tempChar != ',')
            inchar.push_back(tempChar);
    }
}

//  2.加载关键字
void load_keywords()
{
    fstream file;
    file.open(KEYWORDS_FILE_PATH);
    string str = "";
    char tempChar;
    while (file >> tempChar)
    {
        if (tempChar != ',')
            str += tempChar;
        else
        {
            keywords.push_back(str);
            str = "";
        }
    }
}

//  3.加载操作符
void load_opt(){
    fstream file;
    file.open(OPERATOR_FILE_PATH);
    string str="";
    char tempChar;
    while(file>>tempChar){
        if(tempChar!=',') str+=tempChar;
        else{
            opt.push_back(str);
            str="";
        }
    }
}

//  4.加载界符
void load_limiter(){
    fstream file;
    file.open(LIMITER_FILE_PATH);
    string str="";
    char tempChar;
    while(file>>tempChar){
        if(tempChar!=',') str+=tempChar;
        else{
            limiter.push_back(str);
            str="";
        }
    }
}

//  5.初始化
void init(){
    load_inchar();
    load_keywords();
    load_opt();
    load_limiter();
}


/************************************  正规文法 → NFA涉及的操作 ****************************/
// 判断是否是终结符VT
bool isVT(char a)
{
    for (int i = 0; i < inchar.size(); i++)
    {
        if (a == inchar[i])
        {
            return true;
        }
    }
    return false;
}
// 根据读入的正规文法创建NFA
void createNFA()
{
    fstream file;
    file.open(GRAMMAR_FILE_PATH); //读入正规文法
    if (!file.is_open())
    {
        cout << "正规文法文件无法打开" << endl;
        return;
    }

    // 将txt文件里的内容一行一行地读入str_file,对每一个正规文法进行处理
    while (file.getline(str_file, 10))
    {

        Triad tempTriad;
        char *s = str_file;
        tempTriad.startPoint = *s++;
        // s移动,使其指向->后的第一个字符
        s++;
        s++;
        // 右线性正规文法
        // A->epsilon
        if (*s == '$')
        {
            // 增加一个终态结点，开始符号对应的结点作为初态
            nfa.finalState.push_back(tempTriad.startPoint);
        }
        // A->B
        else if (*s >= 'A' && *s <= 'Z')
        {
            tempTriad.input = '$';
            tempTriad.endPoint = *s;

            nfa.f.push_back(tempTriad);
        }
        else if (isVT(*s))
        {
            char tempChar = *s;
            s++;
            // A->a
            if ((int)*s == 0)
            {
                // 对形如 A→a 的规则，引一条从A到终态结点的弧，标记为a
                tempTriad.input = tempChar;
                tempTriad.endPoint = 'Y';
                nfa.f.push_back(tempTriad);
                nfa.finalState.push_back('Y');
            }
            // A->aB
            else
            {
                // 对形如 A→aB 的规则，引一条从A到B的弧，标记为a
                tempTriad.input = tempChar;
                tempTriad.endPoint = *s;
                nfa.f.push_back(tempTriad);
            }
        }
    }
}

// 显示NFA
void printNFA()
{
    cout << "****************NFA Show Start*******************" << endl;
    cout << "正规文法转换后的NFA:" << endl;
    cout << "初态：" << nfa.initialState << endl;
    cout << "终态：";
    for (int i = 0; i < nfa.finalState.size(); i++)
    {
        cout << nfa.finalState[i] << " ";
    }
    cout << endl;
    cout<<"NFA边的数据: "<<nfa.f.size()<<endl;
    cout << "边：" << endl;
    for (int i = 0; i < nfa.f.size(); i++)
    {
        cout << nfa.f[i].startPoint << " -> " << nfa.f[i].input << " -> " << nfa.f[i].endPoint << endl;
    }
    cout << "****************NFA Show END*******************" << endl;
    cout << endl;
}

/************************************   NFA → DFA涉及的操作   ****************************/
// 求状态集T的闭包，闭包也是一个集合
// 闭包，状态集T中的任何状态，经过任意条ε弧所能到达 的状态
// 采用set，不会有重复元素
set<char> e_closure(set<char> T){
    set<char> U=T;
    int previous_size,current_size;
    while(1){
        // 遍历T中的每一个状态
        for(set<char>::iterator it=U.begin();it!=U.end();it++){
            char tempChar=*it;
            for(int k=0;k<nfa.f.size();k++){
                if(nfa.f[k].startPoint == tempChar && nfa.f[k].input =='$'){
                    U.insert(nfa.f[k].endPoint);
                }
            }
        }
        previous_size = current_size;
        current_size=U.size();
        // 当U中状态的数目不再发生变化时结束
        if(current_size==previous_size) break;
    }
    return U;
}

// 求move集
// move集：所有可以从I中的某一状态经一条input弧所能到达的状态
set<char> move(set<char> I,char input){
    set<char> U;
    // 遍历I中的每一个状态
    for(set<char>::iterator it=I.begin();it!=I.end();it++){
        for(int k=0;k<nfa.f.size();k++){
            if(nfa.f[k].startPoint == *it && nfa.f[k].input == input){
                U.insert(nfa.f[k].endPoint);
            }
        }
    }
    return U;
}
char change(int a){
    if(a==0) return 'S';
    else return char(a+64);
}

// NFA转换为DFA
void NFA_TO_DFA(){
    nfa.initialState='S';
    dfa.initialState='S';

    
    set<char> C[MAX_NODES]; //DFA的状态集
    bool marked[MAX_NODES]; //标记状态，用于记录是否已经求出该状态的闭包

    memset(dfa.f,-1,sizeof dfa.f);
    memset(marked,false,sizeof marked);

    set<char>T0;
    // 1）开始，令ε -closure(T0) 为C中唯一成员，并且它是未被标记的。
    T0.insert(nfa.initialState);
    T0=e_closure(T0);

    C[0]=T0;
    int node_count=1;//初始化DFA状态数量
    int i=0;
    // 当C中存在尚未被标记的子集T
    while(!marked[i]&&i<node_count){
        // 标记
        marked[i]=true;
        // 对于每个输入字母inchar
        for(int j=0;j<inchar.size();j++){
            set<char> U =move(C[i],inchar[j]);//求move集
            U=e_closure(U);//求闭包

            bool inC=false;
            if(!U.empty()){
                for(int k=0;k<node_count;k++){
                    // 检查新产生的状态是否再原来的状态集中
                    if(C[k]==U){
                        inC=true;
                        dfa.f[change(i)][inchar[j]]=change(k);
                    }
                }
            }
            if(!inC&&!U.empty()){
                C[node_count]=U;
                // 对DFA的状态重命名
                dfa.f[change(i)][inchar[j]]=change(node_count);
                node_count++;
            }
        }
        i++;
    }
    for(int i=0;i<node_count;i++){
        // 添加DFA的终态
        bool isFinalState=false;
        for(int j=0;j<nfa.finalState.size();j++){
            if(C[i].find(nfa.finalState[j])!=C[i].end()){
                isFinalState=true;
                break;
            }
        }
        // 查重
        if(isFinalState){
            bool inFinal=false;
            for(int j=0;j<dfa.finalState.size();j++){
                if(dfa.finalState[j]==change(i)){
                    inFinal=true;
                    break;
                }
            }
            if(!inFinal){
                dfa.finalState.push_back(change(i));
            }
        }
    }
    // 将DFA终态保存再数组里面
    memset(Final,false,sizeof Final);
    for(int i=0;i<dfa.finalState.size();i++){
        Final[dfa.finalState[i]]=true;
    }
}

// 打印DFA
void printDFA(){

    cout << "****************DFA Show Start*******************" << endl;

    cout<<"初始状态："<<dfa.initialState<<endl;
    cout<<"终止状态：";
    for(int i=0;i<dfa.finalState.size();i++){
        cout<<dfa.finalState[i]<<" ";
    }
    cout<<endl;
    cout<<"状态转移函数："<<endl;
    for(int i=0;i<MAX_NODES;i++){
        for(int j=0;j<MAX_NODES;j++){
            if(dfa.f[i][j] != -1) cout << char(i) << " -> " << char(j) << char(dfa.f[i][j]) << endl;
        }
    }
     cout << "****************DFA Show END*******************" << endl;
    cout << endl;
}

/************************************  扫描识别用户输入的源代码涉及的操作   ****************************/
// 读入源程序 保存在数组里面
void spilitSourceCode(){
    fstream file;
    file.open(SOURCE_FILE_PATH);
    char temp[100];
    while(file.getline(temp,100)){
        vector<string> vec;
        string str="";
        for(int i=0;i<strlen(temp);i++){
            if((temp[i]==' '||temp[i]=='\t') && str=="") continue;
            else if((temp[i]==' '||temp[i]=='\t') && str!=""){
                vec.push_back(str);
                str="";
            }else{
                str+=temp[i];
            }
        }
        if(str!="") vec.push_back(str);
            sourceCode.push_back(vec);
   }
    file.close();
	for(int i = 0;i<sourceCode.size();i++){
		for(int j = 0;j<sourceCode[i].size();j++){
			cout << sourceCode[i][j] << " ";
		}
		cout << endl;
	}

}
// 扫描源程序
void scanSourceCode(){
    spilitSourceCode();
    
}