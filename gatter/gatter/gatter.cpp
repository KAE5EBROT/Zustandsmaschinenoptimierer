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

/*!
* \brief Skip documentation
*
* Print the number of gatter and the DMF (gatter) to the console
*
* \param[in] string of the output file from minilog (e.g. zmnichtoptimiert.min)
* \param[out] none
* \return none
* \note Global variables used: none
*/
void parse(string infilename) { 
	int equal_sign_count = 0;									/*counts equal sign*/
	int gatter = 0;												/*counts gatter */
	int row = 0;												/*row into the truth table*/							
	int letter_on_row = 0;										/*letter in a row by the variable names*/						
	int inputs_index = 0;										/*index inputs*/		
	int outputs_index = 0;										/*index outputs*/
	bool equal_sign = true;									/*equal_sign is set when token == '=' */
	bool input_side = true;										/*input_side is set when token is left from | in the truth table */
	bool outputs_set = false;									/*outputs_set is false (save variables in inputs), outputs_set is true (save variables in outputs)*/		
	char token;													/*current token*/
	string cache;												/*cache for konjunktive gatter*/
	vector <int > output_ones;									/*save all ones for each output (column-wise)*/
	vector<string> variables;									/*save all variables*/
	vector<string> inputs, outputs;								/*seperate variables in inputs and outputs*/
	vector<string> DMF;											/*Disjunktive minimal form of the states coding*/

	ifstream inf(infilename.c_str());							/*open file*/

	while (inf.get(token)) {									/*next token*/
		if (token != '\n') {
			if (equal_sign == false || token == '=')
			{
				if (token == '=')
				{
					equal_sign = false;															/*when after a equal a equal comes*/
				}
				else
				{
					equal_sign = true;															/*when after a equal not a equal comes*/
					equal_sign_count++;															/*counts equal sign*/
					row = 0;
				}
			}

			if (equal_sign_count == 4 && equal_sign == true) {									/*after the 5. equal sign and not while the next equal sign*/
				letter_on_row++;
				if (row == 0) {																	/*skip return*/
					if (letter_on_row > variables.size()) {
						variables.resize(letter_on_row);										/*variables resize with the maximum letter_on_row*/
					}
				}																				/*save all variables (inputs and outputs) letter for letter*/
				if (token != ' ')
					variables.at(letter_on_row - 1) = variables.at(letter_on_row - 1) + token;		/*append one token of a variable*/
			}

			if (equal_sign_count == 5) {														/*after the 6. equal sign: read the truth table*/	
				if (token == '0' || token == '1' || token == '-' || token == '.') {
					if (input_side) {															/*on the left side of the | in the truth table (input side), create input combination in string cache		*/
						if (token != '-') {
							if (cache.size() > 0) {												/*checks, if a variable is already in cache*/
								cache = cache + " ^ ";											/*append a " and " between two inputs*/
							}
							if (cache == "") {													/*checks, if cache is empty then '('*/
								cache = '(';
							}
							if (token == '0') {													/*token == '0'*/
								cache = cache + inputs.at(inputs_index) + '\'';					/*append input name and not ('\'')*/
								gatter++;														/*increase gatter count*/
							}
							else {																/*token == '1'*/
								cache = cache + inputs.at(inputs_index);
								gatter++;														/*increase gatter count*/
							}
						}
						inputs_index++;															/*increase input index (next token comes)*/
						if (inputs_index == inputs.size()) {									/*checks, if all inputs */
							cache = cache + ')';
							gatter--;															/*decrease gatter count because the number of gatter is the number of inputs minus 1*/
							inputs_index = 0;													/*reset inputs_index*/
							input_side = !input_side;											/*input_side --> output_side of the truth table */
						}
					}
					else {																		/*on the right side of the | in the truth table (output side)*/
						if (token == '1') {													/*all other rows (except first row) in the truth table*/
							output_ones[outputs_index] += 1;								/*counts the output ones for each output */
							if (DMF.at(outputs_index).size() > 0) {
								DMF.at(outputs_index) = DMF.at(outputs_index) + " v ";		/*append a " or " between two outputs*/
							}
							DMF.at(outputs_index) = DMF.at(outputs_index) + cache;			/*save the next input combination to a output*/
						}
						outputs_index++;														/*increase output index (next token comes)*/
						if (outputs_index == outputs.size()) {
							cache.clear();
							row++;																/*next row, only important to decide first row from other rows*/
							input_side = !input_side;											/*output_side --> input_side of the truth table */
							outputs_index = 0;													/*reset outputs_index*/
						}
					}
				}
			}
		}
		else {
			if (equal_sign_count == 4 && letter_on_row == 0) {									/*while the 4. equal sign, prepare inputs and outputs*/
				for (int i = 0; i < variables.size(); i++) {									/*seperate all variables in inputs, outputs and delete empty strings*/
					for (int x = 0; x < variables.at(i).size(); x++) {							/*copy letter for letter*/
						cache = cache + variables.at(i).at(x);									/*append letter*/
					}
					if (!outputs_set) {															/*save inputs*/
						if (cache != "") {
							inputs.push_back(cache);											/*push a input in inputs*/
							cache.clear();
						}
						else {
							outputs_set = true;													/*if cache is "" (empty) --> then the outputs comes (three time is cache empty see at textdocument.min */
						}
					}
					else {																		/*save outputs*/
						if (cache != "") {
							outputs.push_back(cache);											/*push a output in outputs*/
							cache.clear();
						}
					}
				}
				output_ones.resize(outputs.size());												/*resize the vector<string> output_ones*/
				for (int i = 0; i < outputs.size(); i++)
					output_ones[i] = 0;
				DMF.resize(outputs.size());														/*resize the vector<string> DMF*/
			}
			letter_on_row = 0;
			row++;
		}
	}

	inf.close();																				/*close file*/

	for (int i = 0; i < outputs.size(); i++) {
		if (output_ones[i] != 0) {
			gatter = gatter + output_ones[i] - 1;												/*add the number of output ones minus 1 (or gatter)*/		
		}
	}

	cout << "Gatteranzahl von '" << infilename.c_str()  << "': " << gatter << "\n";				/*print number of gatter*/

	cout << "Gatter:"  << "\n";
	for (int i = 0; i < outputs.size(); i++) {
		cout << outputs[i] << " = " << DMF[i] << "\n";											/*print DMF*/
	}
}

int main()
{
	string infnopt("zmnichtoptimiert.min");												
	string infopt("zmoptimiert.min");														
	parse(infnopt);																				/*pass parse not optimize document*/
	parse(infopt);																				/*pass parse optimize document*/
	cin.get();
}

