
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
*/
smtable::fstate smtable::init() {
	vector<entry> temp;
	for (uint i = 0; i < iinputs.size(); i++) iwidth *= 2;
	iheight = istates.size();
	temp.resize(iwidth);
	for (int i = 0; i < iwidth; i++) {
		for (uint j = 0; j < ioutputs.size(); j++)
			temp.at(i).out_list.append("x");
	}
	for (uint i = 0; i < istates.size(); i++) {
		table.insert(pair<string, vector<entry>>(istates.at(i), temp));
	}
	return eOK;
}

/*!
* \brief Initialize table with 
*
* 
* 
* \warning setStates(), setInputs() and setOutputs() have to be called beforehand
*
* \param[in] none
* \param[out] none
* \return operation success
* \note Global variables used: table
*/
smtable::fstate smtable::init(elementlist states, elementlist inputs, elementlist outputs) {
	vector<entry> temp;
	istates = states;
	iinputs = inputs;
	ioutputs = outputs;
	for (uint i = 0; i < iinputs.size(); i++) iwidth *= 2;
	iheight = istates.size();
	temp.resize(iwidth);
	for (int i = 0; i < iwidth; i++) {
		for (uint j = 0; j < ioutputs.size(); j++)
			temp.at(i).out_list.append("x");
	}
	for (uint i = 0; i < istates.size(); i++) {
		table.insert(pair<string, vector<entry>>(istates.at(i), temp));
	}
	return eOK;
}

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
smtable::fstate smtable::setStates(elementlist inputs) {
	istates = inputs;
	return eOK;
}

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
smtable::fstate smtable::setInputs(elementlist inputs) {
	iinputs = inputs;
	return eOK;
}

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
smtable::fstate smtable::setOutputs(elementlist outputs) {
	ioutputs = outputs;
	return eOK;
}

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
smtable::fstate smtable::link(elementlist inputs, string inputval, string srcstate, elementlist outputs, string outputval, string dststate) {
	int numberOfMatchingInput = 1;
	int numberOfInput = (1 << iinputs.size());
	bool foundMatch = false;
	entry tempentry;
	/* next state unambiguous, just copy */
	tempentry.next_state = dststate;
	/* Now comes the tricky part: Inputs can either be assigned 1, 0 or x or be unassigned, then x	*/
	for (uint i = 0; i < ioutputs.size(); i++) {		/* run through known outputs					*/
		for (uint j = 0; (j < outputs.size()) && !foundMatch; j++) { /* run through given outputs	*/
			if (outputs.at(j) == ioutputs.at(i)) {	/* output specified?							*/
				char tempOutVal[2] = { 0, 0 };		/* temp string to append						*/
				tempOutVal[0] = outputval.at(j);	/* YES! copy value								*/
				tempentry.out_list.append(tempOutVal); /* and save									*/
				foundMatch = true;					/* ignore remainder								*/
			}
		}
		if (!foundMatch) tempentry.out_list.append("x"); /* not found at all? Unassigned, then x	*/
		foundMatch = false;							/* reset for next run							*/
	}	/* tempentry set up correctly, now has to be copied to matching input values				*/

	/* Another tricky part: extract info, where to copy it all (could be ambiguous)					*/
	foundMatch = false;								/* reuse/reset again							*/
	char* tempInVal = new char[iinputs.size()+1];	/* set up a temporary string					*/
	tempInVal[iinputs.size()] = '\0';				/* determine end of string						*/
	for (uint i = 0; i < iinputs.size(); i++) {		/* run through known inputs						*/
		for (uint j = 0; (j < inputs.size()) && !foundMatch; j++) { /* run through inputs			*/
			if (inputs.at(j) == iinputs.at(i)) {	/* input specified?								*/
				tempInVal[i] = inputval.at(j);		/* YES! copy value								*/
				foundMatch = true;					/* ignore remainder								*/
			}
		}
		if (!foundMatch) tempInVal[i] = 'x';
		foundMatch = false;
	}	/* input values extracted																	*/
	for (int i = 0; i < numberOfInput; i++) {		/* run through all input combinations			*/
		if (bitsMatch(i, tempInVal)) table[srcstate].at(i) = tempentry; /* on match copy			*/
	}

	delete tempInVal;								/* don't forget to clear heap					*/
	return eOK;
}

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
smtable::fstate smtable::print() {
	/* print correct input and output sequences														*/
	cout << "Reihenfolge Eingangssignale: ";
	for (uint i = 0; i < iinputs.capacity(); i++) cout << iinputs.at(i).c_str() << " ";
	cout << "\n";
	cout << "Reihenfolge Ausgangssignale: ";
	for (uint i = 0; i < ioutputs.capacity(); i++) cout << ioutputs.at(i).c_str() << " ";
	cout << "\n\n\t";

	/* print all input combinations																	*/
	for (int i = 0; i < iwidth; i++) { 
		string *bits = int2bit(i, iinputs.capacity());
		cout << bits->c_str() << "\t";
		delete bits;								/* don't forget to clear heap					*/
	}
	cout << "\n-------";							/* table separator								*/
	for (int i = 0; i < iwidth; i++) cout << "|-------";/* table separator							*/
	
	/* print next states determined by current state and input										*/
	for (int i = 0; i < iheight; i++) {
		cout << "\n" << istates.at(i).c_str() << "\t";
		for (int j = 0; j < iwidth; j++) {
			cout << table[istates.at(i)].at(j).next_state.c_str() << "\t";
		}
	}
	cout << "\n-------";							/* table separator								*/
	for (int i = 0; i < iwidth; i++) cout << "|-------";/* table separator							*/

	/* print output values determined by current state and input									*/
	for (int i = 0; i < iheight; i++) {
		cout << "\n" << istates.at(i).c_str() << "\t";
		for (int j = 0; j < iwidth; j++) {
			cout << table[istates.at(i)].at(j).out_list.c_str() << "\t";
		}
	}
	cout << "\n\n";
	return eOK;
}

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
string *smtable::int2bit(int val, int width) {
	string *retval = new string;
	string::iterator strbegin = retval->begin();	/* set pointer to beginning of string			*/
	for (int i = 0; i < width; i++) {				/* run for given length							*/
		retval->insert(strbegin, val % 2 + '0');	/* prepend bit to string						*/
		val /= 2;									/* shift by one to right						*/
	}
	return retval;
}

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
bool smtable::bitsMatch(int a, const char* b) {
	int length = 0;
	bool ret = true;
	for (length = 0; b[length] != '\0'; length++);	/* get length of string							*/
	for (int i = 0; i < length; i++) {				/* run through string b							*/
		if ((((a >> (length - i - 1)) & 0x1) != (b[i] - '0')) && b[i] != 'x') ret = false;
	}												/* if bit doesn't match							*/
	return ret;
}

