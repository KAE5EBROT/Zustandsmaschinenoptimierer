// k2vec1.cpp
//
#define USESQUARETABLE 0
#include "stdafx.h"
#include <iomanip>
#include <iostream>
#include <vector>
#include <list>
using namespace std;

typedef enum {
	eOK,
	eFAIL
}fstate;

class Ctable {
public:
	typedef struct {
		string state; /* Erste Spalte: Aktueller State, Sonst Nächster State*/
		string out_list;
	} entry;
	list<vector<entry>> table;
	int iheight;
	int iwidth;
	//list<vector<entry>>::iterator it_list;

	fstate size(int height, int width, int numberOfOutputs) {
		//table.resize(height); /* allocate table columns */
		/*for (it_list = table.begin; it_list = table.end; it_list++) {
			(*it_list).resize(width);
		}*/
		vector<entry> temp;
		temp.resize(width);
		for (int i = 0; i < width; i++) {
			for (int j = 0; j < numberOfOutputs; j++)
				temp.at(i).out_list.append("x");
		}
		for (int i = 0; i<height; i++) {
			table.push_back(temp);
		}
		iheight = height;
		iwidth = width;
		return eOK;
	}

	fstate setStates(string inputs[]) {
		list<vector<entry>>::iterator it = table.begin();
		for (int i = 0; i < iheight; i++, it++) {
			(*it).at(0).state = inputs[i];
		}
		return eOK;
	}

	fstate setInputs(string inputs) {

	}

	fstate setOutputs(string inputs) {

	}

	fstate link(string inputs, string inputval, string srcstate, string outputs, string outputval, string dststate) {

	}

};

Ctable t;

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
	t.size(table_height, table_width, output_count);
	string teststrings[3] = { "State1","state_B","State_3" };
	t.setStates(teststrings);
	char temp[3];
	cin >> temp;
}