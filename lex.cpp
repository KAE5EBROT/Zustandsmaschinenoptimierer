
#include "stdafx.h"
#include "lex.h"
#include <iostream>

#pragma warning(disable:4786)


// Adds a character to the string value
void CParser::PushString(char c)
{
	yylval.s += c;
}
//------------------------------------------------------------------------
void CParser::Load_tokenentry(string str, int index)
{
	IP_Token_table[str] = index;
	IP_revToken_table[index] = str;
}
void CParser::IP_init_token_table()
{
	Load_tokenentry("STRING1", 3);
	Load_tokenentry("IDENTIFIER", 4);
	Load_tokenentry("INTEGER1", 5);

	int ii = TOKENSTART;
	Load_tokenentry("AND", ii++);
	Load_tokenentry("OR", ii++);
	Load_tokenentry("Begin", ii++);
	Load_tokenentry("End", ii++);
	Load_tokenentry("DEFSTATE", ii++);
	Load_tokenentry("DEFIN", ii++);
	Load_tokenentry("DEFOUT", ii++);
}
//------------------------------------------------------------------------

void CParser::pr_tokentable()
{

	typedef map<string, int>::const_iterator CI;
	const char* buf;

	printf("Symbol Table ---------------------------------------------\n");

	for (CI p = IP_Token_table.begin(); p != IP_Token_table.end(); ++p) {
		buf = p->first.c_str();
		printf(" key:%s", buf);
		printf(" val:%d\n", p->second);;
	}
}
//------------------------------------------------------------------------

