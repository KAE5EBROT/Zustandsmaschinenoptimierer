// k2vec1.cpp
//
#define USESQUARETABLE 0
#include "stdafx.h"
#include <iomanip>
#include <iostream>
#include <vector>
#include <list>
using namespace std;

class Ctable {
public:
	typedef struct {
		char state; /* Erste Spalte: Aktueller State, Sonst Nächster State*/
		char out;
	} entry;
	list<vector<entry>> table;
	//list<vector<entry>>::iterator it_list;

	void size(int height, int width) {
		//table.resize(height); /* allocate table columns */
		/*for (it_list = table.begin; it_list = table.end; it_list++) {
			(*it_list).resize(width);
		}*/
		vector<entry> temp;
		temp.resize(width);
		for (int i = 0; i<height; i++) {
			table.push_back(temp);
		}
	}

};

Ctable t;

void main(void)
{
	int state_count = 3;
	int input_count = 3;
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
	t.size(table_height, table_width);
	char temp[3];
	cin >> temp;
}