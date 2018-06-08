
#include "stdafx.h"
#include <iomanip>
#include <iostream>
#include <vector>
#include <map>
#include "smtable.h"

/*!
* \brief Initialize table only
*
* Set up table with initial values. The next_state of each entry is left blank, but
* output is preset with don't cares.
* 
* \warning setStates(), setInputs() and setOutputs() have to be called beforehand
*
* \param[in] none
* \param[out] none
* \return operation success
* \note Global variables used: table
*/															/*----------------------------------------------*/
smtable::fstate smtable::init() {							/* initialization of the table object			*/
	vector<entry> temp;										/* initial values to append						*/
	for (uint i = 0; i < iinputs.size(); i++) iwidth *= 2;	/* number of all input combinations	(width)		*/
	iheight = istates.size();								/* number of states (height)					*/
	temp.resize(iwidth);									/* set vector to width							*/
	for (int i = 0; i < iwidth; i++) {						/* run through vector							*/
		for (uint j = 0; j < ioutputs.size(); j++)			/* run through outputs							*/
			temp.at(i).out_list.append("x");				/* preset to don't care							*/
	}														/*												*/
	for (uint i = 0; i < istates.size(); i++) {				/* build table line by line						*/
		table.insert(pair<string, vector<entry>>(istates.at(i), temp));	/*									*/
	}														/*												*/
	return eOK;												/*												*/
}															/*												*/
															/*----------------------------------------------*/
/*!
* \brief Get definitions and initialize table 
*
* Set definitions of class.
* Set up table with initial values. The next_state of each entry is left blank, but
* output is preset with don't cares.
*
* \param[in] none
* \param[out] none
* \return operation success
* \note Global variables used: table
*/															/*----------------------------------------------*/
smtable::fstate smtable::init(elementlist states, elementlist inputs, elementlist outputs) {/*				*/
	vector<entry> temp;										/* initial values to append						*/
	istates = states;										/*												*/
	iinputs = inputs;										/*												*/
	ioutputs = outputs;										/*												*/
	for (uint i = 0; i < iinputs.size(); i++) iwidth *= 2;	/* number of all input combinations	(width)		*/
	iheight = istates.size();								/* number of states (height)					*/
	temp.resize(iwidth);									/* set vector to width							*/
	for (int i = 0; i < iwidth; i++) {						/* run through vector							*/
		for (uint j = 0; j < ioutputs.size(); j++)			/* run through outputs							*/
			temp.at(i).out_list.append("x");				/* preset to don't care							*/
	}														/*												*/
	for (uint i = 0; i < istates.size(); i++) {				/* build table line by line						*/
		table.insert(pair<string, vector<entry>>(istates.at(i), temp));/*									*/
	}														/*												*/
	return eOK;												/*												*/
}															/*												*/
															/*----------------------------------------------*/
/*!
* \brief Set state definitions for calss
*
* Reads list of state names into class.
*
* \param[in] tok Current token to check for name of state
* \param[out] none
* \return operation success
* \note Global variables used: istates
*/
smtable::fstate smtable::setStates(elementlist states) {
	istates = states;
	return eOK;
}

/*!
* \brief input definitions for calss
*
* Reads list of input names into class.
*
* \param[in] tok Current token to check for name of input
* \param[out] none
* \return operation success
* \note Global variables used: iinput
*/
smtable::fstate smtable::setInputs(elementlist inputs) {
	iinputs = inputs;
	return eOK;
}

/*!
* \brief output definitions for calss
*
* Reads list of output names into class.
*
* \param[in] tok Current token to check for name of output
* \param[out] none
* \return operation success
* \note Global variables used: ioutput
*/
smtable::fstate smtable::setOutputs(elementlist outputs) {
	ioutputs = outputs;
	return eOK;
}

