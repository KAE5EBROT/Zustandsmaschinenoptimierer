// k7scan1.cpp : Definiert den Einsprungpunkt f�r die Konsolenanwendung.
//

#include "stdafx.h"
#include "main.h"
#include "lex.h"

#pragma warning(disable:4786)
using namespace std;

int main(int argc, char* argv[])
{
	FILE *inf;
	char fistr[100];
	prioritytype high_priority, mean_priority;				/* map of state combinations to prioritize		*/
	lowpriotype low_priority;								/* different structure for low priority			*/
	smtable::elementlist Zustandscodierung;					/* vector of optimized state name sequence		*/
	if (argc == 1) {
		printf("Enter .txt filename:\n");
		scanf("%s", fistr);//gets(fistr);
		inf = fopen(strcat(fistr, ".txt"), "r");
	}
	else {
		inf = fopen(argv[1], "r");
	}
	if(inf==NULL){
		printf("Cannot open input file %s\n",fistr);
		return 0;
	}
	CParser obj;
	smtable table;
	obj.InitParse(inf,stderr,stdout);
//	obj.pr_tokentable();
	obj.yyparse(table);
	table.print();											/* 												*/
															/* 												*/
															/* Codierungsoptimierung						*/
	high_priority = highPriority(table);					/*												*/
	mean_priority = meanPriority(table);					/*												*/
	low_priority = lowPriority(table);						/*												*/
															/*												*/
															/* lowPriority() creates a list of state combinations, satisfying low priority conditions.				*/
															/* This list will most likely contain similar entries. removeSunsets() only leaves the biggest			*/
															/* combinations in that list. These combinations are not compatible with each other.					*/
	removeSubsets(low_priority);							/*												*/
															/*												*/
	Zustandscodierung = optimize(high_priority, mean_priority, low_priority, table);/*						*/
															/*												*/
															/* Write output files							*/
	if (writeOutputFile(table)) {							/*												*/
		fprintf(stderr, "Ausgabedatei \"ZMnichtoptimiert.tbl\" fehlgeschlagen\n");/*						*/
	}														/*												*/
	else {													/*												*/
		cout << "Ausgabedatei \"ZMnichtoptimiert.tbl\" erfolgreich geschrieben\n";/*						*/
	}														/*												*/
	if (writeOutputFile(Zustandscodierung,table)) {			/*												*/
		fprintf(stderr, "Ausgabedatei \"ZMoptimiert.tbl\" fehlgeschlagen\n");/*								*/
	}														/*												*/
	else {													/*												*/
		cout << "Ausgabedatei \"ZMoptimiert.tbl\" erfolgreich geschrieben\n";/*								*/
	}														/*												*/

	if (argc < 2) {
		char c; cin >> c;
	}
	return 0;
}

/*!
* \brief Skip documentation
*
* Get the high priorities of the state diagramm and return them
*
* \param[in] none
* \param[out] none
* \return prioritytype highPriority
* \note Global variables used: class smtable table: iwidth, iheight, istates and tabletype table
*/
prioritytype highPriority(smtable &table)					/* high priority: when at least two states have */
{															/* the same next state by the same input value	*/
	prioritytype high_priority;								/* save all high priorities in a map<string,vector<string>>	*/
	vector<string> candidates;								/* candidates for high priority					*/
	int candidate_count = 0;								/*												*/
															/*												*/
	for (int k = 0; k < table.iwidth; k++)					/*												*/
	{														/*												*/
		for (int j = 0; j < table.iheight; j++) {			/*												*/
			for (int i = 0; i < table.iheight; i++) {		/*												*/
				if (i != j) {				/* checks, if the table height j is unequal the table height i	*/
					if (table.istates.at(j) == table.table[table.istates.at(i)].at(k).next_state) {	/*		*/
										/* checks, if a state is equal with a next state from a other state	*/
						candidate_count++;					/* count candidates								*/
						candidates.push_back(table.istates.at(i).c_str());/* save candidate					*/
					}										/*												*/
				}											/*												*/
			}												/*												*/
			if (candidate_count >= 2) {						/* condition for high priority					*/
				high_priority[table.istates.at(j).c_str()] = candidates;/* if there are more candidates than*/
			}										/* one, then the high priority map is passed candidates	*/
			candidates.clear();								/* clear candidates								*/
			candidate_count = 0;							/* reset candidate_count						*/
		}													/*												*/
	}														/*												*/
	return high_priority;									/*												*/
}															/*												*/
															/*----------------------------------------------*/
