// select.c ... select scan functions
// part of Multi-attribute Linear-hashed Files
// Manage creating and using Selection objects
// Credit: John Shepherd
// Last modified by Xiangjun Zai, Mar 2025
// 2025.
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include "defs.h"
#include "select.h"
#include "reln.h"
#include "tuple.h"
#include "bits.h"
#include "hash.h"

#include "page.h"
#include "reln.h"
#include <unistd.h> // debug only, for sleep


// A suggestion ... you can change however you like

struct SelectionRep {
	Reln    rel;       // need to remember Relation info
    Tuple   query;     //
	Bits    known;     // the hash value from MAH
	Bits    unknown;   // the unknown bits from MAH
	Page    curpage;   // current page in scan
	int     is_ovflow; // are we in the overflow pages?
	PageID  curpageID;
    PageID  curovflowID;  // current overflow page in scan
    Offset  curtupOffset;    // offset of current tuple within page
	//TODO
};

// take a query string (e.g. "1234,?,abc,?")
// set up a SelectionRep object for the scan
static bool matchField(char *qval, char *tval) {
    if (strcmp(qval, "?") == 0) return true;
    if (strchr(qval, '%') != NULL) return strstr(tval, qval + 1) != NULL;
    return strcmp(qval, tval) == 0;
}

// Check if a tuple matches the query
static bool matchTuple(Reln r, Tuple q, Tuple t) {
    int na = nattrs(r);
    char *qv[MAXATTRS];
    char *tv[MAXATTRS];
    tupleVals(q, qv);
    tupleVals(t, tv);
    bool flag = true;

    for (int i = 0; i < na; i++) {
        if (!matchField(qv[i], tv[i])) {
            // freeVals(qv, na);
            // freeVals(tv, na);
            // return false;
            flag = false;
        }
    }
    // freeVals(qv, na);
    // freeVals(tv, na);
    return flag;
}
// start a selection scan
Selection startSelection(Reln r, char *q)
{
    Selection new = malloc(sizeof(struct SelectionRep));
    assert(new != NULL);
    // TODO
	// Partial algorithm:
	// form known bits from known attributes
	// form unknown bits from '?' and '%' attributes
	// compute PageID of first page
	//   using known bits and first "unknown" value
	// set all values in SelectionRep object
    // Parse the query string
    new->query = malloc(strlen(q) + 1);
    assert(new->query != NULL);
    strcpy(new->query, q);
    new->rel = r;

    // Initialize known and unknown bits
    new->known = tupleHash(r, q);
    new->unknown = 0;
    
    // Compute known and unknown bits
    ChVecItem *chevec_item = chvec(r);
    char *delimeter = strtok(q, ",");
    struct _word
	{
		char *buf;
	};
    Count num_attr = nattrs(r);
	struct _word words[num_attr];
    int i = 0;
	while (delimeter != NULL)
	{
		words[i++].buf = delimeter;
		delimeter = strtok(NULL, ",");
	}
	new->known = tupleHash(r, new->query);

	for (int j = 0; j < MAXCHVEC; j++)
	{

		Byte att = chevec_item[j].att;

		if (strcmp(words[att].buf, "?") == 0)
		{
			new->known = unsetBit(new->known, j);
			new->unknown = setBit(new->unknown, j);
		}
	}


    // Compute the first page ID
    // Using the known bits and the first unknown value
    new->curpageID = getLower((new->known & ~new->unknown), depth(r));
    if (new->curpageID < splitp(r)) {
        new->curpageID = getLower((new->known & ~new->unknown), depth(r) + 1);
    }

    // debug
    // char knowBuf[40], unknownBuf[40];
    // bitsString(new->known, knowBuf);
    // bitsString(new->unknown, unknownBuf);
    // printf("Selection: known %s, unknown %s, pageID %u\n", knowBuf, unknownBuf, new->curpageID);
    

    // Initialize other fields
    new->is_ovflow = 0;
    new->curovflowID = NO_PAGE;
    new->curtupOffset = 0;
    new->curpage = getPage(dataFile(r), new->curpageID);
    if (new->curpage == NULL) {
        free(new->query);
        free(new);
        return NULL;
    }
    return new;
}

