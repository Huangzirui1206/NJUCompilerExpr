#pragma once
#ifndef ASSEMBLY_H
#define ASSEMBLY_H

#include "inter.h"
#include "node.h"

#define REG_NUM 32

typedef struct _register* pRegister;
typedef struct _variable* pVariable;
typedef struct _registers* pRegisters;
typedef struct _assemVarList* pAssemVarList;
typedef struct _varTable* pVarTable;

typedef struct _register{
    boolean isFree;
    const char* name;
} Register;

typedef struct _variable{
    int index; // regNo or offset from $fp
    pOperand op;
    pVariable next;
} Variable;

typedef struct _registers{
    pRegister regList[REG_NUM];
    int lastchangedNo;
} Registers;

typedef struct _assemVarList {
    pVariable head;
    pVariable cur;
} AssemVarList;

typedef struct _varTable{
    pAssemVarList varListReg; // The variable table in registers
    pAssemVarList varListMem; // The variable table in memory;
    int sp; // For local variables in the stack
} VarTable;

// Enum defined for registers
typedef enum _regNo {
    ZERO,    AT,
    V0,    V1,
    A0,    A1,    A2,    A3,
    T0,    T1,    T2,    T3,
    T4,    T5,    T6,    T7,
    S0,    S1,    S2,    S3,
    S4,    S5,    S6,    S7,
    T8,    T9,
    K0,    K1,
    GP,    SP,    FP,    RA,
} RegNo;

pRegisters registers;
pVarTable varTable;

pRegisters initRegisters();
void resetRegisters(pRegisters registers);
void deleteRegisters(pRegisters registers);

pVarTable newVarTable();
pAssemVarList newAssemVarList();
void printAssemVarList(FILE* fp, pAssemVarList varList);
void addVariable(pAssemVarList varList, int regNo, pOperand op);
void removeVariable(pAssemVarList varList, pVariable var);
void delVariable(pAssemVarList varList, pVariable var);
void clearAssemVarList(pAssemVarList AssemVarList);
int checkVariable(FILE* fp, pVarTable varTable, pRegisters registers, pOperand op);

int allocReg(pRegisters registers, pVarTable varTable, pOperand op, FILE* fp);

pRegister newRegister(const char* regName);
pVariable newVariable(int regNo, pOperand op);

void genAssemblyCode(FILE* fp);
void initCode(FILE* fp);
void interToAssem(FILE* fp, pInterCodes interCodes);

void pusha(FILE* fp, pVarTable varTable);
void popa(FILE* fp, pVarTable varTable);

void writeBackToStack(FILE* fp, int regNo, pVarTable varTable, pOperand op);


#endif