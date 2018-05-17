
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
int	CParser::yyparse()										/*												*/
{															/*												*/
	int tok = 0;											/* current token								*/
	prioritytype high_priority, mean_priority;				/* map of state combinations to prioritize		*/
	lowpriotype low_priority;								/* different structure for low priority			*/
	smtable::elementlist Zustandscodierung;					/* vector of optimized state name sequence		*/
	parstates state = P_HEADER;								/* state machine variable						*/
	smtable::elementlist inputs;							/* list of mentioned inputs						*/
	string invals;											/* string of associated input trigger values	*/
	string srcstate;										/* source state of transition					*/
	smtable::elementlist outputs;							/* list of mentioned outputs					*/
	string outvals;											/* string of associated output values			*/
	string dststate;										/* destination state of transition				*/
	if (prflag)fprintf(IP_List, "%5d ", (int)IP_LineNumber);/*												*/
															/*												*/
	while ((tok = yylex()) != IP_Token_table["End"]) {		/* Run till End									*/
		switch (state) {									/*												*/
		case P_HEADER:										/* Skip everything until "Begin"				*/
			state = pfSkipHeader(tok);						/*												*/
			break;											/*												*/
		case P_DEFSELECT:									/* Which definition is in queue?				*/
			state = pfGetDef(tok);							/*												*/
			break;											/*												*/
		case P_DEFSTATE:									/* read in state definitions					*/
			state = pfScanState(tok);						/*												*/
			break;											/*												*/
		case P_DEFIN:										/* read in input definitions					*/
			state = pfScanInputs(tok);						/*												*/
			break;											/*												*/
		case P_DEFOUT:										/* read in output definitions					*/
			state = pfScanOutputs(tok);						/*												*/
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
				inputs.clear();								/* clear variables for next line				*/
				invals.clear();								/*												*/
				srcstate.clear();							/*												*/
				outputs.clear();							/*												*/
				outvals.clear();							/*												*/
				dststate.clear();							/*												*/
			}												/*												*/
			break;											/* 												*/
		case P_ERROR:										/* catch error									*/
			state = pfError();								/* 												*/
			break;											/* 												*/
		}													/* 												*/
	}														/* 												*/
															/* 												*/
	printf("\nAnzahl Zustaende: %d", state_count);			/* 												*/
	printf("\nAnzahl Eingangssignale: %d", input_count);	/* 												*/
	printf("\nAnzahl Ausgangssignale: %d \n", output_count);/* 												*/
	table.print();											/* 												*/
															/* 												*/
	/* Codierungsoptimierung																				*/
	high_priority=highPriority();							/*												*/
	mean_priority=meanPriority();							/*												*/
	low_priority=lowPriority();								/*												*/
															/*												*/
	/* lowPriority() creates a list of state combinations, satisfying low priority conditions.				*/
	/* This list will most likely contain similar entries. removeSunsets() only leaves the biggest			*/
	/* combinations in that list. These combinations are not compatible with each other.					*/
	removeSubsets(low_priority);							/*												*/
															/*												*/
	Zustandscodierung = optimize(high_priority, mean_priority, low_priority);/*								*/
															/*												*/
	/* Write output files																					*/
	if (writeOutputFile()) {								/*												*/
		fprintf(stderr, "Ausgabedatei \"ZMnichtoptimiert.tbl\" fehlgeschlagen\n");/*						*/
	}														/*												*/
	else {													/*												*/
		cout << "Ausgabedatei \"ZMnichtoptimiert.tbl\" erfolgreich geschrieben\n";/*						*/
	}														/*												*/
	if (writeOutputFile(Zustandscodierung)) {				/*												*/
		fprintf(stderr, "Ausgabedatei \"ZMoptimiert.tbl\" fehlgeschlagen\n");/*								*/
	}														/*												*/
	else {													/*												*/
		cout << "Ausgabedatei \"ZMoptimiert.tbl\" erfolgreich geschrieben\n";/*								*/
	}														/*												*/
	//smtable::elementlist temp;//testdatei für Schäfers minimallösung
	//temp.push_back("Fahrbtr");
	//temp.push_back("Starten");
	//temp.push_back("Aus_Ein");
	//temp.push_back("S1");
	//temp.push_back("S2");
	//temp.push_back("S4");
	//temp.push_back("S9");
	//temp.push_back("");
	//writeOutputFile(temp);
	return 0;												/*												*/
															/*												*/
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
															/*												*/
															/*----------------------------------------------*/
