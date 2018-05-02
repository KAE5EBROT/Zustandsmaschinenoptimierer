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

int main()
{
	int j = 0;				//Anzahl Gleichketten
	int k = 0;				//Anzahl Gatter
	int i = 0;				//Index
	int zeile = 0;
	int anz = 1;
	vector <int > i_vec;
	char t;					//aktuelles Zeichen

	//Eingabe
	ifstream inf("Beispiel.min");

	//
	while (inf.get(t))          // loop getting single characters
	{
		for (j = 0; j < 5; j++)
		{
			k = 0;
			while (t != '=')
				inf.get(t);

			while (k == 0)
			{
				inf.get(t);

				if (t == '=')
				{
					k = 0;
				}
				else
				{
					k = 1;
				}
			}
		}
		
		if (j > 4) {
			inf.get(t);						//= überspringen
			
			k = 0;
			while (t == '0' || t == '1' || t == '-')
			{
				while (t != '|'){
					if (t == '0' || t == '1') {
						k++;
					}
					inf.get(t);
				}
				k--;
				
				inf.get(t);					//	| überspringen
				inf.get(t);					//Leerzeichen überspringen

				if (zeile == 0) {
					for (i = 0; i < anz; i++) {
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
						inf.get(t);
					}
					zeile++;
				}
				else {
					for (i = 0; i < anz; i++) {
						if (t == '1') {
							i_vec[i] += 1;
						}
						inf.get(t);
					}
				}
			}

			for (i = 0; i < anz-1; i++) {
				if (i_vec[i] != 0) {
					k=k+i_vec[i]-1;
				}
				inf.get(t);
			}
			cout << "Gatteranzahl: " << k << "\n" ;
		}	
	}
	
	inf.close();
}