int	CParser::yyparse()
{
	int tok = 0;
	parstates state = P_HEADER;
	if (prflag)fprintf(IP_List, "%5d ", (int)IP_LineNumber);
	/*
	*	Go parse things!
	*/
	//Einlesen
	while (tok != IP_Token_table["End"]) {
		switch (state) {
		case P_HEADER:
			tok = yylex();
			state = pfSkipHeader(tok);		//Alles vor BEGIN nicht beachten
			break;
		case P_DEFSELECT:
			state = pfGetDef(tok);
			break;
		case P_DEFSTATE:
			tok = yylex();
			state = pfScanState(tok);
			break;
		case P_DEFIN:
			tok = yylex();
			state = pfScanInputs(tok);
			break;
		case P_DEFOUT:
			tok = yylex();
			state = pfScanOutputs(tok);
			break;
		case P_READLINE:
			state = pfReadLine(tok);
			break;
		case P_ERROR:
			state = pfScanOutputs(tok);
			break;
		}
	}

	printf("\nAnzahl Zustaende: %d", state_count);
	printf("\nAnzahl Eingangssignale: %d", input_count);
	printf("\nAnzahl Ausgangssignale: %d \n", output_count);
	table.print();

	// Codierungsoptimierung

	//high priority:

	typedef map<string, vector<string>> priority;
	priority high_priority;		//high priority is when at least two states have the same next state by the same input value
	vector<string> t;
	int c = 0;

	for (int j = 0; j < table.iheight; j++) {
		for (int i = 0; i < table.iheight; i++) {
			if (table.istates.at(j).c_str() != table.istates.at(i).c_str()) {						//checks, if row state unequal nextstate	
				if (table.istates.at(j) == table.table[table.istates.at(i)].at(0).next_state) {
					c++;
					t.push_back(table.istates.at(i).c_str());										//push a candidate for high priority into t
				}
			}
		}
		if (c >= 2) {
			high_priority[table.istates.at(j).c_str()] = t;											//if there are more canditates than one, then the high priority map is passed t
		}
		t.clear();
		c = 0;
	}

	//mean priority:

	priority mean_priority;		// mean priority is when a state have at least two different next states by different input values
	c = 0;
	int z = 0;
	bool nextstate_unequal = true;

	for (int j = 0; j < table.iheight; j++) {
		for (int i = 0; i < table.iwidth; i++) {					//checks, row for row of mean priority
			for (int k = 0; k < table.iwidth; k++) {
				if (table.table[table.istates.at(j)].at(i).next_state == table.table[table.istates.at(j)].at(k).next_state && k != i) {
					nextstate_unequal = false;					//checks, if the next state is equal of a other next state by a other input value 
				}
			}
			if (nextstate_unequal == true) {
				c++;
				t.push_back(table.table[table.istates.at(j).c_str()].at(i).next_state);
			}
			nextstate_unequal = true;
		}
		if (c >= 2) {
			mean_priority[table.istates.at(j).c_str()] = t;									//if there are more canditates than one, then the mean priority map is passed t
		}
		t.clear();
		c = 0;
	}

	//lowest priority:

	priority lowest_priority;		//lowest priority is given when at least two states have the same output behaviour
	c = 0;							//c counts the states with the same output behaviour 	
	bool output_matched = false;
	int outputcount = 1;
	int pos;
	int val;
	int tt;
	string t1, t2;

	for (int i = 1; i < table.ioutputs.size() + 1; i++) outputcount *= 2;						//outputcount = number of output value possibilties

	for (int j = 0; j < table.iwidth; j++) {													//check all input possibilities
		for (int k = 0; k < outputcount; k++) {													//check all output possibilities (00 01 10 11 -> 0 1 2 3) 
			for (int i = 0; i < table.iheight; i++) {
				t1 = table.table[table.istates.at(i)].at(j).out_list;							//save output string
				pos = 1;
				val = 0;

				if (t1.find("x", 0) != -1) {													//checks, if a "x" is in t1
					pos = t1.size();
					while (pos >= 0) {
						pos = t1.rfind("1", pos);												//search backwards after a "1" 
						if (pos >= 0) {
							val += 1 << (t1.size() - pos - 1);									//calculate decimal value of all "1" in binary code 
							pos = pos - 1;
						}
					}
					val -= k;																	//val= decimal value (all "1" in binary code) - one output possibility															 
					if (val < 0)
					{
						pos = (t1.size())- 1;
						while (pos>=0) {
							pos = t1.rfind("x", pos);						
							tt = val;															//save val
							val += 1 << pos;
							if (val > 0) {
								val = tt;														//the "1" value (2^n) to high, take the old value tt
							}
							else if (val == 0) {
								output_matched = true;											//matched e.g. "01" --> "0x" 
								pos = -1;
							}
							pos -= 1;
						}
					}
					else if (val == 0) {
						output_matched = true;
					}
					else {
						output_matched = false;
					}
				}

				if (output_matched == true) {
					c++;
					t.push_back(table.istates.at(i).c_str());							//push candidate state in t 
					output_matched = false;
				}
				else if (t1.find("x", 0) == -1)  
				{
					t2 = table.table[table.istates.at(i)].at(j).out_list;			
					pos = t2.size();													
					while (pos >= 0) {													//binary string to int (output binary string -> int)
						pos = t2.rfind("1", pos);
						if (pos >= 0) {
							val += 1 << (t2.size() - pos - 1);
							pos = pos - 1;
						}
					}

					if (val == k) {   //checks, if output value of the current state is equal with one output possibility
						c++;
						t.push_back(table.istates.at(i).c_str());							//push candidate state in t 
						output_matched = false;
					}
				}
			}
			if (c >= 2) {
				lowest_priority[to_string(z)] = t;											//if there are more canditates than one, then the lowest priority map is passed t
				z += 1;
			}
			t.clear();
			c = 0;
		}	
	}

	return 0;

}	

//------------------------------------------------------------------------

/*
*	Parse Initfile:
*
*	 This builds the context tree and then calls the real parser.
*	It is passed two file streams, the first is where the input comes
*	from; the second is where error messages get printed.
*/
void CParser::InitParse(FILE *inp, FILE *err, FILE *lst)

{

	/*
	*	Set up the file state to something useful.
	*/
	IP_Input = inp;
	IP_Error = err;
	IP_List = lst;

	IP_LineNumber = 1;
	ugetflag = 0;
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
	fprintf(IP_Error, "line %d: %s\n", IP_LineNumber, ers);
}
//------------------------------------------------------------------------

