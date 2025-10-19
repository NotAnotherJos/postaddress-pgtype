// project.h ... interface to query scan functions
// part of Multi-attribute Linear-hashed Files
// See project.c for details of Projection type and functions
// Last modified by Xiangjun Zai, Mar 2025

#ifndef PROJECTION_H
#define PROJECTION_H 1

typedef struct ProjectionRep *Projection;

#include "reln.h"
#include "tuple.h"

Projection startProjection(Reln r, char *attrstr);
void projectTuple(Projection p, Tuple t, char *buf);
void closeProjection(Projection p);

#endif