/*!
* \brief Skip documentation
*
* Get the mean priorities of the state diagramm and return them
*
* \param[in] none
* \param[out] none
* \return prioritytype meanPriority
* \note Global variables used: class smtable table: iwidth, iheight, istates and tabletype table
*/
prioritytype meanPriority(smtable &table)					/* mean priority: is when a state have at least	*/
{													/* two different next states by different input values	*/
	prioritytype mean_priority;					/* save all mean priorities in a map<string,vector<string>> */
	vector<string> candidates;								/* candidates for mean priority					*/
	int candidate_count = 0;								/*												*/
															/*												*/
	for (int j = 0; j < table.iheight; j++) {				/* passed through row for row of mean priority	*/
		for (int i = 0; i < table.iwidth; i++) {			/*												*/
			if ((table.table[table.istates.at(j)].at(i).next_state) != (table.istates.at(j))) {	/*			*/
							/*checks, if a state is unequal of his next_states (independet of input value)	*/
				if (!((find(candidates.begin(), candidates.end(), table.table[table.istates.at(j)].at(i).next_state) != candidates.end())) && (table.table[table.istates.at(j).c_str()].at(i).next_state.size() > 0)) {
					/*checks, if the next state of a state isn't in the candidate vector AND a next state is anyway existing */
					candidate_count++;						/* count candidates								*/
					candidates.push_back(table.table[table.istates.at(j).c_str()].at(i).next_state);/*save	*/
				}											/* a candidate for mean priority into candidates*/
			}												/*												*/
		}													/*												*/
		if (candidate_count >= 2) {							/* condition for mean priority					*/
			mean_priority[table.istates.at(j).c_str()] = candidates;/*if there are more candidates than one,*/
		}												/* then the mean priority map is passed candidates	*/
		candidates.clear();									/* clear candidates								*/
		candidate_count = 0;								/* reset candidate_count						*/
	}														/*												*/
	return mean_priority;									/*												*/
}															/*												*/
															/*----------------------------------------------*/
/*!
* \brief Skip documentation
*
* Get the low priorities of the state diagramm and return them
*
* \param[in] none
* \param[out] none
* \return lowpriotype lowPriority
* \note Global variables used: class smtable table: iwidth, iheight, istates and tabletype table
*/
lowpriotype lowPriority(smtable &table)	/*lowest priority is given when at least two states have the same output behaviour*/
{															/*												*/
	vector < vector<string>> low_priority;					/*save all low priorities in a vector<vector<string>>*/
	vector<string> candidates;								/* candidates for low priority					*/
	int candidate_count = 0;								/*												*/
	bool output_matched = false;							/* candidates for low priority					*/
	for (int j = 0; j < table.iwidth; j++) {				/* check all input possibilities				*/
		for (int k = 0; k < (1 << table.ioutputs.size()); k++) {/*check all output possibilities (00 01 10 11 -> 0 1 2 3), (1 << table.ioutputs.size()) is the number of output possibilities*/
			for (int i = 0; i < table.iheight; i++) {		/*												*/
				output_matched = table.bitsMatch(k, table.table[table.istates.at(i)].at(j).out_list.c_str());/*checks, if int k matches with output bit pattern and set bool output_matched */
				if (output_matched == true) {				/*												*/
					candidate_count++;						/* count candidates								*/
					candidates.push_back(table.istates.at(i).c_str());/*push a candidate for low priority into candidates*/
					output_matched = false;					/* reset bool output_matched					*/
				}											/*												*/
			}												/*												*/
			if (candidate_count >= 2) {						/* condition for low priority					*/
				low_priority.push_back(candidates);			/*if there are more candidates than one, then the low priority vector<vector<<string>>> is passed candidates*/
			}												/*												*/
			candidates.clear();								/* clear candidates								*/
			candidate_count = 0;							/* reset candidate_count						*/
		}													/*												*/
	}														/*												*/
	return low_priority;									/*												*/
}															/*												*/
															/*----------------------------------------------*/
