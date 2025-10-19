// project.c ... project scan functions
// part of Multi-attribute Linear-hashed Files
// Manage creating and using Projection objects
// Last modified by Xiangjun Zai, Mar 2025
// 2025.5
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "defs.h"
#include "project.h"
#include "reln.h"
#include "tuple.h"
#include "util.h"

// A suggestion ... you can change however you like

struct ProjectionRep {
	Reln rel;       // need to remember Relation info
	int attrCount;
    int attrIndexes[MAXATTRS];
    //TODO
};

// take a string of 1-based attribute indexes (e.g. "1,3,4")
// set up a ProjectionRep object for the Projection
Projection startProjection(Reln r, char *attrstr)
{
 // ------------------------------------
    // if (attrstr == NULL) {
    //     fprintf(stderr, "DEBUG: attrstr == NULL!!\n");
    // }
    if (strcmp(attrstr, "*") == 0) {
        // if it's "*", want all 
        Projection new = malloc(sizeof(struct ProjectionRep));

        assert(new != NULL);
        new->rel = r;
        new->attrCount = nattrs(r);
        for (int i = 0; i < new->attrCount; i++) {
            new->attrIndexes[i] = i;
        }
        return new;
    }
       // ------------------------------------
    Projection new = malloc(sizeof(struct ProjectionRep));
    assert(new != NULL);

    // ------------------------------------
    // printf("DEBUG: startProjection() malloc new = %p\n", (void *)new);
    // ------------------------------------
    new->rel = r;
    new->attrCount = 0;


    char tmpAttrStr[MAXATTRS * 10];
    memset(tmpAttrStr, 0, sizeof(tmpAttrStr));  
    strncpy(tmpAttrStr, attrstr, sizeof(tmpAttrStr) - 1);  // safe copy
    char *token = strtok(tmpAttrStr, ",");

    // char tmpAttrStr[MAXATTRS*10];
    // strcpy(tmpAttrStr, attrstr);

    // char *token = strtok(tmpAttrStr,",");
    
    while (token != NULL){
        int attr = atoi(token);
        if (attr < 1 || attr > nattrs(r)){
            // ------------------------------------
            // fprintf(stderr,"Invalid attribute index: %d\n", attr);
            // -----------------------------------------
            free(new);
            return NULL;
        }
        new->attrIndexes[new->attrCount++] = attr - 1;
        token = strtok(NULL,",");
    }
    //TODO

    return new;
}

void projectTuple(Projection p, Tuple t, char *buf)
{
    // TODO: Implement projection of tuple 't' according to 'p' and store result in 'buf'
    // -----------------------------------------
    // printf("DEBUG: projectTuple() received p = %p\n", (void *)p);
    // -----------------------------------------
    if (p == NULL) {  // prevent NULL
        tupleString(t, buf);
        return;
    }

    char *val[MAXATTRS];
    tupleVals(t, val);
    buf[0] = '\0';
    for ( int i = 0; i < p->attrCount; i++){
        strcat(buf,val[p->attrIndexes[i]]);
        if ( i < p->attrCount - 1)
            strcat(buf,",");
    }
      // -----------------------------------------
    // freeVals(val, nattrs(p->rel));
}

void closeProjection(Projection p)
{
    // TODO
    // -----------------------------------------
    // printf("DEBUG: closeProjection() received p = %p\n", (void *)p);
    // -----------------------------------------
    if ( p != NULL){
        free(p);
    }
}


