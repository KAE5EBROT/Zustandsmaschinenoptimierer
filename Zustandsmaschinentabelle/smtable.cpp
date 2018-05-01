
#include "stdafx.h"
#include <iomanip>
#include <iostream>
#include <vector>
#include <map>
#include "smtable.h"
using namespace std;


smtable::fstate smtable::init() {
	vector<entry> temp;
	for (int i = 0; i < iinputs.size(); i++) iwidth *= 2;
	iheight = istates.size();
	temp.resize(iwidth);
	for (int i = 0; i < iwidth; i++) {
		for (int j = 0; j < ioutputs.size(); j++)
			temp.at(i).out_list.append("x");
	}
	for (int i = 0; i < istates.size(); i++) {
		table.insert(pair<string, vector<entry>>(istates.at(i), temp));
	}
	return eOK;
}


smtable::fstate smtable::init(elementlist states, elementlist inputs, elementlist outputs) {
	vector<entry> temp;
	istates = states;
	iinputs = inputs;
	ioutputs = outputs;
	for (int i = 0; i < iinputs.size(); i++) iwidth *= 2;
	iheight = istates.size();
	temp.resize(iwidth);
	for (int i = 0; i < iwidth; i++) {
		for (int j = 0; j < ioutputs.size(); j++)
			temp.at(i).out_list.append("x");
	}
	for (int i = 0; i < istates.size(); i++) {
		table.insert(pair<string, vector<entry>>(istates.at(i), temp));
	}
	return eOK;
}

smtable::fstate smtable::setStates(elementlist inputs) {
	istates = inputs;
	return eOK;
}

smtable::fstate smtable::setInputs(elementlist inputs) {
	iinputs = inputs;
	return eOK;
}

smtable::fstate smtable::setOutputs(elementlist outputs) {
	ioutputs = outputs;
	return eOK;
}

smtable::fstate smtable::link(elementlist inputs, string inputval, string srcstate, elementlist outputs, string outputval, string dststate) {
	map<string, vector<entry>>::iterator it = table.begin();
	int itcount = 0;
	int numberOfMatchingInput = 1;
	int numberOfInput = (1 << iinputs.size());
	bool foundMatch = false;
	entry tempentry;
	tempentry.next_state = dststate;
	for (int i = 0; i < ioutputs.size(); i++) { /* run through known outputs */
		for (int j = 0; (j < outputs.size()) && !foundMatch; j++) { /* run through outputs */
			if (outputs.at(j) == ioutputs.at(i)) { /* output specified? */
				char tempOutVal[2] = { 0, 0 };
				tempOutVal[0] = outputval.at(j);   /* YES! copy value */
				tempentry.out_list.append(tempOutVal);
				foundMatch = true;
			}
		}
		if (!foundMatch) tempentry.out_list.append("x");
		foundMatch = false;
	}	/* tempentry set up correctly, now has to be copied to matching input values */


	foundMatch = false;
	char* tempInVal = new char[iinputs.size()+1];
	//for (int i = 0; i < iinputs.size(); i++) tempInVal[i] = 'x';
	tempInVal[iinputs.size()] = '\0';
	for (int i = 0; i < iinputs.size(); i++) { /* run through known inputs */
		for (int j = 0; (j < inputs.size()) && !foundMatch; j++) { /* run through inputs */
			if (inputs.at(j) == iinputs.at(i)) { /* input specified? */
				tempInVal[i] = inputval.at(j);   /* YES! copy value */
				foundMatch = true;
			}
		}
		if (!foundMatch) tempInVal[i] = 'x';
		foundMatch = false;
	}	/* input values extracted */
	for (int i = 0; i < numberOfInput; i++) {
		if (bitsMatch(i, tempInVal)) table[srcstate].at(i) = tempentry;
	}

	//table[srcstate].at(1).next_state = dststate;
	//table[srcstate].at(1).out_list = outputval;//todo teilweisedefinierte Ausgänge; it->at()

	delete tempInVal;
	return eOK;
}

smtable::fstate smtable::print() {
	cout << "Reihenfolge Eingangssignale: ";
	for (int i = 0; i < iinputs.capacity(); i++) cout << iinputs.at(i).c_str() << " ";
	cout << "\n\t";
	for (int i = 0; i < iwidth; i++) {
		string *bits = int2bit(i, iinputs.capacity());
		cout << bits->c_str() << "\t";
		delete bits;
	}
	cout << "\n-------|-------|-------|-------|-------|-------|-------|-------|-------";
	for (int i = 0; i < iheight; i++) {
		cout << "\n" << istates.at(i).c_str() << "\t";
		for (int j = 0; j < iwidth; j++) {
			cout << table[istates.at(i)].at(j).next_state.c_str() << "\t";
		}
	}
	cout << "\n-------|-------|-------|-------|-------|-------|-------|-------|-------";
	for (int i = 0; i < iheight; i++) {
		cout << "\n" << istates.at(i).c_str() << "\t";
		for (int j = 0; j < iwidth; j++) {
			cout << table[istates.at(i)].at(j).out_list.c_str() << "\t";
		}
	}
	return eOK;
}

string *smtable::int2bit(int val, int width) {
	string *retval = new string;
	string::iterator strbegin = retval->begin();
	for (int i = 0; i < width; i++) {
		retval->insert(strbegin,val % 2 + '0');
		val /= 2;
	}
	return retval;
}

bool smtable::bitsMatch(int a, char* b) {
	int length = 0;
	bool ret = true;
	for (length = 0; b[length] != '\0'; length++);
	for (int i = 0; i < length; i++) {
		if ((((a >> (length - i - 1)) & 0x1) != (b[i] - '0')) && b[i] != 'x') ret = false;
	}
	return ret;
}