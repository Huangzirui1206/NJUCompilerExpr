#include <stdio.h>
#include "syntax.tab.h"
#include "type.h"
#include "node.h"
#include "semantic.h"
#include "inter.h"

/*extern*/
extern pNode root;
void yyrestart(FILE*);

int lexError = 0;
int syntaxError = 0;

pTable table = nullptr;

pInterCodeList interCodeList = nullptr;

int main(int argc, char** argv){
    if (argc <= 2) return 2;
    FILE* fr = fopen(argv[1], "r");
    if (!fr) {
        perror(argv[1]);
        return 1;
    }

    FILE* fw = fopen(argv[2], "wt+");
    if (!fw) {
        perror(argv[2]);
        return 1;
    }
    yyrestart(fr);
    yyparse();
    if(!lexError && !syntaxError){
        table = initTable(root);
        traverseTree(root);
        interCodeList = newInterCodeList();
        genInterCodes(root);
        printInterCode(fw, interCodeList);
        deleteTable(table);
    }
    if(root != nullptr) delNode(root);
    return 0;
}