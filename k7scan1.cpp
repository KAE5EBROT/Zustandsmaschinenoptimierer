// k7scan1.cpp : Definiert den Einsprungpunkt für die Konsolenanwendung.
//

#include "stdafx.h"
#include <stdio.h>
#include <iostream>
#pragma warning(disable:4786)
#include <string>
#include <map>
using namespace std;

#define	Getc(s)			getc(s)
#define	Ungetc(c)		{ungetc(c,IP_Input); ugetflag=1;}


/*
 *	Lexical analyzer states.
 */
enum lexstate{L_START,L_INT,L_IDENT,L_STRING,L_STRING2,
		L_COMMENT,L_TEXT_COMMENT,L_LINE_COMMENT,L_END_TEXT_COMMENT};

const int STRING1=3;
const int IDENTIFIER=4;
const int INTEGER1=5;
const int TOKENSTART=300;

class CParser
{
public:

	string yytext;								//input buffer
	struct tyylval{								//value return
		string s;								//structure
		int i;
	}yylval;
	FILE *IP_Input;								//Input File
	FILE *IP_Error;								//Error Output
	FILE *IP_List;								//List Output
	int  IP_LineNumber;							//Line counter
	int ugetflag;								//checks ungets
	int prflag;									//controls printing
	map<string,int> IP_Token_table;				//Tokendefinitions
	map<int,string> IP_revToken_table;			//reverse Tokendefinitions


	int CParser::yylex();						//lexial analyser
	void CParser::yyerror(char *ers);			//error reporter
	int CParser::IP_MatchToken(string &tok);	//checks the token
	void CParser::InitParse(FILE *inp,FILE *err,FILE *lst);
	int	CParser::yyparse();						//parser
	void CParser::pr_tokentable();				//test output for tokens
	void CParser::IP_init_token_table();		//loads the tokens
	void CParser::Load_tokenentry(string str,int index);//load one token
	void CParser::PushString(char c);			//Used for dtring assembly
	CParser(){IP_LineNumber = 1;ugetflag=0;prflag=0;};	//Constructor
};
//------------------------------------------------------------------------

// Adds a character to the string value
void CParser::PushString(char c)
{
	yylval.s += c;
}
//------------------------------------------------------------------------
void CParser::Load_tokenentry(string str,int index)
{
	IP_Token_table[str]=index;
	IP_revToken_table[index]=str;
}
void CParser::IP_init_token_table()
{
	Load_tokenentry("STRING1",3);
	Load_tokenentry("IDENTIFIER",4);
	Load_tokenentry("INTEGER1",5);

	int ii=TOKENSTART;
	Load_tokenentry("AND",ii++);
	Load_tokenentry("OR",ii++);
	Load_tokenentry("Begin",ii++);
	Load_tokenentry("End",ii++);
	Load_tokenentry("DEFSTATE", ii++);
	Load_tokenentry("DEFIN", ii++);
	Load_tokenentry("DEFOUT", ii++);
}
//------------------------------------------------------------------------

void CParser::pr_tokentable()
{
	
	typedef map<string,int>::const_iterator CI;
	const char* buf;

	printf( "Symbol Table ---------------------------------------------\n");

	for(CI p=IP_Token_table.begin(); p!=IP_Token_table.end(); ++p){
		buf = p->first.c_str();
		printf(" key:%s", buf);
		printf(" val:%d\n", p->second);;
	}
}
//------------------------------------------------------------------------

