
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
		printf(" val:%d\n", p->second);
	}
}
//------------------------------------------------------------------------
															/*----------------------------------------------*/
int	CParser::yyparse(smtable &table)						/*												*/
{															/*												*/
	int tok = 0;											/* current token								*/
	parstates state = P_HEADER;								/* state machine variable						*/
	defScanType defScanned;									/* variable combination to remember read defines*/
	smtable::elementlist inputs;							/* list of mentioned inputs						*/
	string invals;											/* string of associated input trigger values	*/
	string srcstate;										/* source state of transition					*/
	smtable::elementlist outputs;							/* list of mentioned outputs					*/
	string outvals;											/* string of associated output values			*/
	string dststate;										/* destination state of transition				*/
															/*												*/
	while ((tok = yylex()) != IP_Token_table["End"]) {		/* Run till End									*/
		switch (state) {									/*												*/
		case P_HEADER:										/* Skip everything until "Begin"				*/
			state = pfSkipHeader(tok);						/*												*/
			break;											/*												*/
		case P_DEFSELECT:									/* Which definition is in queue?				*/
			state = pfGetDef(tok, table, defScanned);		/*												*/
			break;											/*												*/
		case P_DEFSTATE:									/* read in state definitions					*/
			state = pfScanState(tok, defScanned);			/*												*/
			break;											/*												*/
		case P_DEFIN:										/* read in input definitions					*/
			state = pfScanInputs(tok, defScanned);			/*												*/
			break;											/*												*/
		case P_DEFOUT:										/* read in output definitions					*/
			state = pfScanOutputs(tok, defScanned);			/*												*/
			break;											/*												*/
		case P_READLINEINPUTS:								/* read inputs of state transition definition	*/
			state = pfReadLineInputs(tok, inputs);			/*												*/
			break;											/* 												*/
		case P_READLINEINVALS:								/* read invalues of state transition definition	*/
			state = pfReadLineInvals(tok, invals);			/* 												*/
			break;											/* 												*/
		case P_READLINESSTATE:								/* read initial state of transition definition	*/
			state = pfReadLineSState(tok, srcstate);		/* 												*/
			break;											/* 												*/
		case P_READLINEOUTPUTS:								/* read inputs of state transition definition	*/
			state = pfReadLineOutputs(tok, outputs);		/* 												*/
			break;											/* 												*/
		case P_READLINEOUTVALS:								/* read inputs of state transition definition	*/
			state = pfReadLineOutvals(tok, outvals);		/* 												*/
			break;											/* 												*/
		case P_READLINEDSTATE:								/* read following state of transition definition*/
			state = pfReadLineDState(tok, dststate);		/* 												*/
			if (state == P_READLINEINPUTS){					/* following state read successfully			*/
				table.link(inputs, invals, srcstate, outputs, outvals, dststate);/*							*/
				inputs.clear();								/* clear variables for next line to process		*/
				invals.clear();								/* ^											*/
				srcstate.clear();							/* ^											*/
				outputs.clear();							/* ^											*/
				outvals.clear();							/* ^											*/
				dststate.clear();							/* ^											*/
			}												/*												*/
			break;											/* 												*/
		case P_ERROR:										/* catch error									*/
			state = pfError();								/* 												*/
			break;											/* 												*/
		}													/* 												*/
	}														/* 												*/
															/* 												*/
	printf("\nAnzahl Zustaende: %d", table.istates.size());	/* 												*/
	printf("\nAnzahl Eingangssignale: %d", table.iinputs.size());/* 										*/
	printf("\nAnzahl Ausgangssignale: %d \n", table.ioutputs.size());/* 									*/
															/*												*/
	return (state == P_ERROR) ? 1 : 0;						/* map error to return							*/
}															/*----------------------------------------------*/

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

