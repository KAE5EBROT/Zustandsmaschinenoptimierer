#pragma once

#include <vector>
#include <map>


using namespace std;
typedef unsigned int uint;

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
	int iheight;				/// Effectively the number of states
	int iwidth = 1;				/// Effectively the number of possible input combinations
	typedef vector<string> elementlist;
	elementlist istates;		/// Vector of known states
	elementlist iinputs;		/// Vector of known inputs
	elementlist ioutputs;		/// Vector of known outputs

	/// Initializes the table. Expects the finished setup by the setXx()-functions.
	fstate init();

	/// Initializes the table. Does setup of states, inputs and outputs, too.
	fstate init(elementlist states, elementlist inputs, elementlist outputs);

	/// Setup of states
	fstate setStates(elementlist inputs);

	/// Setup of inputs
	fstate setInputs(elementlist inputs);

	/// Setup of outputs
	fstate setOutputs(elementlist outputs);

	/// Link states on specific input patterns and set outputs.
	/// inputs is a vector of strings, containing the exact names of inputs which values are specified in inputval.
	/// The size of inputs must match the length of inputval. The values can be '0', '1' or 'x'.
	/// If a pair of input and its value is not mentioned, its treated as 'x'.
	fstate link(elementlist inputs, string inputval, string srcstate, elementlist outputs, string outputval, string dststate);

	/// Print content of table.table in familiar manner
	fstate print();

	/// Converts val into string representing the value of val in binary using ASCII ones and zeros.
	/// The return pointer is allocated dynamically and has to be deleted after processing!
	string *int2bit(int val, int width);

	/// checks, if int a matches with bit pattern char* b. b is expected as string containing either '0', '1' or 'x'.
	bool bitsMatch(int a, char * b);

};