/*!
* \brief Skip documentation
*
* Ignores everything until "Begin".
*
* \param[in] tok Current token to check for "Begin"
* \param[out] none
* \return Next state to enter
* \note Global variables used: none
*/
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
*/
CParser::parstates CParser::pfGetDef(const int tok)			/* Parser function: Select next definition		*/
{															/*												*/
	parstates retval;										/*												*/
	static int i = 0;										/*												*/
															/*												*/
	if (defScanned.states && defScanned.inputs && defScanned.outputs) { /* everything defined! Now state machine definitions! */
		table.init(scannedStates, scannedInputs, scannedOutputs);	/* let the table know the definitions	*/
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
*/
CParser::parstates CParser::pfScanState(const int tok)		/* Parser function: Read in state definition	*/
{															/*												*/
	parstates retval = P_DEFSTATE;							/*												*/
	switch (tok) {											/*												*/
	case IDENTIFIERDEF:										/* valid name									*/
		scannedStates.push_back(string(yylval.s));			/* save											*/
		state_count++;										/* update count									*/
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
*/
CParser::parstates CParser::pfScanInputs(const int tok)		/* Parser function: Read in input definition	*/
{															/*												*/
	parstates retval = P_DEFIN;								/*												*/
	switch (tok) {											/*												*/
	case IDENTIFIERDEF:										/* valid name									*/
		scannedInputs.push_back(string(yylval.s));			/* save											*/
		input_count++;										/* update count									*/
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
*/
CParser::parstates CParser::pfScanOutputs(const int tok)	/* Parser function: Read in output definition	*/
{															/*												*/
	parstates retval = P_DEFOUT;							/*												*/
	switch (tok) {											/*												*/
	case IDENTIFIERDEF:										/* valid name									*/
		scannedOutputs.push_back(string(yylval.s));			/* save											*/
		output_count++;										/* update count									*/
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
*/
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
*/
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
*/
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
*/
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
*/
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
*/
CParser::parstates CParser::pfReadLineDState(const int tok, string &dststate)/*								*/
{															/*												*/
	parstates retval = P_READLINEDSTATE;					/*												*/
	switch (tok) {											/*												*/
	case '(':												/* start of line								*/
		break;												/* no operation									*/
	case IDENTIFIERDEF:										/* name of input detected						*/
		dststate = string(yylval.s);					/* save for linking until everything is read	*/
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
															/*												*/
CParser::parstates CParser::pfError(void)					/*												*/
{															/*												*/
	parstates retval = P_ERROR;								/*												*/
	return retval;											/*												*/
}															/*												*/
															/*----------------------------------------------*/
/*!
* \brief Skip documentation
*
* Get the high priorities of the state diagramm and return them
*
* \param[in] none
* \param[out] none
* \return prioritytype highPriority 
* \note Global variables used: class smtable table: iwidth, iheight, istates and tabletype table
*/
CParser::prioritytype CParser::highPriority()				/* high priority: when at least two states have */
{															/* the same next state by the same input value	*/
	prioritytype high_priority;								/* save all high priorities in a map<string,vector<string>>	*/
	vector<string> candidates;								/* candidates for high priority					*/
	int candidate_count = 0;								/*												*/
															/*												*/
	for (int k = 0; k < table.iwidth; k++)					/*												*/
	{														/*												*/
		for (int j = 0; j < table.iheight; j++) {			/*												*/
			for (int i = 0; i < table.iheight; i++) {		/*												*/
				if (i != j) {				/* checks, if the table height j is unequal the table height i	*/
					if (table.istates.at(j) == table.table[table.istates.at(i)].at(k).next_state) {	/*		*/
					/* checks, if a state is equal with a next state from a other state						*/
						candidate_count++;					/* count candidates								*/
						candidates.push_back(table.istates.at(i).c_str());/* save candidate					*/
					}										/*												*/
				}											/*												*/
			}												/*												*/
			if (candidate_count >= 2) {						/* condition for high priority					*/
				high_priority[table.istates.at(j).c_str()] = candidates;/* if there are more candidates than	*/
			}											/* one, then the high priority map is passed candidates	*/
			candidates.clear();								/* clear candidates								*/
			candidate_count = 0;							/* reset candidate_count						*/
		}													/*												*/
	}														/*												*/
	return high_priority;									/*												*/
}															/*												*/
															/*----------------------------------------------*/
/*!
* \brief Skip documentation
*
* Get the mean priorities of the state diagramm and return them
*
* \param[in] none
* \param[out] none
* \return prioritytype meanPriority
* \note Global variables used: class smtable table: iwidth, iheight, istates and tabletype table
*/
CParser::prioritytype CParser::meanPriority()				/* mean priority: is when a state have at least	*/
{													/* two different next states by different input values	*/
	prioritytype mean_priority;					/* save all mean priorities in a map<string,vector<string>> */
	vector<string> candidates;								/* candidates for mean priority					*/
	int candidate_count = 0;								/*												*/
															/*												*/
	for (int j = 0; j < table.iheight; j++) {				/* passed through row for row of mean priority	*/
		for (int i = 0; i < table.iwidth; i++) {			/*												*/
			if ((table.table[table.istates.at(j)].at(i).next_state) != (table.istates.at(j))) {	/*			*/
			/*checks, if a state is unequal of his next_states (independet of input value)					*/
				if (!((find(candidates.begin(), candidates.end(), table.table[table.istates.at(j)].at(i).next_state) != candidates.end())) && (table.table[table.istates.at(j).c_str()].at(i).next_state.size() > 0)) {
				/*checks, if the next state of a state isn't in the candidate vector AND a next state is anyway existing */
					candidate_count++;						/* count candidates								*/		
					candidates.push_back(table.table[table.istates.at(j).c_str()].at(i).next_state);/*save	*/
				}											/* a candidate for mean priority into candidates*/
			}												/*												*/
		}													/*												*/
		if (candidate_count >= 2) {							/* condition for mean priority					*/
			mean_priority[table.istates.at(j).c_str()] = candidates;/*if there are more candidates than one,*/
		}												/* then the mean priority map is passed candidates	*/
		candidates.clear();									/* clear candidates								*/
		candidate_count = 0;								/* reset candidate_count						*/
	}														/*												*/
	return mean_priority;									/*												*/
}															/*												*/
															/*----------------------------------------------*/
/*!
* \brief Skip documentation
*
* Get the low priorities of the state diagramm and return them
*
* \param[in] none
* \param[out] none
* \return lowpriotype lowPriority
* \note Global variables used: class smtable table: iwidth, iheight, istates and tabletype table
*/
CParser::lowpriotype CParser::lowPriority()					/*lowest priority is given when at least two states have the same output behaviour*/
{															/*												*/
	vector < vector<string>> low_priority;					/*save all low priorities in a vector<vector<string>>*/
	vector<string> candidates;								/* candidates for low priority					*/
	int candidate_count = 0;								/*												*/
	bool output_matched = false;							/* candidates for low priority					*/
	for (int j = 0; j < table.iwidth; j++) {				/* check all input possibilities				*/
		for (int k = 0; k < (1 << table.ioutputs.size()); k++) {/*check all output possibilities (00 01 10 11 -> 0 1 2 3), (1 << table.ioutputs.size()) is the number of output possibilities*/
			for (int i = 0; i < table.iheight; i++) {		/*												*/
				output_matched = table.bitsMatch(k, table.table[table.istates.at(i)].at(j).out_list.c_str());/*checks, if int k matches with output bit pattern and set bool output_matched */
				if (output_matched == true) {				/*												*/
					candidate_count++;						/* count candidates								*/
					candidates.push_back(table.istates.at(i).c_str());/*push a candidate for low priority into candidates*/
					output_matched = false;					/* reset bool output_matched					*/
				}											/*												*/
			}												/*												*/
			if (candidate_count >= 2) {						/* condition for low priority					*/
				low_priority.push_back(candidates);			/*if there are more candidates than one, then the low priority vector<vector<<string>>> is passed candidates*/
			}												/*												*/
			candidates.clear();								/* clear candidates								*/
			candidate_count = 0;							/* reset candidate_count						*/
		}													/*												*/
	}														/*												*/
	return low_priority;									/*												*/
}															/*												*/
															/*----------------------------------------------*/
	/*!
	* \brief Skip documentation
	*
	* Get the optimize state coding of the state diagramm and return them
	* Set first the high priorities, then the mean priorities, then the low priorities and at last left states in Zustandscodierung
	*
	* \param[in] prioritytype high_priority, prioritytype mean_priority, lowpriotype low_priority
	* \param[out] none
	* \return elementlist Zustandscodierung
	* \note Global variables used: class smtable table: istates
	*/
smtable::elementlist CParser::optimize(prioritytype high_priority, prioritytype mean_priority, lowpriotype low_priority)
{															/*												*/
	uint set_states = 0;									/*												*/
	int Zustandscodierung_size = 1;						/*												*/
	uint high_priority_count = high_priority.size();		/*												*/
	uint mean_priority_count = mean_priority.size();		/*												*/
	uint low_priority_count = low_priority.size();			/*												*/
	bool is_set = false;									/*												*/
	int candidate_count = 0;								/*												*/
	prioritytype::iterator t3;								/*												*/
	vector <string> Zustandscodierung, candidates;			/*												*/
															/*												*/
	while (Zustandscodierung_size < table.iheight) {		/*												*/
		Zustandscodierung_size *= 2;				/* Zustandscodierung_size is the next power of 2 (2^n)	*/
	}														/*												*/
															/*												*/
															/*												*/
	Zustandscodierung.resize(Zustandscodierung_size);		/*												*/
	t3 = high_priority.begin();								/*												*/
															/*												*/
	/*high priorities*/										/*												*/
	if (set_states < table.istates.size()) {	/* ckecks, if a state isn't set in the Zustandscodierung	*/
		for (uint i = 0; i < high_priority_count; i++) {	/*												*/
			if (!((find(Zustandscodierung.begin(), Zustandscodierung.end(), t3->first) != Zustandscodierung.end()))) {  
			/* ckecks, if a high priority state isn't set in the Zustandscodierung							*/
				Zustandscodierung[set_states] = t3->first;	/* sets a high priority state					*/
				set_states++;								/*												*/
				is_set = true;								/* sets a bool bit								*/
			}												/*												*/
			t3++;											/* increase t3 iterator							*/
															/*												*/
			while (is_set) {								/* run through till bool bit isn't set anymore	*/
				if (high_priority[high_priority.at(Zustandscodierung[set_states - 1]).at(0)].size() > 0) {		
				/* checks, if there is another high priority of the first state of the high priority state	*/
					Zustandscodierung[set_states] = high_priority[Zustandscodierung[set_states - 1]].at(1);
					Zustandscodierung[set_states + 1] = high_priority[Zustandscodierung[set_states - 1]].at(0);	
					/* set the first state of the high priority state at second (the next high priority can be set directly afterwards */
					set_states += 2;						/* e.g. S0 S7 S9 + S7 S3 S4 -> S0 S9 S7 S3 S4	*/
				}											/*												*/
				else {										/*												*/
					Zustandscodierung[set_states] = high_priority[Zustandscodierung[set_states - 1]].at(0);
					Zustandscodierung[set_states + 1] = high_priority[Zustandscodierung[set_states - 1]].at(1);
					set_states += 2;						/*												*/
				}											/*												*/
				if (high_priority[Zustandscodierung[set_states - 1]].size() > 0) {
				/* checks, if there is a high priority state of the last set state							*/
					is_set = true;							/*												*/
				}											/*												*/
				else {										/*												*/
					is_set = false;							/*												*/
				}											/*												*/
			}												/*												*/
		}													/*												*/
	}														/*												*/
															/*												*/
	t3 = mean_priority.begin();								/*												*/
	is_set = false;											/*												*/
															/*												*/
	/*mean priority*/										/*												*/
	if (set_states < table.istates.size()) {	/* ckecks, if a state isn't set in the Zustandscodierung	*/
		for (uint j = 0; j < mean_priority_count; j++) {	/*												*/
			for (uint i = 0; i < t3->second.size(); i++) {	/*												*/
				if (find(Zustandscodierung.begin(), Zustandscodierung.end(), t3->second.at(i)) != Zustandscodierung.end()) {
				/* ckecks, if a state of a mean priority is set in the Zustandscodierung					*/
					is_set = true;							/*												*/
				}											/*												*/
			}												/*												*/
			if (is_set == false) {							/*												*/
				if (!(find(Zustandscodierung.begin(), Zustandscodierung.end(), t3->first) != Zustandscodierung.end())) {
				/* ckecks, if a mean priority state isn't set in the Zustandscodierung						*/
					Zustandscodierung[set_states] = t3->first;/*set mean priority state in the Zustandscodierung*/
					set_states++;							/*												*/
				}											/*												*/
				for (uint p = 0; p < t3->second.size(); p++) {/*											*/
					Zustandscodierung[set_states] = t3->second.at(p);/*set states of mean priority in the Zustandscodierung*/
					set_states++;							/*												*/
				}											/*												*/
			}												/*												*/
			is_set = false;									/*												*/
			t3++;											/* increase t3 iterator							*/
		}													/*												*/
	}														/*												*/
															/*												*/
	/*low priority*/										/*												*/
	if (set_states < table.istates.size())					/*ckecks, if a state isn't set in the Zustandscodierung*/
	{														/*												*/
		for (uint j = 0; j < low_priority_count; j++) {		/*												*/
			for (uint i = 0; i < low_priority[j].size(); i++) {/*											*/
				if (!(find(Zustandscodierung.begin(), Zustandscodierung.end(), low_priority[j].at(i)) != Zustandscodierung.end())) {
				/* ckecks, if a state of a low priority isn't set in the Zustandscodierung					*/
					candidate_count++;						/* count candidates								*/
					candidates.push_back(low_priority[j].at(i));/* push a candidate for low priority		*/
				}											/*		(in Zustandscodierung) into candidates	*/
			}												/*												*/
			if (candidate_count >= 2) {						/*if there are more candidates than one, then the candidates is set in Zustandscodierung*/
				for (uint k = 0; k < candidates.size(); k++) {	/*												*/
					Zustandscodierung[set_states] = t3->second.at(k);/*										*/
					set_states++;							/*												*/
				}											/*												*/
			}												/*												*/
			candidates.clear();								/* clear candidates								*/
			candidate_count = 0;							/* reset candidate_count						*/
		}													/*												*/
	}														/*												*/
	/*Set left states*/										/*												*/
	if (set_states < table.istates.size()) {				/*ckecks, if a state isn't set in the Zustandscodierung*/
		for (uint i = 0; i < table.istates.size(); i++) {	/*												*/
			if (!(find(Zustandscodierung.begin(), Zustandscodierung.end(), table.istates.at(i)) != Zustandscodierung.end())) {
			/* ckecks, if a state isn't set in the Zustandscodierung										*/
				Zustandscodierung[set_states] = table.istates.at(i);/*set a state in the Zustandscodierung	*/
				set_states++;								/*												*/
			}												/*												*/
		}													/*												*/
	}														/*												*/
	return Zustandscodierung;								/*												*/
}															/*												*/
															/*----------------------------------------------*/
/*!
* \brief Skip documentation
*
* Ignores everything until "Begin".
*
* \param[in] tok Current token to check for "Begin"
* \param[out] none
* \return Next state to enter
* \note Global variables used: none
*/
bool CParser::contains(smtable::elementlist base, smtable::elementlist cmp)/*								*/
{															/* check, if base contains all entries of cmp	*/
	bool retval = true;										/* preset retval								*/
	for (uint i = 0; i < cmp.size(); i++) {					/* run through cmp								*/
		smtable::elementlist::iterator it;					/* declare iterator for search					*/
		if ((it = find(base.begin(), base.end(), cmp.at(i))) == base.end()) {/* if entry not found in base	*/
			retval = false;									/* cmp can't be subset of base					*/
		}													/* 												*/
	}														/* 												*/
	return retval;											/* 												*/
}															/* 												*/
															/*----------------------------------------------*/
/*!
* \brief Skip documentation
*
* Ignores everything until "Begin".
*
* \param[in] tok Current token to check for "Begin"
* \param[out] none
* \return Next state to enter
* \note Global variables used: none
*/
void CParser::removeSubsets(vector<vector<string>> &tab) {	/*												*/
	for (uint i = 0; i < tab.size(); i++) {					/* run through list								*/
		for (uint j = 0; (j < tab.size()) && (i < tab.size()); j++) {/* run through list again				*/
			if ((contains(tab.at(i), tab.at(j))) && (i != j)) {/* if j is subset of i and doesnt compar self*/
				vector<vector<string>>::iterator it = tab.begin() + j;/* declare pointer to subset for erase*/
				tab.erase(it);								/* erase										*/
				j--;										/* adjust k, because current element at k is	*/
			}												/*							not checked, yet	*/
		}													/* 												*/
	}														/* 												*/
}															/* 												*/
															/*----------------------------------------------*/
/*!
* \brief Skip documentation
*
* Ignores everything until "Begin".
*
* \param[in] tok Current token to check for "Begin"
* \param[out] none
* \return Next state to enter
* \note Global variables used: none
*/
CParser::funcreturn CParser::writeOutputFile(void)			/* 												*/
{															/* 												*/
	CParser::funcreturn retval = F_SUCCESS;					/* preset return								*/
	int stateCodeBitCount = 1;								/* number of bits needed to code states			*/
	for (; (1 << stateCodeBitCount) < table.istates.size(); stateCodeBitCount++);/* calculation				*/
	ofstream outfile;										/* output file object							*/
	try {													/* safe code block, if write exception occours	*/
		outfile.open("ZMnichtoptimiert.tbl");				/* 												*/
		outfile << "table ZMnichtoptimiert\n  input ";		/* write file header							*/
		for (uint i = 0; i < table.iinputs.size(); i++) {	/* print actual input names						*/
			outfile << table.iinputs.at(i).c_str() << " ";	/* 												*/
		}													/* 												*/
		for (int i = 0; i < stateCodeBitCount; i++) {		/* print names for current state coding			*/
			char statetemp[4] = { 'C', '0', '0', '\0' };	/* 2^100 = 10^30 States, should be enough		*/
			statetemp[1] = i / 10 + '0';					/* 												*/
			statetemp[2] = i + '0';							/* 												*/
			outfile << statetemp << " ";					/* 												*/
		}													/* 												*/
		outfile << "\n  output ";							/* 												*/
		for (int i = 0; i < stateCodeBitCount; i++) {		/* print names for next state coding			*/
			char statetemp[4] = { 'D', '0', '0', '\0' };	/* 												*/
			statetemp[1] = i / 10 + '0';					/* 												*/
			statetemp[2] = i + '0';							/* 												*/
			outfile << statetemp << " ";					/* 												*/
		}													/* 												*/
		outfile << "\n\" nonoptimized transitiontable\n";	/* 												*/
		for (int i = 0; i < (1 << table.iinputs.size()); i++) {/* run through input combinations			*/
			for (int j = 0; j < (1 << stateCodeBitCount); j++) {/* run through state combinations			*/
				int nextstate = 0;							/* 												*/
				if (j < table.istates.size()) {				/* 												*/
					for (; (nextstate < table.istates.size()) && (table.istates.at(nextstate) != table.table[table.istates.at(j)].at(i).next_state); nextstate++);
					outfile << "  " << *table.int2bit(i, table.iinputs.size()) << *table.int2bit(j, stateCodeBitCount) << " | ";
					if (nextstate < table.istates.size()) {	/* 												*/
						outfile << *table.int2bit(nextstate, stateCodeBitCount);/* 							*/
					}										/* 												*/
					else {									/* 												*/
						for (int k = 0; k < stateCodeBitCount; k++) {/* 									*/
							outfile << "-";					/* 												*/
						}									/* 												*/
					}										/* 												*/
					outfile << "\n";						/* 												*/
				}											/* 												*/
				else {										/* 												*/
					outfile << "  " << *table.int2bit(i, table.iinputs.size()) << *table.int2bit(j, stateCodeBitCount) << " | ";
					for (int k = 0; k < stateCodeBitCount; k++) {/* 										*/
						outfile << "-";						/* 												*/
					}										/* 												*/
					outfile << "\n";						/* 												*/
				}											/* 												*/
			}												/* 												*/
		}													/* 												*/
		outfile << "end\n";									/* 												*/
		outfile.close();									/* 												*/
	}														/* 												*/
	catch(...){												/* 												*/
		retval = F_FAIL;									/* 												*/
	}														/* 												*/
	return retval;											/* 												*/
}															/* 												*/
															/*----------------------------------------------*/
/*!
* \brief Skip documentation
*
* Ignores everything until "Begin".
*
* \param[in] tok Current token to check for "Begin"
* \param[out] none
* \return Next state to enter
* \note Global variables used: none
*/
CParser::funcreturn CParser::writeOutputFile(smtable::elementlist statelist)/*								*/
{															/*												*/
	CParser::funcreturn retval = F_SUCCESS;					/*												*/
	int stateCodeBitCount = 1;								/*												*/
	for (; (1 << stateCodeBitCount) < table.istates.size(); stateCodeBitCount++); /* calculate bits needed	*/
	ofstream outfile;										/*												*/
	try{													/*												*/
		outfile.open("ZMoptimiert.tbl");					/*												*/
		outfile << "table ZMoptimiert\n  input ";			/*												*/
		for (uint i = 0; i < table.iinputs.size(); i++) {	/* print actual input names						*/
			outfile << table.iinputs.at(i).c_str() << " ";	/*												*/
		}													/*												*/
		for (uint i = 0; i < stateCodeBitCount; i++) {		/* print names for current state coding			*/
			char statetemp[4] = { 'C', '0', '0', '\0' };	/* 2^100 = 10^30 States							*/
			statetemp[1] = i / 10 + '0';					/*												*/
			statetemp[2] = i + '0';							/*												*/
			outfile << statetemp << " ";					/*												*/
		}													/*												*/
		outfile << "\n  output ";							/*												*/
		for (uint i = 0; i < stateCodeBitCount; i++) {		/* print names for next state coding			*/
			char statetemp[4] = { 'D', '0', '0', '\0' };	/*												*/
			statetemp[1] = i / 10 + '0';					/*												*/
			statetemp[2] = i + '0';							/*												*/
			outfile << statetemp << " ";					/*												*/
		}													/*												*/
		outfile << "\n\" nonoptimized transitiontable\n";	/*												*/
		for (int i = 0; i < (1 << table.iinputs.size()); i++) {/* run through input combinations			*/
			for (int j = 0; j < (1 << stateCodeBitCount); j++) {/* run through state combinations			*/
				int nextstate = 0;							/*												*/
				if (statelist.at(j).size() != 0) {			/* if entry not empty get number of next state	*/
					for (; (nextstate < table.istates.size()) && (statelist.at(nextstate) != table.table[statelist.at(j)].at(i).next_state); nextstate++);
					outfile << "  " << *table.int2bit(i, table.iinputs.size()) << *table.int2bit(int2gray(j), stateCodeBitCount) << " | ";
					if (nextstate < table.istates.size()) {	/* if next_state found print it					*/
						outfile << *table.int2bit(int2gray(nextstate), stateCodeBitCount);/*				*/
					}										/*												*/
					else {									/* if no next_state found						*/
						for (int k = 0; k < stateCodeBitCount; k++) {/* fill with don't cares				*/
							outfile << "-";					/*												*/
						}									/*												*/
					}										/*												*/
					outfile << "\n";						/*												*/
				}											/*												*/
				else {										/* if entry empty								*/
					outfile << "  " << *table.int2bit(i, table.iinputs.size()) << *table.int2bit(int2gray(j), stateCodeBitCount) << " | ";
					for (int k = 0; k < stateCodeBitCount; k++) {	/* fill with don't cares				*/
						outfile << "-";						/*												*/
					}										/*												*/
					outfile << "\n";						/*												*/
				}											/*												*/
			}												/*												*/
		}													/*												*/
		outfile << "end\n";									/*												*/
		outfile.close();									/*												*/
	}														/*												*/
	catch (...) {											/*												*/
		retval = F_FAIL;									/*												*/
	}														/*												*/
	return retval;											/*												*/
}															/*												*/
															/*----------------------------------------------*/
/*!
* \brief Skip documentation
*
* Ignores everything until "Begin".
*
* \param[in] tok Current token to check for "Begin"
* \param[out] none
* \return Next state to enter
* \note Global variables used: none
*/
int CParser::int2gray(int input)							/* returns integer, which bit structure is gray	*/
{															/* coded										*/
	return input ^ (input >> 1);							/*												*/
}															/*----------------------------------------------*/
