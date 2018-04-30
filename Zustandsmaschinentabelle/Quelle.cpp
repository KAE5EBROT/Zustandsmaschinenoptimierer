// k2vec1.cpp
//
#define USESQUARETABLE 0
#include "stdafx.h"
#include <iomanip>
#include <iostream>
#include <vector>
#include <map>
#include "smtable.h"
using namespace std;



smtable t;

int main(void)
{
	int state_count = 3;
	int input_count = 3;
	int output_count = 2;
	int table_height = 2;
	int table_width = 2;
	cout << "Testprogram table allocation!\n";
#if 0
	vector<int> i_vec(10); // 10 elements of type int allocated
	vector<int> i_vec2; // Object i_vec2 created  
#endif // 0
#if USESQUARETABLE
	if (state_count > input_count) {
		for (int i = 1; i < state_count; i++) table_height *= 2;
		for (int i = 1; i < state_count; i++) table_width *= 2;
	}
	else {
		for (int i = 1; i < input_count; i++) table_height *= 2;
		for (int i = 1; i < input_count; i++) table_width *= 2;
	}
#else
	table_height = state_count;
	for (int i = 1; i < input_count; i++) table_width *= 2;
#endif // USESQUARETABLE
	smtable::elementlist teststates;
	teststates.push_back("State1");
	teststates.push_back("state_B");
	teststates.push_back("State_3");
	smtable::elementlist testinputs;
	testinputs.push_back("In1");
	testinputs.push_back("in_B");
	testinputs.push_back("In_3");
	smtable::elementlist testoutputs;
	testoutputs.push_back("Out1");
	testoutputs.push_back("out_B");
	testoutputs.push_back("Out_3");
	t.setStates(teststates);
	t.setInputs(testinputs);
	t.setOutputs(testoutputs);
	t.init();
	t.link({ "In1","in_B" }, "10", "State1", { "Out_3","Out1" }, "10", "State_3");
	char temp[3];
	cin >> temp;
}