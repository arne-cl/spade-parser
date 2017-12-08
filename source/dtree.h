#ifndef _DTREE_
#define _DTREE_ 

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>

typedef struct {
  char *status;
  int span[2];
  char *rel2par;
  int mark; /* 0:nothing, 1:sentence tree root node, 2:all children are marked >0, 
	       3: siblings marked 0 and 2-3, 4: siblings marked 2-4 and 0  */
  char *text;
  int *btext, *etext; /* begin and end of each piece (nucleus-only || all) in char-length */
  int *bw, *ew; /* begin and end of each piece (nucleus-only || all) in word-length */
  int npieces;
} Node;

typedef struct struct_DTree{
  Node *node;
  struct struct_DTree *child;
  struct struct_DTree *sibling;
  struct struct_DTree *bsibling;
} DTree;

DTree* readDTree(FILE *fti, int offset);
DTree* readDTreeStruct(FILE *fti, DTree *lsib, char **text, int *pos, int *wc, int offset);
Node* readNode(FILE *fti, char **text, int *pos, int *wc, int offset);
int updateText(DTree *tree, char *text);
void restoreSpans(DTree *tree);

DTree *deepCopyDTree(DTree *dtree, DTree *bsibling);
DTree *findParent(DTree *tree, DTree *ct);
int replaceNodeKeepRel(DTree *tree, DTree *ct, DTree *rt);

void updateMarks(DTree *tree);
DTree** getSentenceTrees(DTree *tree, int *nstrees);
DTree* locatedTree(DTree *tree, int b, int e);
void getSentenceEndEdu(DTree *tree, int *seduNr, int *nstrees);
int endSentence(Node *node, char nextC);
int unambiguousEOSChar(char c);

DTree** getParagraphTrees(DTree *tree, int *nstrees);
void getParagraphEndEdu(DTree *tree, int *seduNr, int *nstrees);

DTree** getSegmentTrees(char *fname, DTree *tree, int *nstrees);

int endOfSentenceChar(char c);
char quotingChar(char c);
int endParagraph(Node *node);

void writeDTree(FILE *fto, DTree *tree, int tab);
void writeNode(FILE *fto, Node *node);

int countLeaves(DTree *tree);
int treeHeight(DTree *tree);

void relClass(char class[], char *rel);
void nodeText(DTree *dtree, char bigbuf[]);

void getName(char fname[], char *allname);

#endif
