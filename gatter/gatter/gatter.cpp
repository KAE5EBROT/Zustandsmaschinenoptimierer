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
	int j = 0;				//Anzahl Gleichketten
	int k = 0;				//Anzahl Gatter
	int i = 0;				//Index
	int gatter = 0;
	int i_index_string = 0;
	int zeile = 0;
	int anz = 1;
	int letter_on_row = 0;
	vector <int > i_vec;
	char t;					//aktuelles Zeichen
	int eingaenge_index = 0;
	int ausgaenge_index = 0;
	int input_count = 0;
	bool eingangsseite = true;
	string h;
	vector<string> eingaenge, ausgaenge;
	vector<string> variables;
	eingaenge.push_back("e");
	eingaenge.push_back("e1");
	eingaenge.push_back("Z0");
	eingaenge.push_back("Z1");
	ausgaenge.push_back("Z0+");
	ausgaenge.push_back("Z1+");
	vector<string> DMF(ausgaenge.size());

	while (inf.get(t)) {

		if (t == '=')
		{
			k = 0;
		}
		if (k == 0)
		{
			if (t == '=')
			{
				k = 0;
			}
			else
			{
				k = 1;
				j++;
			}
		}
		if (j == 5 && k == 1) {
			if (t == '\n') {
				letter_on_row = 0;
			}
			else if (t == ' ') {

			}
			else {
				letter_on_row++;
				if (letter_on_row > variables.size()) {
					variables.resize(letter_on_row);
				}
				variables.at(letter_on_row - 1) = variables.at(letter_on_row - 1) + t;
			}
		}
		if (j == 7) {
			if (t == '0' || t == '1' || t == '-' || t == '.') {
				if (eingangsseite) {
					if (t != '-') {
						if (h.size() > 0) {
							h = h + " and ";
						}
						if (t == '0') {
							if (h == "") {
								h = '(';
							}
							h = h + variables.at(eingaenge_index) + '\'';
							gatter++;
						}
						else if (t == '1') {
							if (h == "") {
								h = '(';
							}
							h = h + variables.at(eingaenge_index);
							gatter++;
						}	
					}
					if (eingaenge_index == eingaenge.size() - 1) {
						h = h + ')';
						gatter--;
					}
					eingaenge_index++;
					if (zeile == 0) {
						input_count = eingaenge_index;
					}
				}
				else {
					if (t == '1') {
						if (DMF.at(ausgaenge_index).size() > 0) {
							DMF.at(ausgaenge_index) = DMF.at(ausgaenge_index) + " or ";
						}
						DMF.at(ausgaenge_index) = DMF.at(ausgaenge_index) + h;
					}
					else if (t == '.') {

					}
					if (zeile == 0) {
						if (t == '1') {
							i_vec.resize(anz);
							i_vec[i] = 1;
							anz = anz + 1;
						}
						else if (t == '.') {
							i_vec.resize(anz);
							i_vec[i] = 0;
							anz = anz + 1;
						}
					}
					else {		
						if (t == '1') {
						i_vec[i] += 1;
						}
					}
					ausgaenge_index++;
					if (ausgaenge_index == ausgaenge.size()) {
						h = "";
						zeile++;
					}
				}
			}

			if (t == '|' || t == '\n') {
				eingangsseite = !eingangsseite;
				ausgaenge_index = 0;
				eingaenge_index = 0;
			}
		}
		if (j == 6) {
			j++;
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
	int l = 0;
	for (int i = input_count; i < variables.size(); i++, l++) {
		cout << variables[i] << " = " << DMF[l] << "\n";
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

