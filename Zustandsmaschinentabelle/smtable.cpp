
#include "stdafx.h"
#include <iomanip>
#include <iostream>
#include <vector>
#include <map>
#include "smtable.h"
using namespace std;


smtable::fstate smtable::init(/*int state_count, int input_count, int output_count*/) {
	vector<entry> temp;
	for (int i = 0; i < iinputs.size(); i++) iwidth *= 2;
	iheight = istates.size();
	temp.resize(iwidth);
	for (int i = 0; i < iwidth; i++) {
		for (int j = 0; j < ioutputs.size(); j++)
			temp.at(i).out_list.append("x");
	}
	for (int i = 0; i < istates.size(); i++) {
		table.insert(pair<string,vector<entry>>(istates.at(i),temp));
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
	table[srcstate].at(1).next_state = dststate;
	table[srcstate].at(1).out_list = outputval;//todo teilweisedefinierte Ausgänge
											//it->at()
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