#include "lexical.h"
#include "route.h"
#include<string.h>
NFA nfa;             //���������ķ� ת���� ��NFA
char str_file[1000]; //�����ļ������������
vector<char> inchar; //�����ս��VT
vector<string> keywords;
vector<string> opt;
vector<string> limiter;

DFA dfa;
bool Final[MAX_NODES];						//����DFA״̬������Щ���ս��������O(1)����  

vector<vector<string> > sourceCode; 	            //����Դ���� 

/************************************  ��ʼ���漰�Ĳ���   *********************************/
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


/************************************  �����ķ� �� NFA�漰�Ĳ��� ****************************/
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

/************************************   NFA �� DFA�漰�Ĳ���   ****************************/
// ��״̬��T�ıհ����հ�Ҳ��һ������
// �հ���״̬��T�е��κ�״̬�������������Ż����ܵ��� ��״̬
// ����set���������ظ�Ԫ��
set<char> e_closure(set<char> T){
    set<char> U=T;
    int previous_size,current_size;
    while(1){
        // ����T�е�ÿһ��״̬
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
        // ��U��״̬����Ŀ���ٷ����仯ʱ����
        if(current_size==previous_size) break;
    }
    return U;
}

// ��move��
// move�������п��Դ�I�е�ĳһ״̬��һ��input�����ܵ����״̬
set<char> move(set<char> I,char input){
    set<char> U;
    // ����I�е�ÿһ��״̬
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

// NFAת��ΪDFA
void NFA_TO_DFA(){
    nfa.initialState='S';
    dfa.initialState='S';

    
    set<char> C[MAX_NODES]; //DFA��״̬��
    bool marked[MAX_NODES]; //���״̬�����ڼ�¼�Ƿ��Ѿ������״̬�ıհ�

    memset(dfa.f,-1,sizeof dfa.f);
    memset(marked,false,sizeof marked);

    set<char>T0;
    // 1����ʼ����� -closure(T0) ΪC��Ψһ��Ա����������δ����ǵġ�
    T0.insert(nfa.initialState);
    T0=e_closure(T0);

    C[0]=T0;
    int node_count=1;//��ʼ��DFA״̬����
    int i=0;
    // ��C�д�����δ����ǵ��Ӽ�T
    while(!marked[i]&&i<node_count){
        // ���
        marked[i]=true;
        // ����ÿ��������ĸinchar
        for(int j=0;j<inchar.size();j++){
            set<char> U =move(C[i],inchar[j]);//��move��
            U=e_closure(U);//��հ�

            bool inC=false;
            if(!U.empty()){
                for(int k=0;k<node_count;k++){
                    // ����²�����״̬�Ƿ���ԭ����״̬����
                    if(C[k]==U){
                        inC=true;
                        dfa.f[change(i)][inchar[j]]=change(k);
                    }
                }
            }
            if(!inC&&!U.empty()){
                C[node_count]=U;
                // ��DFA��״̬������
                dfa.f[change(i)][inchar[j]]=change(node_count);
                node_count++;
            }
        }
        i++;
    }
    for(int i=0;i<node_count;i++){
        // ���DFA����̬
        bool isFinalState=false;
        for(int j=0;j<nfa.finalState.size();j++){
            if(C[i].find(nfa.finalState[j])!=C[i].end()){
                isFinalState=true;
                break;
            }
        }
        // ����
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
    // ��DFA��̬��������������
    memset(Final,false,sizeof Final);
    for(int i=0;i<dfa.finalState.size();i++){
        Final[dfa.finalState[i]]=true;
    }
}

// ��ӡDFA
void printDFA(){

    cout << "****************DFA Show Start*******************" << endl;

    cout<<"��ʼ״̬��"<<dfa.initialState<<endl;
    cout<<"��ֹ״̬��";
    for(int i=0;i<dfa.finalState.size();i++){
        cout<<dfa.finalState[i]<<" ";
    }
    cout<<endl;
    cout<<"״̬ת�ƺ�����"<<endl;
    for(int i=0;i<MAX_NODES;i++){
        for(int j=0;j<MAX_NODES;j++){
            if(dfa.f[i][j] != -1) cout << char(i) << " -> " << char(j) << char(dfa.f[i][j]) << endl;
        }
    }
     cout << "****************DFA Show END*******************" << endl;
    cout << endl;
}

/************************************  ɨ��ʶ���û������Դ�����漰�Ĳ���   ****************************/
// ����Դ���� ��������������
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
// ɨ��Դ����
void scanSourceCode(){
    spilitSourceCode();
    
}