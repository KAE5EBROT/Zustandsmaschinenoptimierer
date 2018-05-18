#pragma once
#include <stdio.h>
#include <iostream>
#include <string>
#include <map>
#include <algorithm>
#include <fstream>
#include "Zustandsmaschinentabelle\smtable.h"

using namespace std;
typedef unsigned int uint;

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
		P_HEADER,				/* Skip everything until "Begin"						*/
		P_DEFSELECT,			/* Select which definition to scan						*/
		P_DEFSTATE,				/* Read in state definition								*/
		P_DEFIN,				/* Read in input definition								*/
		P_DEFOUT,				/* Read in output definition							*/
		P_READLINEINPUTS,		/* Read in inputs of transition definition				*/
		P_READLINEINVALS,		/* Read in input values of transition definition		*/
		P_READLINESSTATE,		/* Read in source state of transition definition		*/
		P_READLINEOUTPUTS,		/* Read in outputs of transition definition				*/
		P_READLINEOUTVALS,		/* Read in output values of transition definition		*/
		P_READLINEDSTATE,		/* Read in destination state of transition definition	*/
		P_ERROR					/* Error handler										*/
	};
	typedef struct {
		bool states = false;
		bool inputs = false;
		bool outputs = false;
		smtable::elementlist scannedStates;
		smtable::elementlist scannedInputs;
		smtable::elementlist scannedOutputs;
	} defScanType;
	/**
	*  \brief function returns
	*  
	*  \input 
	*/
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
	map<string, int> IP_Token_table;			//Tokendefinitions
	map<int, string> IP_revToken_table;			//reverse Tokendefinitions

	int CParser::yylex();						//lexial analyser
	void CParser::yyerror(char *ers);			//error reporter
	int CParser::IP_MatchToken(string &tok);	//checks the token
	void CParser::InitParse(FILE *inp, FILE *err, FILE *lst);
	int	CParser::yyparse(smtable &table);		//parser
	void CParser::pr_tokentable();				//test output for tokens
	void CParser::IP_init_token_table();		//loads the tokens
	void CParser::Load_tokenentry(string str, int index);//load one token
	void CParser::PushString(char c);			//Used for dtring assembly
	CParser::parstates CParser::pfSkipHeader(const int tok);
	CParser::parstates CParser::pfGetDef(const int tok, smtable &table, defScanType &defScanned);
	CParser::parstates CParser::pfScanState(const int tok, defScanType &defScanned);
	CParser::parstates CParser::pfScanInputs(const int tok, defScanType &defScanned);
	CParser::parstates CParser::pfScanOutputs(const int tok, defScanType &defScanned);
	CParser::parstates CParser::pfReadLineInputs(const int tok, smtable::elementlist &inlist);
	CParser::parstates CParser::pfReadLineInvals(const int tok, string &invals);
	CParser::parstates CParser::pfReadLineSState(const int tok, string &srcstate);
	CParser::parstates CParser::pfReadLineOutputs(const int tok, smtable::elementlist &outlist);
	CParser::parstates CParser::pfReadLineOutvals(const int tok, string &outvals);
	CParser::parstates CParser::pfReadLineDState(const int tok, string &dststate);
	CParser::parstates CParser::pfError(void);
	CParser() { IP_LineNumber = 1; ugetflag = 0; prflag = 0; };	//Constructor
};
//------------------------------------------------------------------------