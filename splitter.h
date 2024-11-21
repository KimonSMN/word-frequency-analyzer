#ifndef SPLITTER_H
#define SPLITTER_H


#include "hashtable.h"

void splitter(int splitterIndex, int numOfSplitters, int numOfBuilders, char *inputFile, int inputFileLines, int builderPipes[numOfBuilders][2], int exclusionListSize ,char *exclusionList[]);
void trim_newline(char *str);
void clean_text(char *str);
#endif