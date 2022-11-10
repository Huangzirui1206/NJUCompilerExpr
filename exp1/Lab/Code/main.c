#include <stdio.h>
#include "syntax.tab.h"
#include "type.h"
#include "node.h"
#include "semantic.h"

/*extern*/
extern pNode root;
void yyrestart(FILE*);

unsigned lexError = 0;
unsigned synError = 0;

int main(int argc, char** argv){
    if (argc <= 1) return 1;
    FILE* f = fopen(argv[1], "r");
    if (!f){
        perror(argv[1]);
        return 1;
    }
    yyrestart(f);
    yyparse();
    if(!lexError && !synError){
        table = initTable(root);
        traverseTree(root);
        // printTreeInfo(root, 0);
        deleteTable(table);
    }
    delNode(&root);
    return 0;
}