int	CParser::yyparse()
{
	int tok;
	int k, j, j_old;
	int state_count, input_count,output_count;
	if (prflag)fprintf(IP_List, "%5d ", (int)IP_LineNumber);
	/*
	*	Go parse things!
	*/
	//Einlesen
	for (int i = 0; (tok = yylex()) != IP_Token_table["Begin"];i++) {		//Alles vor BEGIN nicht beachten
		i++;
		if (i > 100000) {													//maximal 100000 Zeichen
			fprintf(stderr, "Ihr Header ist zu lang");
			return 0;
		}
	};

	if ((tok = yylex()) == IP_Token_table["DEFSTATE"]) {
		tok = yylex();
		if (tok == ';') {
			state_count = 0;
		}
		for (state_count = 0; tok == IDENTIFIER; state_count++) {
			printf("%c ", yylval.s[0]);
			tok = yylex();
			if (tok == ',') {
				tok = yylex();
				if (tok != IDENTIFIER) {
					fprintf(stderr, "Eingabedaten sind fehlerhaft");
				}
			}
			else if (tok == ';') {
				state_count += 1;
				break;
			}
			else {
				fprintf(stderr, "Eingabedaten sind fehlerhaft");
			}
		}
		
		tok = yylex();
	}
	else {
		fprintf(stderr, "DEFSTATE not found");
	}
	if (tok == IP_Token_table["DEFIN"]) {
		tok = yylex();
		if (tok == ';') {
			input_count = 0;
		}
		for (input_count = 0; tok == IDENTIFIER; input_count++) {
			printf("%c ", yylval.s[0]);
			tok = yylex();
			if (tok == ',') {
				tok = yylex();
				if (tok != IDENTIFIER) {
					fprintf(stderr, "Eingabedaten sind fehlerhaft");
				}
			}
			else if (tok == ';') {
				input_count += 1;
				break;
			}
			else {
				fprintf(stderr, "Eingabedaten sind fehlerhaft");
			}
		}
		tok = yylex();
	}
	else {
		fprintf(stderr, "DEFIN not found");
	}

	if (tok == IP_Token_table["DEFOUT"]) {
		tok = yylex();
		if (tok == ';') {
			output_count = 0;
		}
		for (output_count = 0; tok == IDENTIFIER; output_count++) {
			printf("%c ", yylval.s[0]);
			tok = yylex();
			if (tok == ',') {
				tok = yylex();
				if (tok != IDENTIFIER) {
					fprintf(stderr, "Eingabedaten sind fehlerhaft");
				}
			}
			else if (tok == ';') {
				output_count += 1;
				break;
			}
			else {
				fprintf(stderr, "Eingabedaten sind fehlerhaft");
			}
		}
		tok = yylex();
	}
	else {
		fprintf(stderr, "DEFOUT not found");
	}

	printf("\nAnzahl Zustaende: %d", state_count);
	printf("\nAnzahl Eingangssignale: %d", input_count);
	printf("\nAnzahl Ausgangssignale: %d \n", output_count);

	while (tok == '[')			
	{
		tok = yylex();										//nächstes Token
			
		for (k = 0,j=0; tok == IDENTIFIER; k++,j++)	//j = Anzahl der DEFSTATES
		{
			printf("%c ", yylval.s[0]);					
			tok = yylex();									
			if (tok == ',') {
				tok = yylex();							
			}
			else if(tok == ']'){
				tok = yylex();
				if (tok != '=') {
					fprintf(stderr, "Eingabedaten sind fehlerhaft");
				}
			}
			else {
				fprintf(stderr,"Eingabedaten sind fehlerhaft");
			}
		}
		tok = yylex();
			
		if (tok == '(') {
			tok = yylex();
		}
		else {
			fprintf(stderr, "Eingabedaten sind fehlerhaft");
		}

		j_old = j;											

		for (k = 0, j = 0; tok == INTEGER1 || *yylval.s.c_str() == 'x' || *yylval.s.c_str() == 'X'; k++, j++)
		{
			if (tok == IDENTIFIER) {
				printf("x ");
			}
			else {
				printf("%d ", yylval.i);
			}
			tok = yylex();
			if (tok == ',') {
				tok = yylex();
			}
			else if (tok == ')') {
				tok = yylex();
				if (tok != '(') {
					fprintf(stderr, "Eingabedaten sind fehlerhaft");
				}
			}
			else {
				fprintf(stderr, "Eingabedaten sind fehlerhaft");
			}
		}

		if (j > j_old)
			fprintf(stderr, "Fehlermeldung: Es gibt mehr Werte fuer Eingangssignale als Eingangssignale");		
		else if (j<j_old)
			fprintf(stderr, "Fehlermeldung: Es gibt weniger Werte fuer Eingangssignale als Eingangssignale");		

		tok = yylex();

		for (k = 0,j=0; tok == IDENTIFIER; k++,j++)	
		{
			printf("%c ", yylval.s[0]);

			tok = yylex();

			if (tok == ')') {
				tok = yylex();
				if (tok != '>') {
					fprintf(stderr, "Eingabedaten sind fehlerhaft");
				}
			}
			else{
				fprintf(stderr, "Eingabedaten sind fehlerhaft");
			}
		}
		tok = yylex();

		if (tok == '[') {
			tok = yylex();
		}
		else {
			fprintf(stderr, "Eingabedaten sind fehlerhaft");
		}
			
		for (k = 0,j=0; tok == IDENTIFIER; k++,j++)	
		{
			printf("%c ", yylval.s[0]);
			tok = yylex();
			if (tok == ',') {
				tok = yylex();
			}
			else if (tok == ']') {
				tok = yylex();
				if (tok != ':') {
					fprintf(stderr, "Eingabedaten sind fehlerhaft");
				}
			}
			else {
				fprintf(stderr, "Eingabedaten sind fehlerhaft");
			}
		}

		tok = yylex();

		if (tok == '(') {
			tok = yylex();
		}
		else {
			fprintf(stderr, "Eingabedaten sind fehlerhaft");
		}

		j_old = j;

		for (k = 0, j = 0; tok == INTEGER1 || *yylval.s.c_str() == 'x' || *yylval.s.c_str() == 'X'; k++, j++)
		{
			if (tok == IDENTIFIER) {
				printf("x ");
			}
			else {
				printf("%d ", yylval.i);
			}
			tok = yylex();
			if (tok == ',') {
				tok = yylex();
			}
			else if (tok == ')') {
				tok = yylex();
				if (tok != '(') {
					fprintf(stderr, "Eingabedaten sind fehlerhaft");
				}
			}
			else {
				fprintf(stderr, "Eingabedaten sind fehlerhaft");
			}
		}

		if (j > j_old)
			fprintf(stderr, "Fehlermeldung: Es gibt mehr Werte fuer Ausgangssignale als Ausgangssignale");
		else if (j<j_old)
			fprintf(stderr, "Fehlermeldung: Es gibt weniger Werte fuer Ausgangssignale als Ausgangssignale");

		tok = yylex();

		for (k = 0; tok == IDENTIFIER; k++)
		{
			printf("%c ", yylval.s[0]);
			tok = yylex();
			if (tok == ')') {
				tok = yylex();
				if (tok == 303) {
					break;
				}
				else if (tok == '[') {
					printf("\n"); 
				}
				else
				{
					fprintf(stderr, "Eingabedaten sind fehlerhaft");
				}
			}
			else {
				fprintf(stderr, "Eingabedaten sind fehlerhaft");
			}
		}
	}

	//Überprüfen der Daten 
	

	//while ((tok = yylex()) != 0) {
	//	printf("%d ", tok);
	//	if (tok == STRING1)
	//		printf("%s %s ", IP_revToken_table[tok].c_str(), yylval.s.c_str());
	//	else
	//		if (tok == INTEGER1)
	//			printf("%s %d ", IP_revToken_table[tok].c_str(), yylval.i);
	//		else
	//			if (tok == IDENTIFIER)
	//				printf("%s %s ", IP_revToken_table[tok].c_str(), yylval.s.c_str());
	//			else
	//				if (tok >= TOKENSTART)
	//					printf("%s ", IP_revToken_table[tok].c_str());
	//				else
	//					printf("%c ", tok);
	//	if (!prflag)printf("\n");
	//}

	////Ausgabe
	//FILE *fpout; /* file pointer output file*/
	//char zeichen; /* file pointer output file*/

	//if ((fpout = fopen("myfileout.tbt", "w")) == NULL) {
	//	printf("Cannot open file : myfileout.txt \n");
	//	return 0;
	//}
	//zeichen = 'a';
	//fputc(zeichen, fpout);
	//fclose(fpout);

	return 0;

}
//------------------------------------------------------------------------