/*!
* \brief Skip documentation
*
* Get the optimize state coding of the state diagramm and return them
* Set first the high priorities, then the mean priorities, then the low priorities and at last left states in Zustandscodierung
*
* \param[in] prioritytype high_priority, prioritytype mean_priority, lowpriotype low_priority
* \param[out] none
* \return elementlist Zustandscodierung
* \note Global variables used: class smtable table: istates
*/
smtable::elementlist optimize(prioritytype high_priority, prioritytype mean_priority, lowpriotype low_priority, smtable &table)
{															/*												*/
	uint set_states = 0;									/*												*/
	int Zustandscodierung_size = 1;							/*												*/
	uint high_priority_count = high_priority.size();		/*												*/
	uint mean_priority_count = mean_priority.size();		/*												*/
	uint low_priority_count = low_priority.size();			/*												*/
	bool is_set = false;									/*												*/
	int candidate_count = 0;								/*												*/
	prioritytype::iterator t3;								/*												*/
	vector <string> Zustandscodierung, candidates;			/*												*/
															/*												*/
	while (Zustandscodierung_size < table.iheight) {		/*												*/
		Zustandscodierung_size *= 2;				/* Zustandscodierung_size is the next power of 2 (2^n)	*/
	}														/*												*/
															/*												*/
															/*												*/
	Zustandscodierung.resize(Zustandscodierung_size);		/*												*/
	t3 = high_priority.begin();								/*												*/
															/*												*/
															/*												*/
															/*				high priorities					*/
	if (set_states < table.istates.size()) {	/* ckecks, if a state isn't set in the Zustandscodierung	*/
		for (uint i = 0; i < high_priority_count; i++) {	/*												*/
			if (!((find(Zustandscodierung.begin(), Zustandscodierung.end(), t3->first) != Zustandscodierung.end()))) {
									  /* ckecks, if a high priority state isn't set in the Zustandscodierung*/
				Zustandscodierung[set_states] = t3->first;	/* sets a high priority state					*/
				set_states++;								/*												*/
				is_set = true;								/* sets a bool bit								*/
			}												/*												*/
			t3++;											/* increase t3 iterator							*/
															/*												*/
			while (is_set) {								/* run through till bool bit isn't set anymore	*/
				if (high_priority[high_priority.at(Zustandscodierung[set_states - 1]).at(0)].size() > 0) {
					/* checks, if there is another high priority of the first state of the high priority state*/
					Zustandscodierung[set_states] = high_priority[Zustandscodierung[set_states - 1]].at(1);
					Zustandscodierung[set_states + 1] = high_priority[Zustandscodierung[set_states - 1]].at(0);
					/* set the first state of the high priority state at second (the next high priority can be set directly afterwards */
					set_states += 2;						/* e.g. S0 S7 S9 + S7 S3 S4 -> S0 S9 S7 S3 S4	*/
				}											/*												*/
				else {										/*												*/
					Zustandscodierung[set_states] = high_priority[Zustandscodierung[set_states - 1]].at(0);
					Zustandscodierung[set_states + 1] = high_priority[Zustandscodierung[set_states - 1]].at(1);
					set_states += 2;						/*												*/
				}											/*												*/
				if (high_priority[Zustandscodierung[set_states - 1]].size() > 0) {
					/* checks, if there is a high priority state of the last set state						*/
					is_set = true;							/*												*/
				}											/*												*/
				else {										/*												*/
					is_set = false;							/*												*/
				}											/*												*/
			}												/*												*/
		}													/*												*/
	}														/*												*/
															/*												*/
	t3 = mean_priority.begin();								/*												*/
	is_set = false;											/*												*/
															/*												*/
															/*				mean priority					*/
	if (set_states < table.istates.size()) {	/* ckecks, if a state isn't set in the Zustandscodierung	*/
		for (uint j = 0; j < mean_priority_count; j++) {	/*												*/
			for (uint i = 0; i < t3->second.size(); i++) {	/*												*/
				if (find(Zustandscodierung.begin(), Zustandscodierung.end(), t3->second.at(i)) != Zustandscodierung.end()) {
					/* ckecks, if a state of a mean priority is set in the Zustandscodierung				*/
					is_set = true;							/*												*/
				}											/*												*/
			}												/*												*/
			if (is_set == false) {							/*												*/
				if (!(find(Zustandscodierung.begin(), Zustandscodierung.end(), t3->first) != Zustandscodierung.end())) {
					/* ckecks, if a mean priority state isn't set in the Zustandscodierung						*/
					Zustandscodierung[set_states] = t3->first;/*set mean priority state in the Zustandscodierung*/
					set_states++;							/*												*/
				}											/*												*/
				for (uint p = 0; p < t3->second.size(); p++) {/*											*/
					Zustandscodierung[set_states] = t3->second.at(p);/*set states of mean priority in the Zustandscodierung*/
					set_states++;							/*												*/
				}											/*												*/
			}												/*												*/
			is_set = false;									/*												*/
			t3++;											/* increase t3 iterator							*/
		}													/*												*/
	}														/*												*/
															/*												*/
															/*					low priority				*/		
	if (set_states < table.istates.size())			 /*ckecks, if a state isn't set in the Zustandscodierung*/
	{														/*												*/
		for (uint j = 0; j < low_priority_count; j++) {		/*												*/
			for (uint i = 0; i < low_priority[j].size(); i++) {/*											*/
				if (!(find(Zustandscodierung.begin(), Zustandscodierung.end(), low_priority[j].at(i)) != Zustandscodierung.end())) {
					/* ckecks, if a state of a low priority isn't set in the Zustandscodierung				*/
					candidate_count++;						/* count candidates								*/
					candidates.push_back(low_priority[j].at(i));/* push a candidate for low priority		*/
				}											/*		(in Zustandscodierung) into candidates	*/
			}												/*												*/
			if (candidate_count >= 2) {						/*if there are more candidates than one, then the candidates is set in Zustandscodierung*/
				for (uint k = 0; k < candidates.size(); k++) {	/*											*/
					Zustandscodierung[set_states] = candidates.at(k);/*										*/
					set_states++;							/*												*/
				}											/*												*/
			}												/*												*/
			candidates.clear();								/* clear candidates								*/
			candidate_count = 0;							/* reset candidate_count						*/
		}													/*												*/
	}														/*												*/
															/*Set left states*/								/*												*/
	if (set_states < table.istates.size()) {				/*ckecks, if a state isn't set in the Zustandscodierung*/
		for (uint i = 0; i < table.istates.size(); i++) {	/*												*/
			if (!(find(Zustandscodierung.begin(), Zustandscodierung.end(), table.istates.at(i)) != Zustandscodierung.end())) {
				/* ckecks, if a state isn't set in the Zustandscodierung									*/
				Zustandscodierung[set_states] = table.istates.at(i);/*set a state in the Zustandscodierung	*/
				set_states++;								/*												*/
			}												/*												*/
		}													/*												*/
	}														/*												*/
	return Zustandscodierung;								/*												*/
}															/*												*/
															/*----------------------------------------------*/