/*!
* \brief Skip documentation
*
* Ignores everything until "Begin".
*
* \param[in] tok Current token to check for "Begin"
* \param[out] none
* \return Next state to enter
* \note Global variables used: none
*/															/*----------------------------------------------*/
CParser::parstates CParser::pfSkipHeader(const int tok)		/* Parser function: Skip until begin			*/
{															/*												*/
	parstates retval;										/*												*/
	static int i = 0;										/*												*/
	i++;													/*												*/
	if (i > 100000) {										/* maximum of 100000 elements					*/
		fprintf(stderr, "\nIhr Header ist zu lang");		/*												*/
		retval = P_ERROR;									/*												*/
	}														/*												*/
	else if (tok != IP_Token_table["Begin"]) {				/* still no "Begin"								*/
		retval = P_HEADER;									/*												*/
	}														/*												*/
	else {													/* "Begin" found! definitions are next			*/
		retval = P_DEFSELECT;								/*												*/
	}														/*												*/
	return retval;											/*												*/
}															/*												*/
															/*----------------------------------------------*/
/*!
* \brief Select next definition
*
* At the beginning of each state machine to read, states, inputs and outputs have to be defined first.
* To make things more flexible, this sequence is not fixed. This function selects which definition comes next.
*
* \param[in] tok Current token to check for definition key
* \param[out] none
* \return Next state to enter
* \note Global variables used: none
*/															/*----------------------------------------------*/
CParser::parstates CParser::pfGetDef(const int tok, smtable &table, defScanType &defScanned)/*				*/
{															/* Parser function: Select next definition		*/
	parstates retval;										/*												*/
	static int i = 0;										/*												*/
															/*												*/
	if (defScanned.states && defScanned.inputs && defScanned.outputs) { /* everything defined! Now state machine definitions! */
		table.init(defScanned.scannedStates, defScanned.scannedInputs, defScanned.scannedOutputs);	/* let the table know the definitions	*/
		retval = P_READLINEINPUTS;							/*												*/
	}														/*												*/
	else if (((tok < IP_Token_table["DEFSTATE"]) || (tok > IP_Token_table["DEFOUT"])) && (i<1000)) {/*		*/
		i++;												/* still no definition key? skip				*/
		retval = P_DEFSELECT;								/*												*/
	}														/*												*/
	else if ((tok == IP_Token_table["DEFSTATE"]) && !defScanned.states) {/*									*/
		retval = P_DEFSTATE;								/* DEFSTATE found								*/
	}														/*												*/
	else if ((tok == IP_Token_table["DEFIN"]) && !defScanned.inputs) {/*									*/
		retval = P_DEFIN;									/* DEFIN found									*/
	}														/*												*/
	else if ((tok == IP_Token_table["DEFOUT"]) && !defScanned.outputs) {/*									*/
		retval = P_DEFOUT;									/* DEFOUT found									*/
	}														/*												*/
	else {													/* 1000 keys reached and still not defined?		*/
		retval = P_ERROR;									/*												*/
	}														/*												*/
	return retval;											/*												*/
}															/*												*/
															/*----------------------------------------------*/
/*!
* \brief Scan state definitions
*
* Read in defined states and save in list.
*
* \param[in] tok Current token to check for name
* \param[out] none
* \return Next state to enter
* \note Global variables used: CParser::scannedStates
*/															/*----------------------------------------------*/
CParser::parstates CParser::pfScanState(const int tok, defScanType &defScanned )		/* Parser function: Read in state definition	*/
{															/*												*/
	parstates retval = P_DEFSTATE;							/*												*/
	switch (tok) {											/*												*/
	case IDENTIFIERDEF:										/* valid name									*/
		defScanned.scannedStates.push_back(string(yylval.s));/* save										*/
		retval = P_DEFSTATE;								/*												*/
		break;												/*												*/
	case ',':												/* skip separator								*/
		retval = P_DEFSTATE;								/*												*/
		break;												/*												*/
	case ';':												/* end of DEFSTATE								*/
		defScanned.states = true;							/* mark finished scan							*/
		retval = P_DEFSELECT;								/* return to selection							*/
		break;												/*												*/
	default:												/* anything else scanned?						*/
		fprintf(stderr, "\nEingabedaten sind fehlerhaft (DEFSTATE)");/*										*/
		break;												/*												*/
	}														/*												*/
	return retval;											/*												*/
}															/*												*/
															/*----------------------------------------------*/
