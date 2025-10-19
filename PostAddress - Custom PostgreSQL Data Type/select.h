// select.h ... interface to query scan functions
// part of Multi-attribute Linear-hashed Files
// See select.c for details of Selection type and functions
// Credit: John Shepherd
// Last modified by Xiangjun Zai, Mar 2025

#ifndef SELECTION_H
#define SELECTION_H 1

typedef struct SelectionRep *Selection;

#include "reln.h"
#include "tuple.h"

Selection startSelection(Reln, char *);
Tuple getNextTuple(Selection);
void closeSelection(Selection);

#endif