// get next tuple during a scan

Tuple getNextTuple(Selection q)
{
    // TODO
	// Partial algorithm:
    // if (more tuples in current page)
    //    get next matching tuple from current page
    // else if (current page has overflow)
    //    move to overflow page
    //    grab first matching tuple from page
    // else
    //    move to "next" bucket
    //    grab first matching tuple from data page
    // endif
    // if (current page has no matching tuples)
    //    go to next page (try again)
    // endif
    if (q == NULL || q->curpage == NULL)
        return NULL;

    while (1) {
        // Get current page info
        Count ntuples = pageNTuples(q->curpage);
        char *data = pageData(q->curpage);
        
        // Skip to current offset in the page
        for (int i = 0; i < q->curtupOffset; i++) {
            data += strlen(data) + 1;
        }

        // printf("Current page(cur%u/cur_ov%u:is_ov?%d) has %d tuples, scanning...\n", q->curpageID, q->curovflowID, q->is_ovflow, ntuples);
        
        // Scan remaining tuples in current page
        while (q->curtupOffset < ntuples) {
            if (matchTuple(q->rel, q->query, data)) {
                // Found a matching tuple
                Tuple t = copyString(data);
                q->curtupOffset++; // Move to next tuple for next call
                return t;
            }
            
            // Move to next tuple
            data += strlen(data) + 1;
            q->curtupOffset++;
        }
        
        // Case 1: Current page has overflow
        if (q->is_ovflow == 0 && pageOvflow(q->curpage) != NO_PAGE) {
            // printf("Current page has overflow, moving to overflow page...\n");
            PageID ovp = pageOvflow(q->curpage);
            free(q->curpage);
            
            q->is_ovflow = 1;
            q->curovflowID = ovp;
            q->curpage = getPage(ovflowFile(q->rel), q->curovflowID);
            q->curtupOffset = 0;
        }
        // Case 2: In overflow chain and current page has next overflow
        else if (q->is_ovflow == 1 && pageOvflow(q->curpage) != NO_PAGE) {
            // printf("In overflow chain, moving to next overflow page...\n");
            PageID ovp = pageOvflow(q->curpage);
            free(q->curpage);
            
            q->curovflowID = ovp;
            q->curpage = getPage(ovflowFile(q->rel), ovp);
            q->curtupOffset = 0;
        }
        // Case 3: In overflow chain but no more overflow pages
        else if (q->is_ovflow == 1 && pageOvflow(q->curpage) == NO_PAGE) {
            // printf("In overflow chain, but no more overflow pages...\n");
            free(q->curpage);
            q->is_ovflow = 0;
            q->curtupOffset = 0;
            q->curovflowID = 0;
            q->curpage = getPage(dataFile(q->rel), q->curpageID);
            q->curpageID++;
        }
        // Case 3: Move to next bucket (data page)
        else if (q->curpageID < npages(q->rel) - 1) {
            // sleep(1);
            // printf("Moving to next bucket (data page)...cur: d%u, next: %u, total: %d\n", q->curpageID, q->curpageID + 1, npages(q->rel));
            free(q->curpage);
            
            q->curpageID++;
            q->is_ovflow = 0;
            q->curpage = getPage(dataFile(q->rel), q->curpageID);
            q->curtupOffset = 0;
        }
        // Case 4: No more pages to scan
        else {
            // printf("No more pages to scan, ending selection...\n");
            free(q->curpage);
            q->curpage = NULL;
            return NULL;
        }
    }
}

// clean up a SelectionRep object and associated data

void closeSelection(Selection q)
{
    // TODO
    if (q != NULL) {
        if (q->query) free(q->query);
        // if (q->curpage) free(q->curpage);
        free(q);
    }

}