/*!
* \brief Skip documentation
*
* Ignores everything until "Begin".
*
* \param[in] tok Current token to check for "Begin"
* \param[out] none
* \return Next state to enter
* \note Global variables used: none
*/
bool contains(smtable::elementlist base, smtable::elementlist cmp)/*								*/
{															/* check, if base contains all entries of cmp	*/
	bool retval = true;										/* preset retval								*/
	for (uint i = 0; i < cmp.size(); i++) {					/* run through cmp								*/
		smtable::elementlist::iterator it;					/* declare iterator for search					*/
		if ((it = find(base.begin(), base.end(), cmp.at(i))) == base.end()) {/* if entry not found in base	*/
			retval = false;									/* cmp can't be subset of base					*/
		}													/* 												*/
	}														/* 												*/
	return retval;											/* 												*/
}															/* 												*/
															/*----------------------------------------------*/
															/*!
															* \brief Skip documentation
															*
															* Ignores everything until "Begin".
															*
															* \param[in] tok Current token to check for "Begin"
															* \param[out] none
															* \return Next state to enter
															* \note Global variables used: none
															*/
void removeSubsets(lowpriotype &tab) {						/*												*/
	for (uint i = 0; i < tab.size(); i++) {					/* run through list								*/
		for (uint j = 0; (j < tab.size()) && (i < tab.size()); j++) {/* run through list again				*/
			if ((contains(tab.at(i), tab.at(j))) && (i != j)) {/* if j is subset of i and doesnt compar self*/
				vector<vector<string>>::iterator it = tab.begin() + j;/* declare pointer to subset for erase*/
				tab.erase(it);								/* erase										*/
				j--;										/* adjust k, because current element at k is	*/
			}												/*							not checked, yet	*/
		}													/* 												*/
	}														/* 												*/
}															/* 												*/
															/*----------------------------------------------*/
