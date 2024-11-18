#ifndef SPLITTER_H
#define SPLITTER_H


#include "hashtable.h"

void splitter(int splitterIndex, int numOfSplitters, int numOfBuilders, char *inputFile, int inputFileLines, int pipes[numOfSplitters][numOfBuilders][2]);

#endif