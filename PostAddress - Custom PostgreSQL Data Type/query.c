// query.c ... run queries
// part of Multi-attribute linear-hashed files
// Ask a query on a named relation
// Usage:  ./query  [-v]  'a1,a3,..'  from  RelName where 'v1,v2,v3,v4,...'
// - a1,a3,... can be '*' to indicate all attributes
// - Any vi can be '?' to indicate an unknown value
// - Any vi can contain '%' as a wildcard matching zero or more characters
// Credit: John Shepherd
// Last modified by Xiangjun Zai, Mar 2025

#include "defs.h"
#include "select.h"
#include "project.h"
#include "tuple.h"
#include "reln.h"
#include "chvec.h"

#define USAGE "./query  [-v]  a1,a3,..(*)  from  RelName  where  v1,v2,v3,v4,..."

// Main ... process args, run query

int main(int argc, char **argv)
{
	Reln r;  // handle on the open relation
	Selection s;  // handle on the selection
	Projection p;  // handle on the projection
	Tuple t;  // tuple pointer
	char err[MAXERRMSG];  // buffer for error messages
	int offset = 0; // adapt offset for -v
	int verbose = 0;  // show extra info on query progress
	char *rname;  // name of table/file
	char *valstr;   // a query string of values for selection
	char *attrstr;   // string of 1-based attribute indexes used for projection

	// process command-line args
	if (argc < 6 || argc > 7) fatal(USAGE);
	if (strcmp(argv[1], "-v") == 0) {
		if (argc != 7) fatal(USAGE);
		offset = 1;  verbose = 1;
	}
	if (strcmp(argv[offset+2], "from") != 0 || strcmp(argv[offset+4], "where") != 0) {
        fatal(USAGE);
    }
	attrstr = argv[offset+1];  rname = argv[offset+3];  valstr = argv[offset+5];
	if (verbose) { /* keeps compiler quiet */ }

	// initialise relation, scanning, projection structure

	if (!existsRelation(rname)) {
		sprintf(err, "No such relation: %s",rname);
		fatal(err);
	}
	if ((r = openRelation(rname,"r")) == NULL) {
		sprintf(err, "Can't open relation: %s",rname);
		fatal(err);
	}
	if ((s = startSelection(r, valstr)) == NULL) {	
		sprintf(err, "Invalid selection: %s",valstr);
		fatal(err);
	}
	if ((p = startProjection(r, attrstr)) == NULL) {	
		sprintf(err, "Invalid projection: %s",attrstr);
		fatal(err);
	}

	// execute the query (find matching tuples and project on specified attributes)

	char tup[MAXTUPLEN];
	while ((t = getNextTuple(s)) != NULL) {
		projectTuple(p,t,tup);
		//-------------------------------------------------------------------
		printf("%s\n",tup);

	}

	// clean up
	closeProjection(p);
	closeSelection(s);
	closeRelation(r);

	return 0;
}

