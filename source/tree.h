#include <stdio.h>
#include <string.h>

typedef struct struct_Tree{
  char *pos;
  char *lex;
  char **text; /* word array of the whole tree */
  int ntext;
  int *edubreak; /* edubreak[] = i if edu break after i-th child; list ends in 0 */
  int b, e;    /* begining/end of spanned text as words */
  struct struct_Tree* child;
  struct struct_Tree *siblingL, *siblingR;
  struct struct_Tree *parent;
} Tree;

Tree* readTree(FILE *f);
Tree* readTree1(FILE *f, Tree *parent, Tree *siblingL, int *off, char last[]);
int increaseLeavesCount(char *pos, char *lex, char last[]);
void updateSpans(Tree *tree);
void updateTreeText(Tree *tree);
int getLeaves(Tree *tree, char *leaf[], int i);
void updateRecTreeText(Tree *tree, char **text);

void updateLexHead(Tree *tree);
char* lexHead(Tree *tree);
char *lexSearchLeft(Tree *tree, char* list[], int nlist, int *found);
char* lexSearchRight(Tree *tree, char* list[], int nlist, int *found);

int nodeNumber(Tree *tree);
int leafNumber(Tree *tree);
int height(Tree *tree);
int countArity(Tree *tree, int c);
int hasPOSLeaf(Tree *tree, char *pos);

void writeTree(Tree *tree, int tab);
void writeLEXPOSLeaves(FILE *f, Tree *tree);
void writePOSLeaves(Tree *tree);
int writeLeaves(Tree *tree, int flag);
int fwriteLeaves(FILE *f, Tree *tree, int flag);
void writePOSClusters(Tree *tree);

Tree* deepCopy(Tree *tree, Tree *siblingL);
void freeTree(Tree *tree);

int sameString(char *ts, char *ds);
int sameCompactString(char *ts, char *ds);
int almostEqual(char *s1, char *s2);
void getLex(char *lex, char *fullex);
int getBLex(char *lex);
int getELex(char *lex);
int isPunctuation(char *s);
