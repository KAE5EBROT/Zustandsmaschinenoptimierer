#pragma once
#include <stdio.h>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <time.h>
#include ".\Zustandsmaschinentabelle\smtable.h"

typedef map<string, vector<string>> prioritytype;
typedef vector < vector<string>> lowpriotype;

enum funcreturn {
	F_SUCCESS = 0, F_FAIL
};

prioritytype highPriority(smtable &table);
prioritytype meanPriority(smtable &table);
lowpriotype lowPriority(smtable &table);
smtable::elementlist optimize(prioritytype high_priority, prioritytype mean_priority, lowpriotype low_priority, smtable &table);
bool contains(smtable::elementlist base, smtable::elementlist cmp);
void removeSubsets(lowpriotype &tab);
funcreturn writeOutputFile(smtable &table);
funcreturn writeOutputFile(smtable::elementlist statelist, smtable &table);
int int2gray(int input);

