#pragma once
//#include "StdAfx.h"
#include <stdio.h>
#include <iostream>
#include <string>
#include <map>
using namespace std;

#define	Getc(s)			getc(s)
#define	Ungetc(c)		{ungetc(c,IP_Input); ugetflag=1;}
#define STRING1DEF 3
#define IDENTIFIERDEF 4
#define INTEGER1DEF 5
#define TOKENSTARTDEF 300

class CParser
{
public:

	const int STRING1 = STRING1DEF;
	const int IDENTIFIER = IDENTIFIERDEF;
	const int INTEGER1 = INTEGER1DEF;
	const int TOKENSTART = TOKENSTARTDEF;
	int state_count = 0;
	int input_count = 0;
	int output_count = 0;
	struct {
		bool states = false;
		bool inputs = false;
		bool outputs = false;
	} defScanned;
	string yytext;								//input buffer

	/*
	*	Lexical analyzer states.
	*/
	enum lexstate {
		L_START, L_INT, L_IDENT, L_STRING, L_STRING2,
		L_COMMENT, L_TEXT_COMMENT, L_LINE_COMMENT, L_END_TEXT_COMMENT
	};

	/*
	*	Parser states
	*/
	enum parstates {
		P_HEADER, P_DEFSELECT, P_DEFSTATE, P_DEFIN, P_DEFOUT, P_READLINE, P_ERROR
	};
	struct tyylval {								//value return
		string s;								//structure
		int i;
	}yylval;
	FILE *IP_Input;								//Input File
	FILE *IP_Error;								//Error Output
	FILE *IP_List;								//List Output
	int  IP_LineNumber;							//Line counter
	int ugetflag;								//checks ungets
	int prflag;									//controls printing
	map<string, int> IP_Token_table;				//Tokendefinitions
	map<int, string> IP_revToken_table;			//reverse Tokendefinitions

	int CParser::yylex();						//lexial analyser
	void CParser::yyerror(char *ers);			//error reporter
	int CParser::IP_MatchToken(string &tok);	//checks the token
	void CParser::InitParse(FILE *inp, FILE *err, FILE *lst);
	int	CParser::yyparse();						//parser
	void CParser::pr_tokentable();				//test output for tokens
	void CParser::IP_init_token_table();		//loads the tokens
	void CParser::Load_tokenentry(string str, int index);//load one token
	void CParser::PushString(char c);			//Used for dtring assembly
	CParser::parstates CParser::pfSkipHeader(int &tok);
	CParser::parstates CParser::pfGetDef(int & tok);
	CParser::parstates CParser::pfScanState(int & tok);
	CParser::parstates CParser::pfScanInputs(int & tok);
	CParser::parstates CParser::pfScanOutputs(int & tok);
	CParser::parstates CParser::pfReadLine(int & tok);
	CParser() { IP_LineNumber = 1; ugetflag = 0; prflag = 0; };	//Constructor
};
//------------------------------------------------------------------------