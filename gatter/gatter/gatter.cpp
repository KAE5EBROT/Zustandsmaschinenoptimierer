// gatter.cpp: Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include "stdafx.h"
#include <stdio.h>
#include <string>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <vector>


using namespace std;
void parse(string infilename) {
	ifstream inf(infilename.c_str());
	int equal_sign_count = 0;						//Anzahl Gleichketten
	int i = 0;
	int equal_sign = 0;				
	int gatter = 0;					//Anzahl Gatter
	int row = 0;
	int anz = 1;
	int letter_on_row = 0;
	bool input_state = true;
	vector <int > i_vec;
	char token;					//aktuelles Zeichen
	int eingaenge_index = 0;
	int ausgaenge_index = 0;
	int input_count = 4;
	bool eingangsseite = true;
	bool ausgaenge_set = false;
	string h;
	vector<string> eingaenge, ausgaenge;
	vector<string> variables;
	vector<string> DMF;

	while (inf.get(token)) {				//next token

		if (token == '=')
		{
			equal_sign = 0;
		}
		if (equal_sign == 0)
		{
			if (token == '=')
			{
				equal_sign = 0;
			}
			else
			{
				equal_sign = 1;
				equal_sign_count++;
			}
		}

		if (equal_sign_count == 5 && equal_sign == 1) {
			if (token == '\n') {
				letter_on_row = 0;
			}
			else {
				letter_on_row++;
				if (letter_on_row > variables.size()) {
					variables.resize(letter_on_row);
				}
				variables.at(letter_on_row - 1) = variables.at(letter_on_row - 1) + token;
			}
		}
		if (equal_sign_count == 7) {
			if (token == '0' || token == '1' || token == '-' || token == '.') {
				if (eingangsseite) {
					if (token != '-') {
						if (h.size() > 0) {
							h = h + " and ";
						}
						if (token == '0') {
							if (h == "") {
								h = '(';
							}
							h = h + eingaenge.at(eingaenge_index) + '\'';
							gatter++;
						}
						else if (token == '1') {
							if (h == "") {
								h = '(';
							}
							h = h + eingaenge.at(eingaenge_index);
							gatter++;
						}	
					}
					if (eingaenge_index == eingaenge.size() - 1) {
						h = h + ')';
						gatter--;
					}
					eingaenge_index++;
				}
				else {
					if (token == '1') {
						if (DMF.at(ausgaenge_index).size() > 0) {
							DMF.at(ausgaenge_index) = DMF.at(ausgaenge_index) + " or ";
						}
						DMF.at(ausgaenge_index) = DMF.at(ausgaenge_index) + h;
					}
					else if (token == '.') {

					}
					if (row == 0) {
						if (token == '1') {
							i_vec.resize(anz);
							i_vec[i] = 1;
							anz = anz + 1;
						}
						else if (token == '.') {
							i_vec.resize(anz);
							i_vec[i] = 0;
							anz = anz + 1;
						}
					}
					else {		
						if (token == '1') {
						i_vec[i] += 1;
						}
					}
					ausgaenge_index++;
					if (ausgaenge_index == ausgaenge.size()) {
						h = "";
						row++;
					}
				}
			}

			if (token == '|' || token == '\n') {
				eingangsseite = !eingangsseite;
				if (input_state) {
					DMF.resize(variables.size() - eingaenge_index - 3);
					input_state = false;
				}
				ausgaenge_index = 0;
				eingaenge_index = 0;
			}
		}
		if (equal_sign_count == 6) {
			for (int i = 0; i < variables.size(); i++) {
				if (!ausgaenge_set) {
					for (int x = 0; x < variables.at(i).size(); x++) {
						if(variables.at(i).at(x) != ' ')
						h = h + variables.at(i).at(x);
					}
					if (h != "") {
						eingaenge.push_back(h);
						h.clear();
					}
					else {
						ausgaenge_set = true;
					}
				}
				if (ausgaenge_set) {
					for (int x = 0; x < variables.at(i).size(); x++) {
						if (variables.at(i).at(x) != ' ')
							h = h + variables.at(i).at(x);
					}
					if (h != "") {
						ausgaenge.push_back(h);
						h.clear();
					}

				}
			}
			equal_sign_count++;
		}
	}

	inf.close();

	for (i = 0; i < anz - 1; i++) {
		if (i_vec[i] != 0) {
			gatter = gatter + i_vec[i] - 1;
		}
	}

	cout << "Gatteranzahl von '" << infilename.c_str()  << "': " << gatter << "\n";

	cout << "Gatter:"  << "\n";
	for (int i = 0; i < ausgaenge.size(); i++) {
		cout << ausgaenge[i] << " = " << DMF[i] << "\n";
	}
}

int main()
{
	string infnopt("../../zmnichtoptimiert.min");
	string infopt("../../zmoptimiert.min");
	parse(infnopt);
	parse(infopt);
	cin.get();
}

