// k7scan1.cpp : Definiert den Einsprungpunkt f�r die Konsolenanwendung.
//

#include "stdafx.h"
#include "main.h"
#include "lex.h"
#pragma warning(disable:4786)
using namespace std;



int main(int argc, char* argv[])
{
	FILE *inf;
	char fistr[100];
	if (argc == 1) {
		printf("Enter .txt filename:\n");
		scanf("%s", fistr);//gets(fistr);
		inf = fopen(strcat(fistr, ".txt"), "r");
	}
	else {
		inf = fopen(argv[1], "r");
	}
	if(inf==NULL){
		printf("Cannot open input file %s\n",fistr);
		return 0;
	}
	CParser obj;
	obj.InitParse(inf,stderr,stdout);
//	obj.pr_tokentable();
	obj.yyparse();
	if (argc < 2) {
		char c; cin >> c;
	}

	return 0;
}