/*!
* \brief Skip documentation
*
* Ignores everything until "Begin".
*
* \param[in] tok Current token to check for "Begin"
* \param[out] none
* \return Next state to enter
* \note Global variables used: none
*/
funcreturn writeOutputFile(smtable &table)			/* 												*/
{															/* 												*/
	funcreturn retval = F_SUCCESS;					/* preset return								*/
	int stateCodeBitCount = 1;								/* number of bits needed to code states			*/
	for (; (1 << stateCodeBitCount) < table.istates.size(); stateCodeBitCount++);/* calculation				*/
	ofstream outfile;										/* output file object							*/
	try {													/* safe code block, if write exception occours	*/
		outfile.open("ZMnichtoptimiert.tbl");				/* 												*/
		outfile << "table ZMnichtoptimiert\n  input ";		/* write file header							*/
		for (uint i = 0; i < table.iinputs.size(); i++) {	/* print actual input names						*/
			outfile << table.iinputs.at(i).c_str() << " ";	/* 												*/
		}													/* 												*/
		for (int i = 0; i < stateCodeBitCount; i++) {		/* print names for current state coding			*/
			char statetemp[4] = { 'C', '0', '0', '\0' };	/* 2^100 = 10^30 States, should be enough		*/
			statetemp[1] = i / 10 + '0';					/* 												*/
			statetemp[2] = i + '0';							/* 												*/
			outfile << statetemp << " ";					/* 												*/
		}													/* 												*/
		outfile << "\n  output ";							/* 												*/
		for (int i = 0; i < stateCodeBitCount; i++) {		/* print names for next state coding			*/
			char statetemp[4] = { 'D', '0', '0', '\0' };	/* 												*/
			statetemp[1] = i / 10 + '0';					/* 												*/
			statetemp[2] = i + '0';							/* 												*/
			outfile << statetemp << " ";					/* 												*/
		}													/* 												*/
		outfile << "\n\" nonoptimized transitiontable\n";	/* 												*/
		for (int i = 0; i < (1 << table.iinputs.size()); i++) {/* run through input combinations			*/
			for (int j = 0; j < (1 << stateCodeBitCount); j++) {/* run through state combinations			*/
				int nextstate = 0;							/* 												*/
				if (j < table.istates.size()) {				/* 												*/
					for (; (nextstate < table.istates.size()) && (table.istates.at(nextstate) != table.table[table.istates.at(j)].at(i).next_state); nextstate++);
					outfile << "  " << *table.int2bit(i, table.iinputs.size()) << *table.int2bit(j, stateCodeBitCount) << " | ";
					if (nextstate < table.istates.size()) {	/* 												*/
						outfile << *table.int2bit(nextstate, stateCodeBitCount);/* 							*/
					}										/* 												*/
					else {									/* 												*/
						for (int k = 0; k < stateCodeBitCount; k++) {/* 									*/
							outfile << "-";					/* 												*/
						}									/* 												*/
					}										/* 												*/
					outfile << "\n";						/* 												*/
				}											/* 												*/
				else {										/* 												*/
					outfile << "  " << *table.int2bit(i, table.iinputs.size()) << *table.int2bit(j, stateCodeBitCount) << " | ";
					for (int k = 0; k < stateCodeBitCount; k++) {/* 										*/
						outfile << "-";						/* 												*/
					}										/* 												*/
					outfile << "\n";						/* 												*/
				}											/* 												*/
			}												/* 												*/
		}													/* 												*/
		outfile << "end\n";									/* 												*/
		outfile.close();									/* 												*/
	}														/* 												*/
	catch (...) {											/* 												*/
		retval = F_FAIL;									/* 												*/
	}														/* 												*/
	return retval;											/* 												*/
}															/* 												*/
															/*----------------------------------------------*/
