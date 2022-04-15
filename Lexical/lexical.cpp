#include "lexical.h"
#include "route.h"

NFA nfa;             //���������ķ� ת���� ��NFA
char str_file[1000]; //�����ļ������������
vector<char> inchar; //�����ս��VT
vector<string> keywords;
vector<string> opt;
vector<string> limiter;

/**************************************��ʼ���漰�Ĳ��� **********************************/
//  1.����VT�ս��
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

//  2.���عؼ���
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

//  3.���ز�����
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

//  4.���ؽ��
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

//  5.��ʼ��
void init(){
    load_inchar();
    load_keywords();
    load_opt();
    load_limiter();
}


/**************************************�����ķ���NFA�漰�Ĳ��� ****************************/
// �ж��Ƿ����ս��VT
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
// ���ݶ���������ķ�����NFA
void createNFA()
{
    fstream file;
    file.open(GRAMMAR_FILE_PATH); //���������ķ�
    if (!file.is_open())
    {
        cout << "�����ķ��ļ��޷���" << endl;
        return;
    }

    // ��txt�ļ��������һ��һ�еض���str_file,��ÿһ�������ķ����д���
    while (file.getline(str_file, 10))
    {

        Triad tempTriad;
        char *s = str_file;
        tempTriad.startPoint = *s++;
        // s�ƶ�,ʹ��ָ��->��ĵ�һ���ַ�
        s++;
        s++;
        // �����������ķ�
        // A->epsilon
        if (*s == '$')
        {
            // ����һ����̬��㣬��ʼ���Ŷ�Ӧ�Ľ����Ϊ��̬
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
                // ������ A��a �Ĺ�����һ����A����̬���Ļ������Ϊa
                tempTriad.input = tempChar;
                tempTriad.endPoint = 'Y';
                nfa.f.push_back(tempTriad);
                nfa.finalState.push_back('Y');
            }
            // A->aB
            else
            {
                // ������ A��aB �Ĺ�����һ����A��B�Ļ������Ϊa
                tempTriad.input = tempChar;
                tempTriad.endPoint = *s;
                nfa.f.push_back(tempTriad);
            }
        }
    }
}

// ��ʾNFA
void printNFA()
{
    cout << "****************NFA Show Start*******************" << endl;
    cout << "�����ķ�ת�����NFA:" << endl;
    cout << "��̬��" << nfa.initialState << endl;
    cout << "��̬��";
    for (int i = 0; i < nfa.finalState.size(); i++)
    {
        cout << nfa.finalState[i] << " ";
    }
    cout << endl;
    cout<<"NFA�ߵ�����: "<<nfa.f.size()<<endl;
    cout << "�ߣ�" << endl;
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