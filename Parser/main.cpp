#include"parser.h";
int main(){
    memset(AG,0,sizeof(AG));
	readGrammarFile();
	getFirstSet();
	create('Z',"S");
	show();
	scan(token);
}