/*!
* \brief Scan input definitions
*
* Read in defined inputs and save in list.
*
* \param[in] tok Current token to check for name
* \param[out] none
* \return Next state to enter
* \note Global variables used: CParser::scannedInputs
*/															/*----------------------------------------------*/
CParser::parstates CParser::pfScanInputs(const int tok, defScanType &defScanned)		/* Parser function: Read in input definition	*/
{															/*												*/
	parstates retval = P_DEFIN;								/*												*/
	switch (tok) {											/*												*/
	case IDENTIFIERDEF:										/* valid name									*/
		defScanned.scannedInputs.push_back(string(yylval.s));/* save										*/
		retval = P_DEFIN;									/*												*/
		break;												/*												*/
	case ',':												/* skip separator								*/
		retval = P_DEFIN;									/*												*/
		break;												/*												*/
	case ';':												/* end of DEFIN									*/
		defScanned.inputs = true;							/* mark finished scan							*/
		retval = P_DEFSELECT;								/* return to selection							*/
		break;												/*												*/
	default:												/* anything else scanned?						*/
		fprintf(stderr, "\nEingabedaten sind fehlerhaft (DEFIN)");/*										*/
		break;												/*												*/
	}														/*												*/
	return retval;											/*												*/
}															/*												*/
															/*----------------------------------------------*/
/*!
* \brief Scan output definitions
*
* Read in defined outputs and save in list.
*
* \param[in] tok Current token to check for name
* \param[out] none
* \return Next state to enter
* \note Global variables used: CParser::scannedOutputs
*/															/*----------------------------------------------*/
CParser::parstates CParser::pfScanOutputs(const int tok, defScanType &defScanned)/* Parser function: Read in output definition*/
{															/*												*/
	parstates retval = P_DEFOUT;							/*												*/
	switch (tok) {											/*												*/
	case IDENTIFIERDEF:										/* valid name									*/
		defScanned.scannedOutputs.push_back(string(yylval.s));/* save										*/
		retval = P_DEFOUT;									/*												*/
		break;												/*												*/
	case ',':												/* skip separator								*/
		retval = P_DEFOUT;									/*												*/
		break;												/*												*/
	case ';':												/* end of DEFOUT								*/
		defScanned.outputs = true;							/* mark finished scan							*/
		retval = P_DEFSELECT;								/* return to selection							*/
		break;												/*												*/
	default:												/* anything else scanned?						*/
		fprintf(stderr, "\nEingabedaten sind fehlerhaft (DEFOUT)");/*										*/
		break;												/*												*/
	}														/*												*/
	return retval;											/*												*/
}															/*												*/
															/*----------------------------------------------*/
/*!
* \brief Read transition definition: inputs
*
* Read in inputs to be defined for transition to trigger.
*
* \param[in] tok Current token to check for input
* \param[in,out] List of mentioned inputs with current one appended
* \return Next state to enter
* \note Global variables used: none
*/															/*----------------------------------------------*/
CParser::parstates CParser::pfReadLineInputs(const int tok, smtable::elementlist &inlist)/*					*/
{															/*												*/
	parstates retval = P_READLINEINPUTS;					/*												*/
	switch (tok) {											/*												*/
	case '[':												/* start of line								*/
		break;												/* no operation									*/
	case ',':												/* separator									*/
		break;												/*												*/
	case IDENTIFIERDEF:										/* name of input detected						*/
		inlist.push_back(string(yylval.s));					/* save for linking until everything is read	*/
		break;												/*												*/
	case ']':												/* end of input names							*/
		retval = P_READLINEINVALS;							/* read values next								*/
		break;												/*												*/
	default:												/* none of expected tokens read					*/
		retval = P_ERROR;									/* stop operation								*/
		break;												/*												*/
	}														/*												*/
	return retval;											/*												*/
}															/*												*/
															/*----------------------------------------------*/