/*!
* \brief Insert transition into table
*
* This function is the heart of the class smtable. It is used to enter a single transition information
* into the table. It handles ambiguous description of values.
* They can either be don't cares ('x') or be unmentioned in transition definition.
* 
*
* \param[in] inputs List of input names to be read. Type: vector<string>
* \param[in] inputval String with values the corresponding inputs should have for transition to trigger.
*					  Values can either be '0', '1' or 'x' (don't care). Has to match size of inputs.
* \param[in] srcstate String with name of initial state.
* \param[in] outputs List of output names to be set. Type: vector<string>
* \param[in] outputval String with values the corresponding outputs should have after transition.
*					   Values can either be '0', '1' or 'x' (don't care). Has to match size of outputs.
* \param[in] dststate List of input names. Type: vector<string>
* \param[out] none
* \return execution status
* \note Global variables used: table
*/															/*----------------------------------------------*/
smtable::fstate smtable::link(elementlist inputs, string inputval, string srcstate, elementlist outputs, string outputval, string dststate) {
	int numberOfInput = (1 << iinputs.size());				/* number of input combinations					*/
	bool foundMatch = false;								/* 												*/
	entry tempentry;										/* temporary cell to be copied to table			*/
	tempentry.next_state = dststate;						/* next state unambiguous, just copy			*/
	/* Now comes the tricky part: Inputs can either be assigned 1, 0 or x or be unassigned, then x			*/
	for (uint i = 0; i < ioutputs.size(); i++) {			/* run through known outputs					*/
		for (uint j = 0; (j < outputs.size()) && !foundMatch; j++) { /* run through given outputs			*/
			if (outputs.at(j) == ioutputs.at(i)) {			/* output specified?							*/
				char tempOutVal[2] = { 0, 0 };				/* temp string to append						*/
				tempOutVal[0] = outputval.at(j);			/* YES! copy value								*/
				tempentry.out_list.append(tempOutVal);		/* and save										*/
				foundMatch = true;							/* ignore remainder								*/
			}												/*												*/
		}													/*												*/
		if (!foundMatch) tempentry.out_list.append("x");	/* not found at all? Unassigned, then x			*/
		foundMatch = false;									/* reset for next run							*/
	}	/* tempentry set up correctly, now has to be copied to matching input values						*/
															/*												*/
	/* Another tricky part: extract info, where to copy it all (could be ambiguous)							*/
	foundMatch = false;										/* reuse/reset again							*/
	char* tempInVal = new char[iinputs.size()+1];			/* set up a temporary string					*/
	tempInVal[iinputs.size()] = '\0';						/* determine end of string						*/
	for (uint i = 0; i < iinputs.size(); i++) {				/* run through known inputs						*/
		for (uint j = 0; (j < inputs.size()) && !foundMatch; j++) { /* run through inputs					*/
			if (inputs.at(j) == iinputs.at(i)) {			/* input specified?								*/
				tempInVal[i] = inputval.at(j);				/* YES! copy value								*/
				foundMatch = true;							/* ignore remainder								*/
			}												/*												*/
		}													/*												*/
		if (!foundMatch) tempInVal[i] = 'x';				/* not found at all? Unassigned, then x			*/
		foundMatch = false;									/* reset for next run							*/
	}	/* input values extracted																			*/
	for (int i = 0; i < numberOfInput; i++) {				/* run through all input combinations			*/
		if (bitsMatch(i, tempInVal)) table[srcstate].at(i) = tempentry; /* on match copy					*/
	}														/*												*/
															/*												*/
	delete tempInVal;										/* don't forget to clear heap					*/
	return eOK;												/*												*/
}															/*												*/
															/*----------------------------------------------*/
