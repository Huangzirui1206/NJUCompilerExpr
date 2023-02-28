#pragma once
#ifndef SEMANTIC_H
#define SEMENTIC_H

#include "type.h"
#include "node.h"

// C/C++ uses NAME_EQUIVALENT
// To implement the requirement of experiment 2, implement STRUCTURE_EQUIVALENT here. 
#define STRUCTURE_EQUIVALENT

#define HASH_TABLE_SIZE 0x400

#define ERROR_MSG_SIZE 100

typedef struct type* pType;
typedef struct fieldList* pFieldList;
typedef struct tableItem* pItem;
typedef struct hashTable* pHash;
typedef struct stack* pStack;
typedef struct table* pTable;
typedef boolean* pboolean;

typedef struct type {
    Kind kind; // The Kind of this typt: [BASIC, ARRAY, FUNC, STRUCTURE]
    union {
        // BASE
        BasicType basic;
        // ARRAY = array.type + array.size
        struct {
            int size; // The array.size is calculated in inter code translation.
            pType elem;
        } array;
        // STRUCTURE = structure.structName + structure.field (a link)
        struct {
            char* structName;
            pFieldList field;
        } structure;
        // FUNC = func.argc + func.argv (a link) + func.returnType
        struct {
            FuncState state;    // The state of the function: declared or defined
            int argc;          // argument counter
            pFieldList argv;   // argument vector
            pType returnType;  // returnType
            int lineno;
        } func;
        // POINTER = pointer.elem
        struct {
            pType elem;
        } pointer;
    } u;
} Type;

typedef struct fieldList {
    char* name;       // The name of the field
    pType type;       // The type of the field
    boolean isArg;    // For inter code translation
    pFieldList tail;  // Link next node
} FieldList;

typedef struct tableItem {
    int symbolDepth;
    char* icname;
    pFieldList field;
    pItem nextSymbol; // Next symbol in the same scope
    pItem prevSymbol; // Prev symbol in the same scope
    pItem nextHash; // Next symbol in the same hash bucket
    pItem prevHash; // Prev symbol in the same hash bucket
} TableItem; // Hash table item

typedef struct hashTable{
    pItem* hashArray;
} HashTable; // Open hash table

typedef struct stack {
    pItem* stackArray;
    int curStackDepth;
} Stack; // Stack for nesting field

typedef struct table {
    pHash hash;
    pStack stack;
    int unNamedStructNum;
} Table; // Crossing listed table

extern pTable table;

// Type functions
pType newType(Kind kind, ...);
pType copyType(pType src);
void deleteType(pType type);
boolean checkType(pType type1, pType type2);
void printType(pType type);

// Field Functions
pFieldList newFieldList(char* newName, pType newType);
pFieldList copyFieldList(pFieldList src);
void deleteFieldList(pFieldList fieldList);
void setFieldListName(pFieldList p, char* newName);
boolean checkFieldList(pFieldList list1, pFieldList list2);
void printFieldList(pFieldList fieldList);

// tableItem functions
pItem newItem(int symbolDepth, pFieldList pfield);
void deleteItem(pItem item);
boolean isStructDef(pItem src);

// Hash functions
pHash newHash();
void deleteHash(pHash hash);
pItem getHashHead(pHash hash, int index);
void setHashHead(pHash hash, int index, pItem newVal);
void setHashHeadEmpty(pHash hash, int index);

// Stack functions
pStack newStack();
void deleteStack(pStack stack);
void addStackDepth(pStack stack);
void minusStackDepth(pStack stack);
pItem getCurDepthStackHead(pStack stack);
void setCurDepthStackHead(pStack stack, pItem newVal);
void setCurDepthStackHeadEmpty(pStack stack);

// Table functions
pTable initTable();
void deleteTable(pTable table);
pItem searchFirstTableItem(pTable table, char* name);
boolean checkTableItemConflict(pTable table, pItem item);
void addTableItem(pTable table, pItem item);
void deleteTableItem(pTable table, pItem item);
void clearCurDepthStackList(pTable table);
void printTable(pTable table);

// Global functions
// The Hash function
static inline unsigned int getHashCode(char* name) {
    unsigned int val = 0, i;
    for (; *name; ++name) {
        val = (val << 2) + *name;
        if (i = val & ~HASH_TABLE_SIZE)
            val = (val ^ (i >> 12)) & HASH_TABLE_SIZE;
    }
    return val;
}

static inline void pError(ErrorType type, int line, char* msg) {
    printf("Error type %d at Line %d: %s\n", type, line, msg);
}

// Check func dec
boolean checkFunDec(pItem prev, pItem curr);

// Traverse tree
void traverseTree(pNode node);

extern pTable table;

// Generate symbol table functions
void ExtDef(pNode node);
void ExtDecList(pNode node, pType specifier);
pType Specifier(pNode node);
pType StructSpecifier(pNode node);
pItem VarDec(pNode node, pType specifier);
pItem FunDec(pNode node, pType returnType, FuncState funcState);
void VarList(pNode node, pItem func);
pFieldList ParamDec(pNode node);
void CompSt(pNode node, pItem funcItem);
boolean StmtList(pNode node, pItem funcItem);
boolean Stmt(pNode node, pItem funcItem);
void DefList(pNode node, pItem structInfo);
void Def(pNode node, pItem structInfo);
void DecList(pNode node, pType specifier, pItem structInfo);
void Dec(pNode node, pType specifier, pItem structInfo);
pType Exp(pNode node, pboolean lvalue);
void Args(pNode node, pFieldList funcArgInfo, int lineno);



#endif