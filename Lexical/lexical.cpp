#include "lexical.h"
#include "route.h"

NFA nfa;             //保存正规文法 转换成 的NFA
char str_file[1000]; //保存文件里读出的内容
vector<char> inchar; //保存终结符VT
vector<string> keywords;
vector<string> opt;
vector<string> limiter;

/**************************************初始化涉及的操作 **********************************/
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


/**************************************正规文法→NFA涉及的操作 ****************************/
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

void initial()
{
}