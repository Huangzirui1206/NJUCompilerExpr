#include <stdio.h>
#include "syntax.tab.h"
#include "type.h"
#include "node.h"
#include "semantic.h"

/*extern*/
extern pNode root;
void yyrestart(FILE*);

int lexError = 0;
int syntaxError = 0;

pTable table = nullptr;

int main(int argc, char** argv){
    if (argc <= 1) return 1;
    FILE* f = fopen(argv[1], "r");
    if (!f){
        perror(argv[1]);
        return 1;
    }
    yyrestart(f);
    yyparse();
    if(!lexError && !syntaxError){
        table = initTable(root);
        traverseTree(root);
        deleteTable(table);
    }
    if(root != nullptr) delNode(root);
    return 0;
}