int CParser::IP_MatchToken(string &tok)
{
	int retval;
	if (IP_Token_table.find(tok) != IP_Token_table.end()) {
		retval = (IP_Token_table[tok]);
	}
	else {
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
	for (s = L_START, yytext = ""; 1;) {
		c = Getc(IP_Input);
		yytext = yytext + (char)c;
		if (!ugetflag) {
			if (c != EOF)if (prflag)fprintf(IP_List, "%c", c);
		}
		else ugetflag = 0;
		switch (s) {
			//Starting state, look for something resembling a token.
		case L_START:
			if (isdigit(c)) {
				s = L_INT;
			}
			else if (isalpha(c) || c == '\\') {
				s = L_IDENT;
			}
			else if (isspace(c)) {
				if (c == '\n') {
					IP_LineNumber += 1;
					if (prflag)
						fprintf(IP_List, "%5d ", (int)IP_LineNumber);
				}
				yytext = "";
			}
			else if (c == '/') {
				yytext = "";
				s = L_COMMENT;
			}
			else if (c == '"') {
				s = L_STRING;
				yylval.s = "";
			}
			else if (c == EOF) {
				return ('\0');
			}
			else {
				return (c);
			}
			break;

		case L_COMMENT:
			if (c == '/')
				s = L_LINE_COMMENT;
			else	if (c == '*')
				s = L_TEXT_COMMENT;
			else {
				Ungetc(c);
				return('/');	/* its the division operator not a comment */
			}
			break;
		case L_LINE_COMMENT:
			if (c == '\n') {
				s = L_START;
				Ungetc(c);
			}
			yytext = "";
			break;
		case L_TEXT_COMMENT:
			if (c == '\n') {
				IP_LineNumber += 1;
			}
			else if (c == '*')
				s = L_END_TEXT_COMMENT;
			yytext = "";
			break;
		case L_END_TEXT_COMMENT:
			if (c == '/') {
				s = L_START;
			}
			else {
				s = L_TEXT_COMMENT;
			}
			yytext = "";
			break;

			/*
			*	Suck up the integer digits.
			*/
		case L_INT:
			if (isdigit(c)) {
				break;
			}
			else {
				Ungetc(c);
				yylval.s = yytext.substr(0, yytext.size() - 1);
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
			yytext = yytext.substr(0, yytext.size() - 1);
			yylval.s = yytext;
			if (c = IP_MatchToken(yytext)) {
				return (c);
			}
			else {
				return (IDENTIFIER);
			}

			/*
			*	Suck up string characters but once resolved they should
			*	be deposited in the string bucket because they can be
			*	arbitrarily long.
			*/
		case L_STRING2:
			s = L_STRING;
			if (c == '"') {// >\"< found
				PushString((char)c);
			}
			else {
				if (c == '\\') {// >\\< found
					PushString((char)c);
				}
				else {
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
			else	if (c == '"' || c == EOF) {
				return (STRING1);
			}
			else	if (c == '\\') {
				s = L_STRING2;
				//PushString((char)c);
			}
			else
				PushString((char)c);
			break;
		default: printf("***Fatal Error*** Wrong case label in yylex\n");
		}
	}
}
//------------------------------------------------------------------------


CParser::parstates CParser::pfSkipHeader(int &tok)
{
	parstates retval;
	static int i = 0;
	i++;
	if (i > 100000) {										//maximal 100000 Zeichen
		fprintf(stderr, "Ihr Header ist zu lang");
		retval = P_ERROR;
	}
	else if (tok != IP_Token_table["Begin"]) {
		retval = P_HEADER;
	}
	else {
		retval = P_DEFSELECT;
	}
	return retval;
}

CParser::parstates CParser::pfGetDef(int &tok)
{
	parstates retval;
	static int i = 0;

	if (defScanned.states && defScanned.inputs && defScanned.outputs) {
		table.init(scannedStates, scannedInputs, scannedOutputs);//printf("\n"); //todo vielleicht entfernen, nur zur Ausgabe in Konsole
		retval = P_READLINE;
	}
	else if ((((tok = yylex()) < IP_Token_table["DEFSTATE"]) || (tok > IP_Token_table["DEFOUT"])) && (i<1000)) {
		i++;
		retval = P_DEFSELECT;
	}
	else if ((tok == IP_Token_table["DEFSTATE"]) && !defScanned.states) {
		retval = P_DEFSTATE;
	}
	else if ((tok == IP_Token_table["DEFIN"]) && !defScanned.inputs) {
		retval = P_DEFIN;
	}
	else if ((tok == IP_Token_table["DEFOUT"]) && !defScanned.outputs) {
		retval = P_DEFOUT;
	}
	else {
		retval = P_ERROR;
	}
	return retval;
}

CParser::parstates CParser::pfScanState(int &tok)
{
	parstates retval;
	switch (tok) {
	case IDENTIFIERDEF:
		scannedStates.push_back(string(yylval.s));//printf("%c ", yylval.s[0]);//todo abspeichern
		state_count++;
		retval = P_DEFSTATE;
		break;
	case ',':
		retval = P_DEFSTATE;
		break;
	case ';':
		defScanned.states = true;
		retval = P_DEFSELECT;
		break;
	default:
		fprintf(stderr, "Eingabedaten sind fehlerhaft");
		break;
	}
	return retval;
}

CParser::parstates CParser::pfScanInputs(int &tok)
{
	parstates retval;
	switch (tok) {
	case IDENTIFIERDEF:
		scannedInputs.push_back(string(yylval.s));//printf("%c ", yylval.s[0]);//todo abspeichern
		input_count++;
		retval = P_DEFIN;
		break;
	case ',':
		retval = P_DEFIN;
		break;
	case ';':
		defScanned.inputs = true;
		retval = P_DEFSELECT;
		break;
	default:
		fprintf(stderr, "Eingabedaten sind fehlerhaft");
		break;
	}
	return retval;
}

CParser::parstates CParser::pfScanOutputs(int &tok)
{
	parstates retval;
	switch (tok) {
	case IDENTIFIERDEF:
		scannedOutputs.push_back(string(yylval.s));//printf("%c ", yylval.s[0]);//todo abspeichern
		output_count++;
		retval = P_DEFOUT;
		break;
	case ',':
		retval = P_DEFOUT;
		break;
	case ';':
		defScanned.outputs = true;
		retval = P_DEFSELECT;
		break;
	default:
		fprintf(stderr, "Eingabedaten sind fehlerhaft");
		break;
	}
	return retval;
}

CParser::parstates CParser::pfReadLine(int &tok)
{
	parstates retval = P_READLINE;
	int k, j, j_old;
	smtable::elementlist inputs;
	string invals;
	string srcstate;
	smtable::elementlist outputs;
	string outvals;
	string dststate;
	while (tok != '[')tok = yylex();
	tok = yylex();									//erste Eingangsvariable

	for (k = 0, j = 0; tok == IDENTIFIER; k++, j++)	//j = Anzahl der DEFSTATES
	{
		inputs.push_back(string(yylval.s));// printf("%c ", yylval.s[0]);					//todo abspeichern
		tok = yylex();
		if (tok == ',') {
			tok = yylex();
		}
		else if (tok == ']') {
			tok = yylex();
			if (tok != '=') {
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

	for (k = 0, j = 0; tok == INTEGER1 || (*yylval.s.c_str() == 'x' && tok == IDENTIFIER) || (*yylval.s.c_str() == 'X' && tok == IDENTIFIER); k++, j++)
	{
		if (tok == IDENTIFIER) {
			invals.append("x");// printf("x ");
		}
		else {
			if(yylval.i)
				invals.append("1");// printf("%d ", yylval.i);
			else
				invals.append("0");
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
	else if (j < j_old)
		fprintf(stderr, "Fehlermeldung: Es gibt weniger Werte fuer Eingangssignale als Eingangssignale");

	tok = yylex();

	for (k = 0, j = 0; tok == IDENTIFIER; k++, j++)
	{
		srcstate.append(yylval.s);// printf("%c ", yylval.s[0]);

		tok = yylex();

		if (tok == ')') {
			tok = yylex();
			if (tok != '>') {
				fprintf(stderr, "Eingabedaten sind fehlerhaft");
			}
		}
		else {
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

	for (k = 0, j = 0; tok == IDENTIFIER; k++, j++)
	{
		outputs.push_back(string(yylval.s));// printf("%c ", yylval.s[0]);
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

	for (k = 0, j = 0; tok == INTEGER1 || (*yylval.s.c_str() == 'x' && tok == IDENTIFIER) || (*yylval.s.c_str() == 'X' && tok == IDENTIFIER); k++, j++)
	{
		if (tok == IDENTIFIER) {
			outvals.append("x");// printf("x ");
		}
		else {
			if (yylval.i)
				outvals.append("1");// printf("%d ", yylval.i);
			else
				outvals.append("0");
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
	else if (j < j_old)
		fprintf(stderr, "Fehlermeldung: Es gibt weniger Werte fuer Ausgangssignale als Ausgangssignale");

	tok = yylex();

	for (k = 0; tok == IDENTIFIER; k++)
	{
		dststate.append(yylval.s);// printf("%c ", yylval.s[0]);
		tok = yylex();
		if (tok == ')') {
			tok = yylex();
			if (tok == 303) {
				break;
			}
			else if (tok == '[') {
				//printf("\n");
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
	table.link(inputs, invals, srcstate, outputs, outvals, dststate);
	return retval;
}