/*!
* \brief print transition table
*
* Prints out the table in readable form.
*
* \param[in] none
* \param[out] none
* \return execution status
* \note Global variables used: table (read only)
*/															/*----------------------------------------------*/
smtable::fstate smtable::print() {							/*												*/
	/* print correct input and output sequences																*/
	cout << "Reihenfolge Eingangssignale: ";				/* print sequence of input names				*/
	for (uint i = 0; i < iinputs.size(); i++) cout << iinputs.at(i).c_str() << " ";/*						*/
	cout << "\n";											/*												*/
	cout << "Reihenfolge Ausgangssignale: ";				/* print sequence of output names				*/
	for (uint i = 0; i < ioutputs.size(); i++) cout << ioutputs.at(i).c_str() << " ";/*						*/
	cout << "\n\n|\t|";										/*												*/
															/*												*/
	/* print all input combinations																			*/
	for (int i = 0; i < iwidth; i++) { 						/*												*/
		string bits = int2bit(i, iinputs.size());			/* convert integer to string					*/
		cout << bits.c_str() << "\t|";						/* print it										*/									/* don't forget to clear heap					*/
	}														/*												*/
	cout << "\n|-------";									/* table separator								*/
	for (int i = 0; i < iwidth; i++) cout << "|-------";	/* table separator								*/
															/*												*/
	/* print next states determined by current state and input												*/
	cout << "|";											/* line end										*/
	for (int i = 0; i < iheight; i++) {						/*												*/
		if(istates.at(i).size() < 7)						/* print maximum first seven characters			*/
			cout << "\n|" << istates.at(i).c_str() << "\t|";/* ^											*/
		else												/* ^											*/
			cout << "\n|" << istates.at(i).substr(0,7).c_str() << "|";/* ^									*/
		for (int j = 0; j < iwidth; j++) {					/*												*/
			if(table[istates.at(i)].at(j).next_state.size() < 7)/* print maximum first seven characters		*/
				cout << table[istates.at(i)].at(j).next_state.c_str() << "\t|";/* ^							*/
			else											/* ^											*/
				cout << table[istates.at(i)].at(j).next_state.substr(0,7).c_str() << "|";/* ^				*/
		}													/*												*/
	}														/*												*/
	cout << "\n|-------";									/* table separator								*/
	for (int i = 0; i < iwidth; i++) cout << "|-------";	/* table separator								*/
	cout << "|";											/*												*/
															/*												*/
	/* print output values determined by current state and input											*/
	for (int i = 0; i < iheight; i++) {						/*												*/
		if (istates.at(i).size() < 7)						/* print maximum first seven characters			*/
			cout << "\n|" << istates.at(i).c_str() << "\t|";/* ^											*/
		else												/* ^											*/
			cout << "\n|" << istates.at(i).substr(0, 7).c_str() << "|";/* ^									*/
		for (int j = 0; j < iwidth; j++) {					/*												*/
			cout << table[istates.at(i)].at(j).out_list.c_str() << "\t|";/*									*/
		}													/*												*/
	}														/*												*/
	cout << "\n\n";											/*												*/
	return eOK;												/*												*/
}															/*												*/
															/*----------------------------------------------*/
/*!
* \brief Convert integer to ASCII binary string
*
* Converts the integer val to a string with length width which values are either '0' or '1'.
*
* \param[in] val The integer value to convert.
* \param[in] width The number of bits to be converted, starting from right (LSB)
* \param[out] none
* \return string of binary val of length width
* \note Global variables used: none
*/															/*----------------------------------------------*/
string smtable::int2bit(int val, int width) {				/*												*/
	string retval;											/*												*/
	for (int i = 0; i < width; i++) {						/* run for given length							*/
		retval.insert(retval.begin(), val % 2 + '0');		/* prepend bit to string						*/
		val /= 2;											/* shift by one to right						*/
	}														/*												*/
	return retval;											/*												*/
}															/*												*/
															/*----------------------------------------------*/
/*!
* \brief Check bits with string
*
* Check, if bits of integer match with ascii coded. Don't cares are possible.
*
* \param[in] a Integer to check
* \param[in] b String to check. May contain '0', '1' and 'x'
* \param[out] none
* \return boolean answer
* \note Global variables used: none
*/															/*----------------------------------------------*/
bool smtable::bitsMatch(int a, const char* b) {				/*												*/
	int length = 0;											/*												*/
	bool ret = true;										/*												*/
	for (length = 0; b[length] != '\0'; length++);			/* get length of string							*/
	for (int i = 0; i < length; i++) {						/* run through string b							*/
		if ((((a >> (length - i - 1)) & 0x1) != (b[i] - '0')) && b[i] != 'x') ret = false;/*				*/
	}														/* if bit doesn't match							*/
	return ret;												/*												*/
}															/*												*/
															/*----------------------------------------------*/

