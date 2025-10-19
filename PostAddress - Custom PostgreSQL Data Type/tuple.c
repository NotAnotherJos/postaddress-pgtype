// tuple.c ... functions on tuples
// part of Multi-attribute Linear-hashed Files
// Credt: John Shepherd
// Last modified by Xiangjun Zai, 24/03/2024
// 2025.5

#include "defs.h"
#include "tuple.h"
#include "reln.h"
#include "hash.h"
#include "chvec.h"
#include "bits.h"
#include "util.h"

// return number of bytes/chars in a tuple

int tupLength(Tuple t)
{
	return strlen(t);
}

// reads/parses next tuple in input

Tuple readTuple(Reln r, FILE *in)
{
	char line[MAXTUPLEN];
	if (fgets(line, MAXTUPLEN-1, in) == NULL)
		return NULL;
	line[strlen(line)-1] = '\0';
	// count fields
	// cheap'n'nasty parsing
	char *c; 
	int nf = 1;
	for (c = line; *c != '\0'; c++)
		if (*c == ',') 
			nf++;
	// invalid tuple
	if (nf != nattrs(r)) 
		return NULL;
	return copyString(line); // needs to be free'd sometime
}

// extract values into an array of strings

void tupleVals(Tuple t, char **vals)
{
	char *c = t, *c0 = t;
	int i = 0;
	for (;;) 
	{
		while (*c != ',' && *c != '\0') 
			c++;
		if (*c == '\0') {
			// end of tuple; add last field to vals
			vals[i++] = copyString(c0);
			break;
		}
		else {
			// end of next field; add to vals
			*c = '\0';
			vals[i++] = copyString(c0);
			*c = ',';
			c++; 
			c0 = c;
		}
	}
}

// release memory used for separate attribute values

void freeVals(char **vals, int nattrs)
{
	int i;
    // release memory used for each attribute
	for (i = 0; i < nattrs; i++) 
		free(vals[i]);
    // release memory used for pointer array
    free(vals);
}

// hash a tuple using the choice vector
// TODO: actually use the choice vector to make the hash

Bits tupleHash(Reln r, Tuple t)
{
	// char buf[MAXBITS+5];  //*** for debug
	Count nvals = nattrs(r);
    char **vals = malloc(nvals*sizeof(char *));
    assert(vals != NULL);
    tupleVals(t, vals);
	Bits hashes[nvals];
	Bits hash = 0;
	struct _hash
	{
		char buf[MAXBITS + 5];
	};
	struct _hash buffes[nvals];
	// Bits hash = hash_any((unsigned char *)vals[0],strlen(vals[0]));
	// bitsString(hash,buf);  //*** for debug
	// printf("hash(%s) = %s\n", vals[0], buf);  //*** for debug
    for (int i = 0; i < nvals; i++)
	{
		hashes[i] = hash_any((unsigned char *)vals[i],strlen(vals[i]));
		bitsString(hashes[i],buffes[i].buf);
	}
	ChVecItem *chevec_item = chvec(r);
	for (int i = 0; i < MAXCHVEC; i++)
	{

		Byte att = chevec_item[i].att;
		Bits bit = chevec_item[i].bit;

		if (bitIsSet(hashes[att], bit))
		{

			hash = setBit(hash, i);
		}
	}
	char bitstring[MAXBITS+1];
	bitsString(hash, bitstring);
	printf("hash(%s) = %s\n", t, bitstring);

	freeVals(vals,nvals);
	// free(vals);
	return hash;
}

// compare two tuples (allowing for "unknown" values)
// TODO: actually compare values
Bool tupleMatch(Reln r, Tuple pt, Tuple t)
{
	Count na = nattrs(r);
	char **ptv = malloc(na*sizeof(char *));
	tupleVals(pt, ptv);
	char **v = malloc(na*sizeof(char *));
	tupleVals(t, v);
	Bool match = TRUE;
	// TODO: actually compare values
	int i;
	for (i = 0; i < na; i++)
	{
		if (ptv[i][0] == '?' || v[i][0] == '?')  
			continue;
		if (strchr(ptv[i], '%') != NULL) {
			if (strstr(v[i], ptv[i]+1) == NULL) 
				match = FALSE;
		}
		else {
			if (strcmp(ptv[i], v[i]) != 0) 
				match = FALSE;
        }
		
	}
	freeVals(ptv,na); 
	freeVals(v,na);
	// free(ptv);
	// free(v);
	return match;
}

// puts printable version of tuple in user-supplied buffer

void tupleString(Tuple t, char *buf)
{
	strcpy(buf,t);
}

// release memory used for tuple
void freeTuple(Tuple t)
{
    if (t != NULL) {
        free(t);
    }
}