/*!
* \brief Skip documentation
*
* Ignores everything until "Begin".
*
* \param[in] tok Current token to check for "Begin"
* \param[out] none
* \return Next state to enter
* \note Global variables used: none
*/
funcreturn writeOutputFile(smtable::elementlist statelist, smtable &table)/*								*/
{															/*												*/
	funcreturn retval = F_SUCCESS;							/*												*/
	int stateCodeBitCount = 1;								/*												*/
	for (; (1 << stateCodeBitCount) < table.istates.size(); stateCodeBitCount++); /* calculate bits needed	*/
	ofstream outfile;										/*												*/
	try {													/*												*/
		outfile.open("ZMoptimiert.tbl");					/*												*/
		outfile << "table ZMoptimiert\n  input ";			/*												*/
		for (uint i = 0; i < table.iinputs.size(); i++) {	/* print actual input names						*/
			outfile << table.iinputs.at(i).c_str() << " ";	/*												*/
		}													/*												*/
		for (uint i = 0; i < stateCodeBitCount; i++) {		/* print names for current state coding			*/
			char statetemp[4] = { 'C', '0', '0', '\0' };	/* 2^100 = 10^30 States							*/
			statetemp[1] = i / 10 + '0';					/*												*/
			statetemp[2] = i + '0';							/*												*/
			outfile << statetemp << " ";					/*												*/
		}													/*												*/
		outfile << "\n  output ";							/*												*/
		for (uint i = 0; i < stateCodeBitCount; i++) {		/* print names for next state coding			*/
			char statetemp[4] = { 'D', '0', '0', '\0' };	/*												*/
			statetemp[1] = i / 10 + '0';					/*												*/
			statetemp[2] = i + '0';							/*												*/
			outfile << statetemp << " ";					/*												*/
		}													/*												*/
		outfile << "\n\" nonoptimized transitiontable\n";	/*												*/
		for (int i = 0; i < (1 << table.iinputs.size()); i++) {/* run through input combinations			*/
			for (int j = 0; j < (1 << stateCodeBitCount); j++) {/* run through state combinations			*/
				int nextstate = 0;							/*												*/
				if (statelist.at(j).size() != 0) {			/* if entry not empty get number of next state	*/
					for (; (nextstate < table.istates.size()) && (statelist.at(nextstate) != table.table[statelist.at(j)].at(i).next_state); nextstate++);
					outfile << "  " << *table.int2bit(i, table.iinputs.size()) << *table.int2bit(int2gray(j), stateCodeBitCount) << " | ";
					if (nextstate < table.istates.size()) {	/* if next_state found print it					*/
						outfile << *table.int2bit(int2gray(nextstate), stateCodeBitCount);/*				*/
					}										/*												*/
					else {									/* if no next_state found						*/
						for (int k = 0; k < stateCodeBitCount; k++) {/* fill with don't cares				*/
							outfile << "-";					/*												*/
						}									/*												*/
					}										/*												*/
					outfile << "\n";						/*												*/
				}											/*												*/
				else {										/* if entry empty								*/
					outfile << "  " << *table.int2bit(i, table.iinputs.size()) << *table.int2bit(int2gray(j), stateCodeBitCount) << " | ";
					for (int k = 0; k < stateCodeBitCount; k++) {	/* fill with don't cares				*/
						outfile << "-";						/*												*/
					}										/*												*/
					outfile << "\n";						/*												*/
				}											/*												*/
			}												/*												*/
		}													/*												*/
		outfile << "end\n";									/*												*/
		outfile.close();									/*												*/
	}														/*												*/
	catch (...) {											/*												*/
		retval = F_FAIL;									/*												*/
	}														/*												*/
	return retval;											/*												*/
}															/*												*/
															/*----------------------------------------------*/
/*!
* \brief Skip documentation
*
* Ignores everything until "Begin".
*
* \param[in] tok Current token to check for "Begin"
* \param[out] none
* \return Next state to enter
* \note Global variables used: none
*/
int int2gray(int input)										/* returns integer, which bit structure is gray	*/
{															/* coded										*/
	return input ^ (input >> 1);							/*												*/
}															/*----------------------------------------------*/