/*
 *	Parse Initfile:
 *
 *	  This builds the context tree and then calls the real parser.
 *	It is passed two file streams, the first is where the input comes
 *	from; the second is where error messages get printed.
 */
void CParser::InitParse(FILE *inp,FILE *err,FILE *lst)

{

	/*
	*	Set up the file state to something useful.
	*/
	IP_Input = inp;
	IP_Error = err;
	IP_List  = lst;

	IP_LineNumber = 1;
	ugetflag=0;
	/*
	*	Define both the enabled token and keyword strings.
	*/
	IP_init_token_table();
}
//------------------------------------------------------------------------

/*
 *	yyerror:
 *
 *	  Standard error reporter, it prints out the passed string
 *	preceeded by the current filename and line number.
 */
void CParser::yyerror(char *ers)

{
  fprintf(IP_Error,"line %d: %s\n",IP_LineNumber,ers);
}
//------------------------------------------------------------------------

int CParser::IP_MatchToken(string &tok)
{
	int retval;
	if(	IP_Token_table.find(tok) != IP_Token_table.end()){
		retval = (IP_Token_table[tok]);
	}else{
		retval = (0);
	}
	return retval;
} 

//------------------------------------------------------------------------

/*
 *	yylex:
 *
 */
int CParser::yylex()
{
	//Locals
	int c;
	lexstate s;
	/*
	*	Keep on sucking up characters until we find something which
	*	explicitly forces us out of this function.
	*/
	for (s = L_START,yytext = ""; 1;){
		c = Getc(IP_Input);
		yytext = yytext + (char)c;
		if(!ugetflag) { 
			if(c != EOF)if(prflag)fprintf(IP_List,"%c",c);
		}else ugetflag = 0;
		switch (s){
		  //Starting state, look for something resembling a token.
			case L_START:
				if (isdigit(c)){
				  s = L_INT;
				}else if (isalpha(c) || c == '\\' ){
						s = L_IDENT;
				}else if (isspace(c)){
							if (c == '\n'){
								IP_LineNumber += 1;
								if(prflag)
									fprintf(IP_List,"%5d ",(int)IP_LineNumber);
							}
							yytext = "";
				}else if (c == '/'){
							yytext = "";
							s = L_COMMENT;
				}else if (c == '"'){
							s = L_STRING;
							yylval.s="";
				}else if (c == EOF){
							return ('\0');
				}else{						
							return (c);
				}
			break;

			case L_COMMENT:
				if (c == '/') 
					s = L_LINE_COMMENT;
				else	if(c == '*')
							s = L_TEXT_COMMENT;
						else{
								Ungetc(c);
								return('/');	/* its the division operator not a comment */
							}
			break;
			case L_LINE_COMMENT:
				if ( c == '\n'){
					s = L_START;
					Ungetc(c);
				}
				yytext = "";
			break;
			case L_TEXT_COMMENT:
				if ( c == '\n'){
					IP_LineNumber += 1;
				}else if (c == '*')
							s = L_END_TEXT_COMMENT;
				yytext = "";
			break;
			case L_END_TEXT_COMMENT:
				if (c == '/'){
					s = L_START;
				}else{
					s = L_TEXT_COMMENT;
				}
				yytext = "";
			break;

		  /*
		   *	Suck up the integer digits.
		   */
			case L_INT:
				if (isdigit(c)){
				  break;
				}else {
					Ungetc(c);
					yylval.s = yytext.substr(0,yytext.size()-1);
					yylval.i = atoi(yylval.s.c_str());
					return (INTEGER1);
				}
			break;

		  /*
		   *	Grab an identifier, see if the current context enables
		   *	it with a specific token value.
		   */
			
			case L_IDENT:
			   if (isalpha(c) || isdigit(c) || c == '_')
				  break;
				Ungetc(c);
				yytext = yytext.substr(0,yytext.size()-1);
				yylval.s = yytext;
				if (c = IP_MatchToken(yytext)){
					return (c);
				}else{
					return (IDENTIFIER);
				}

		   /*
		   *	Suck up string characters but once resolved they should
		   *	be deposited in the string bucket because they can be
		   *	arbitrarily long.
		   */
			case L_STRING2:
				s = L_STRING;
				if(c == '"'){// >\"< found
					PushString((char)c);
				}else{
					if(c == '\\'){// >\\< found
						PushString((char)c);
					}else{
						PushString((char)'\\');// >\x< found
						PushString((char)c);
					}
				}
			break;
			case L_STRING:
				if (c == '\n')
				  IP_LineNumber += 1;
				else if (c == '\r')
						;
					 else	if (c == '"' || c == EOF){
								return (STRING1);
							} else	if(c=='\\'){
										s = L_STRING2;
										//PushString((char)c);
									}else
										PushString((char)c);
			break;
			default: printf("***Fatal Error*** Wrong case label in yylex\n");
		}
	}
}
//------------------------------------------------------------------------

int main(int argc, char* argv[])
{
	FILE *inf;
	char fistr[100];
	printf("Enter .txt filename:\n");
	//scanf("%s",fistr);//gets(fistr);
	//inf = fopen(strcat(fistr,".txt"),"r");
	inf = fopen("in2.txt", "r");
	if(inf==NULL){
		printf("Cannot open input file %s\n",fistr);
		return 0;
	}
	CParser obj;
	obj.InitParse(inf,stderr,stdout);
//	obj.pr_tokentable();
	obj.yyparse();
	char c; cin>>c;

	return 0;
}



