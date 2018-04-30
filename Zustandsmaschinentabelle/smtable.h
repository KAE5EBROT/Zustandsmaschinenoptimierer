#pragma once

#include <vector>
#include <map>


using namespace std;

class smtable {
public:
	typedef struct {
		string next_state;
		string out_list;
	} entry;
	typedef enum {
		eOK,
		eFAIL
	}fstate;
	typedef map<string, vector<entry>> tabletype;
	tabletype table; /* map<state,row> */
	int iheight;
	int iwidth = 1;
	typedef vector<string> elementlist;
	elementlist istates;
	elementlist iinputs;
	elementlist ioutputs;

	fstate init(/*int state_count, int input_count, int output_count*/);

	fstate setStates(elementlist inputs);

	fstate setInputs(elementlist inputs);

	fstate setOutputs(elementlist outputs);

	fstate link(elementlist inputs, string inputval, string srcstate, elementlist outputs, string outputval, string dststate);

	fstate print();

	string int2bit(int val, int width);

};