/*!
* \brief Read transition definition: input values
*
* Read in input values for transition to trigger.
*
* \param[in] tok Current token to check for input values
* \param[in,out] String of read input values with current one appended
* \return Next state to enter
* \note Global variables used: none
*/															/*----------------------------------------------*/
CParser::parstates CParser::pfReadLineInvals(const int tok, string &invals)/*								*/
{															/*												*/
	parstates retval = P_READLINEINVALS;					/*												*/
	char temp[2] = { '0', '\0' };							/* temporary string to append value				*/
	switch (tok) {											/*												*/
	case '=':												/* start of line								*/
		break;												/* no operation									*/
	case '(':												/* start of line								*/
		break;												/* no operation									*/
	case ',':												/* separator									*/
		break;												/*												*/
	case IDENTIFIERDEF:										/* don't care ('x') detected					*/
		invals.append("x");									/* save for linking until everything is read	*/
		break;												/*												*/
	case INTEGER1DEF:										/* value detected								*/
		temp[0] += char(yylval.i);							/* update string								*/
		invals.append(temp);								/* save											*/
		break;												/*												*/
	case ')':												/* end of input names							*/
		retval = P_READLINESSTATE;							/* read values next								*/
		break;												/*												*/
	default:												/* none of expected tokens read					*/
		retval = P_ERROR;									/* stop operation								*/
		break;												/*												*/
	}														/*												*/
	return retval;											/*												*/
}															/*												*/
															/*----------------------------------------------*/
/*!
* \brief Read transition definition: initial state
*
* Read in initial state (or source state) from which the transition should trigger.
*
* \param[in] tok Current token to check for state
* \param[out] String of initial state name
* \return Next state to enter
* \note Global variables used: none
*/															/*----------------------------------------------*/
CParser::parstates CParser::pfReadLineSState(const int tok, string &srcstate)/*								*/
{															/*												*/
	parstates retval = P_READLINESSTATE;					/*												*/
	switch (tok) {											/*												*/
	case '(':												/* start of line								*/
		break;												/* no operation									*/
	case IDENTIFIERDEF:										/* name of input detected						*/
		srcstate = string(yylval.s);						/* save for linking until everything is read	*/
		break;												/*												*/
	case ')':												/* end of input names							*/
		retval = P_READLINEOUTPUTS;							/* read values next								*/
		break;												/*												*/
	default:												/* none of expected tokens read					*/
		retval = P_ERROR;									/* stop operation								*/
		break;												/*												*/
	}														/*												*/
	return retval;											/*												*/
}															/*												*/
															/*----------------------------------------------*/
/*!
* \brief Read transition definition: outputs
*
* Read in outputs to be set at the end of the transition.
*
* \param[in] tok Current token to check for output
* \param[in,out] List of mentioned outputs with current one appended
* \return Next state to enter
* \note Global variables used: none
*/															/*----------------------------------------------*/
CParser::parstates CParser::pfReadLineOutputs(const int tok, smtable::elementlist &outlist)/*				*/
{															/*												*/
	parstates retval = P_READLINEOUTPUTS;					/*												*/
	switch (tok) {											/*												*/
	case '>':												/* start of outputs								*/
		break;												/* no operation									*/
	case '[':												/* start of outputs								*/
		break;												/* no operation									*/
	case ',':												/* separator									*/
		break;												/*												*/
	case IDENTIFIERDEF:										/* name of input detected						*/
		outlist.push_back(string(yylval.s));				/* save for linking until everything is read	*/
		break;												/*												*/
	case ']':												/* end of input names							*/
		retval = P_READLINEOUTVALS;							/* read values next								*/
		break;												/*												*/
	default:												/* none of expected tokens read					*/
		retval = P_ERROR;									/* stop operation								*/
		break;												/*												*/
	}														/*												*/
	return retval;											/*												*/
}															/*												*/
															/*----------------------------------------------*/
