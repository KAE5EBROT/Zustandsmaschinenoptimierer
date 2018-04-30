
#include "stdafx.h"
#include <iomanip>
#include <iostream>
#include <vector>
#include <map>
#include "smtable.h"
using namespace std;


smtable::fstate smtable::init(/*int state_count, int input_count, int output_count*/) {
	vector<entry> temp;
	for (int i = 1; i < iinputs.size; i++) iwidth *= 2;
	iheight = istates.size;
	temp.resize(iwidth);
	for (int i = 1; i < iwidth; i++) {
		for (int j = 0; j < ioutputs.size; j++)
			temp.at(i).out_list.append("x");
	}
	for (int i = 0; i < istates.size; i++) {
		table.insert.insert(temp);
	}
	return eOK;
}

smtable::fstate smtable::setStates(elementlist inputs) {
	tabletype::iterator it = table.begin();
	for (int i = 0; i < iheight; i++, it++) {
		it->first.at(i) = inputs.at(i); // vielleicht beim initialisiern eintragen
	}
	return eOK;
}

smtable::fstate smtable::setInputs(elementlist inputs) {
	iinputs = inputs;
}

smtable::fstate smtable::setOutputs(elementlist outputs) {
	ioutputs = outputs;
}

smtable::fstate smtable::link(elementlist inputs, string inputval, string srcstate, elementlist outputs, string outputval, string dststate) {
	map<string, vector<entry>>::iterator it = table.begin();
	int itcount = 0;
	table[srcstate].at(1).next_state = dststate;
	table[srcstate].at(1).out_list = "0";//testvalue todo
											//it->at()
	return eOK;
}