/*!
* \brief Read transition definition: output values
*
* Read in output values to be set at the end of the transition.
*
* \param[in] tok Current token to check for output values
* \param[in,out] String of read output values with current one appended
* \return Next state to enter
* \note Global variables used: none
*/															/*----------------------------------------------*/
CParser::parstates CParser::pfReadLineOutvals(const int tok, string &outvals)/*								*/
{															/*												*/
	parstates retval = P_READLINEOUTVALS;					/*												*/
	char temp[2] = { '0', '\0' };							/* temporary string to append value				*/
	switch (tok) {											/*												*/
	case ':':												/* start of outvals								*/
		break;												/* no operation									*/
	case '(':												/* start of outvals								*/
		break;												/* no operation									*/
	case ',':												/* separator									*/
		break;												/*												*/
	case IDENTIFIERDEF:										/* don't care ('x') detected					*/
		outvals.append("x");								/* save for linking until everything is read	*/
		break;												/*												*/
	case INTEGER1DEF:										/* value detected								*/
		temp[0] += char(yylval.i);							/* update string								*/
		outvals.append(temp);								/* save											*/
		break;												/*												*/
	case ')':												/* end of input names							*/
		retval = P_READLINEDSTATE;							/* read values next								*/
		break;												/*												*/
	default:												/* none of expected tokens read					*/
		retval = P_ERROR;									/* stop operation								*/
		break;												/*												*/
	}														/*												*/
	return retval;											/*												*/
}															/*												*/
															/*----------------------------------------------*/
/*!
* \brief Read transition definition: destinantion state
*
* Read in destinanion state (or next state) to which the transition should step to.
*
* \param[in] tok Current token to check for state
* \param[out] String of destination state name
* \return Next state to enter
* \note Global variables used: none
*/															/*----------------------------------------------*/
CParser::parstates CParser::pfReadLineDState(const int tok, string &dststate)/*								*/
{															/*												*/
	parstates retval = P_READLINEDSTATE;					/*												*/
	switch (tok) {											/*												*/
	case '(':												/* start of line								*/
		break;												/* no operation									*/
	case IDENTIFIERDEF:										/* name of input detected						*/
		dststate = string(yylval.s);						/* save for linking until everything is read	*/
		break;												/*												*/
	case ')':												/* end of input names							*/
		retval = P_READLINEINPUTS;							/* read values next								*/
		break;												/*												*/
	default:												/* none of expected tokens read					*/
		retval = P_ERROR;									/* stop operation								*/
		break;												/*												*/
	}														/*												*/
	return retval;											/*												*/
}															/*												*/
															/*----------------------------------------------*/
/*!
* \brief Error state
*
* Report error and stay in here
*
* \param[in] none
* \param[out] none
* \return Next state to enter
* \note Global variables used: none
*/															/*----------------------------------------------*/
CParser::parstates CParser::pfError(void)					/*												*/
{															/*												*/
	parstates retval = P_ERROR;								/* don't exit error state						*/
	static bool noerroroccoured = true;						/* variable to remember error					*/
	if (noerroroccoured) {									/* check if already executed					*/
		noerroroccoured = false;							/* set remember variable						*/
		printf("     ______ ______ _    _ _      ______ _____  \n");
		printf("    |  ____|  ____| |  | | |    |  ____|  __ \\ \n");
		printf("    | |__  | |__  | |__| | |    | |__  | |__) |\n");
		printf("    |  __| |  __| |  __  | |    |  __| |  _  / \n");
		printf("    | |    | |____| |  | | |____| |____| | \\ \\ \n");
		printf("    |_|    |______|_|  |_|______|______|_|  \\_\\ in Zeile %d\n", IP_LineNumber);
	}
	return retval;											/*												*/
}															/*												*/
															/*----------------------------------------------*/