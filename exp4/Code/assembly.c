#include "assembly.h"

#define debug_assem(a)   // printf(a)
#define debug_call(a)    // printf(a)
#define debug_devide(fw) // fprintf(fw, "\n\n")

const char *REG_NAME[REG_NUM] = {
    "$0", "$at", "$v0", "$v1", "$a0", "$a1", "$a2", "$a3", "$t0", "$t1", "$t2",
    "$t3", "$t4", "$t5", "$t6", "$t7", "$s0", "$s1", "$s2", "$s3", "$s4", "$s5",
    "$s6", "$s7", "$t8", "$t9", "$k0", "$k1", "$gp", "$sp", "$gp", "$ra"};

pRegisters initRegisters()
{
    pRegisters p = (pRegisters)malloc(sizeof(Registers));
    assert(p != nullptr);
    for (int i = 0; i < REG_NUM; i++)
    {
        p->regList[i] = newRegister(REG_NAME[i]);
    }
    p->regList[0]->isFree = false; // $0 is not for use.
    p->lastchangedNo = 0;
    return p;
}

void resetRegisters(pRegisters registers)
{
    for (int i = 1; i < REG_NUM; i++)
    {
        registers->regList[i]->isFree = true;
    }
}

void deleteRegisters(pRegisters registers)
{
    assert(registers != nullptr);
    for (int i = 0; i < REG_NUM; i++)
    {
        free(registers->regList[i]);
    }
    free(registers);
}

pVarTable newVarTable()
{
    pVarTable p = (pVarTable)malloc(sizeof(VarTable));
    assert(p != nullptr);
    p->varListReg = newAssemVarList();
    p->varListMem = newAssemVarList();
    p->sp = 0;
    return p;
}

void deleteVarTable(pVarTable varTable)
{
    assert(varTable != nullptr);
    clearAssemVarList(varTable->varListReg);
    clearAssemVarList(varTable->varListMem);
    free(varTable->varListReg);
    free(varTable->varListMem);
    varTable->sp = 0;
    free(varTable);
}

pAssemVarList newAssemVarList()
{
    pAssemVarList p = (pAssemVarList)malloc(sizeof(AssemVarList));
    assert(p != nullptr);
    p->head = nullptr;
    p->cur = nullptr;
    return p;
}

void printAssemVarList(FILE *fp, pAssemVarList varList)
{
    fprintf(fp, "------VariableList------\n");
    pVariable tmp = varList->head;
    while (tmp != nullptr)
    {
        if (tmp->op->kind == OP_CONSTANT)
        {
            fprintf(fp, "reg: %s, value: %d\n", REG_NAME[tmp->index], tmp->op->u.value);
        }
        else
        {
            fprintf(fp, "reg: %s, value: %s\n", REG_NAME[tmp->index], tmp->op->u.name);
        }
        tmp = tmp->next;
    }
    fprintf(fp, "--------end--------\n");
}

void addVariable(pAssemVarList varList, int regNo, pOperand op)
{
    pVariable newVar = newVariable(regNo, op);
    if (varList->head == nullptr)
    {
        varList->head = newVar;
    }
    else
    {
        assert(varList->cur != nullptr);
        varList->cur->next = newVar;
    }
    varList->cur = newVar;
}

void removeVariable(pAssemVarList varList, pVariable var)
{
    if (var == varList->head)
    {
        varList->head = varList->head->next;
    }
    else
    {
        pVariable tmp = varList->head;
        while (tmp != nullptr)
        {
            if (tmp->next == var)
                break;
            tmp = tmp->next;
        }
        if (varList->cur == var)
            varList->cur = tmp;
        tmp->next = var->next;
    }
    var->next = nullptr;
}

void delVariable(pAssemVarList varList, pVariable var)
{
    removeVariable(varList, var);
    free(var);
}

void clearAssemVarList(pAssemVarList varList)
{
    assert(varList != nullptr);
    pVariable tmp = varList->head;
    while (tmp != nullptr)
    {
        pVariable p = tmp;
        tmp = tmp->next;
        free(p);
    }
    varList->head = nullptr;
    varList->cur = nullptr;
}

int checkVariable(FILE *fp, pVarTable varTable, pRegisters registers, pOperand op)
{
    assert(op != nullptr);
    if (op->kind != OP_CONSTANT)
    {
        int regNo = allocReg(registers, varTable, op, fp);
        pVariable memTmp = varTable->varListMem->head;
        while (memTmp != nullptr)
        {
            assert(memTmp->op->kind != OP_CONSTANT);
            if (!strcmp(memTmp->op->u.name, op->u.name))
            {
                assert((memTmp->index < 0 || memTmp->index >= 8) && memTmp->index % 4 == 0);
                fprintf(fp, "  lw %s, %d($gp)\n", registers->regList[regNo]->name, memTmp->index);
                return regNo;
            }
            memTmp = memTmp->next;
        }
        if(memTmp == nullptr){
            assert(0); // allocate space in IR_FUNCTION
        }
        return regNo;
    }
    else
    {
        // For an intermediate, allocate a reg for it; if it is 0, return $0.
        if (op->u.value == 0)
            return ZERO;
        pVariable tmp = varTable->varListReg->head;
        // always allocate a new reg for intermediate, otherwise inside a loop, program may go wrong.
        int regNo = allocReg(registers, varTable, op, fp);
        fprintf(fp, "  li %s, %d\n", registers->regList[regNo]->name, op->u.value);
        return regNo;
    }
}

int allocReg(pRegisters registers, pVarTable varTable, pOperand op, FILE *fp)
{
    // If there are free regs, use them first.
    for (int i = T0; i <= T9; i++)
    {
        if (registers->regList[i]->isFree)
        {
            registers->regList[i]->isFree = false;
            addVariable(varTable->varListReg, i, op);
            return i;
        }
    }
    // If no free regs to allocated,
    // free regs with recently-not-used intermediates
    pVariable tmp = varTable->varListReg->head;
    while (tmp != nullptr)
    {
        if (tmp->op->kind == OP_CONSTANT &&
            tmp->index != registers->lastchangedNo)
        {
            int regNo = tmp->index;
            registers->lastchangedNo = regNo;
            delVariable(varTable->varListReg, tmp);
            addVariable(varTable->varListReg, regNo, op);
            return regNo;
        }
        tmp = tmp->next;
    }
    // if no such intermediates, free a recently-not-used temporary variable
    tmp = varTable->varListReg->head;
    while (tmp != nullptr)
    {
        // The temporary variable: 't' + number
        // The defined variable: 't_' + name
        // The arrays as global variable: '_t_' + name
        // The parameters: 'v_' + name
        if (tmp->op->kind != OP_CONSTANT && tmp->index != registers->lastchangedNo)
        {
            int regNo = tmp->index;
            registers->lastchangedNo = regNo;
            // add new operand to registers
            delVariable(varTable->varListReg, tmp);
            addVariable(varTable->varListReg, regNo, op);
            return regNo;
        }
        tmp = tmp->next;
    }
    // should not reach here
    assert(0);
    return 0;
}

pRegister newRegister(const char *regName)
{
    pRegister p = (pRegister)malloc(sizeof(Register));
    assert(p != nullptr);
    p->isFree = true;
    p->name = regName;
    return p;
}

pVariable newVariable(int regNo, pOperand op)
{
    pVariable p = (pVariable)malloc(sizeof(Variable));
    assert(p != nullptr);
    p->index = regNo;
    p->op = op;
    p->next = nullptr;
}

void genAssemblyCode(FILE *fp)
{
    registers = initRegisters();
    varTable = newVarTable();
    initCode(fp);
    pInterCodes tmp = interCodeList->head;
    while (tmp != nullptr)
    {
        interToAssem(fp, tmp);
        debug_devide(fp);
        tmp = tmp->next;
    }
    deleteRegisters(registers);
    deleteVarTable(varTable);
    registers = nullptr;
    varTable = nullptr;
}

void initCode(FILE *fp)
{
    fprintf(fp, ".data\n");
    fprintf(fp, "_prompt: .asciiz \"Enter an integer:\"\n");
    fprintf(fp, "_ret: .asciiz \"\\n\"\n");
    fprintf(fp, ".globl main\n");
    
    // read function
    fprintf(fp, ".text\n");
    fprintf(fp, "read:\n");
    fprintf(fp, "  li $v0, 4\n");
    fprintf(fp, "  la $a0, _prompt\n");
    fprintf(fp, "  syscall\n");
    fprintf(fp, "  li $v0, 5\n");
    fprintf(fp, "  syscall\n");
    fprintf(fp, "  jr $ra\n\n");

    // write function
    fprintf(fp, "write:\n");
    fprintf(fp, "  li $v0, 1\n");
    fprintf(fp, "  syscall\n");
    fprintf(fp, "  li $v0, 4\n");
    fprintf(fp, "  la $a0, _ret\n");
    fprintf(fp, "  syscall\n");
    fprintf(fp, "  move $v0, $0\n");
    fprintf(fp, "  jr $ra\n");
}

void interToAssem(FILE *fp, pInterCodes interCodes)
{
    pInterCode interCode = interCodes->code;
    int kind = interCode->kind;
    if (kind == IR_LABEL)
    {
        debug_assem("IR_LABEL\n");
        fprintf(fp, "%s:\n", interCode->u.oneOp.op->u.name);
    }
    else if (kind == IR_FUNCTION)
    {
        debug_assem("IR_FUNCTION\n");
        fprintf(fp, "\n%s:\n", interCode->u.oneOp.op->u.name);
        // init $gp
        fprintf(fp, "  move $gp, $sp\n");

        // According to the experiment requirement, there is no global variable.
        // So when calling a function, reset the variable table.
        resetRegisters(registers);
        clearAssemVarList(varTable->varListReg);
        clearAssemVarList(varTable->varListMem);
        varTable->sp = 0;

        // handle main function specifically.
        // handle parameters IR_PARAM:
        pItem item = searchFirstTableItem(table, interCode->u.oneOp.op->u.name + 2);
        int argc = 0;
        pInterCodes tmp = interCodes->next;
        while (tmp != nullptr && tmp->code->kind == IR_PARAM)
        {
            pOperand op = tmp->code->u.oneOp.op;
            // All arguments are stored in the stack
            assert(op->kind != OP_CONSTANT);
            addVariable(varTable->varListMem, argc * 4 + 8, op);
            argc++;
            tmp = tmp->next;
        }

        pInterCodes icptr = interCodes->next;
        while(icptr != nullptr){
            int kind = icptr->code->kind;
            pOperand op = nullptr; 
            if(kind == IR_FUNCTION) break;
            switch(kind){
            case IR_READ:
                op = icptr->code->u.oneOp.op;
                break;
            case IR_ASSIGN: // assign
            case IR_CALL:
            case IR_GET_ADDR:
            case IR_READ_ADDR:
                op = icptr->code->u.assign.left;
                break;
            case IR_ADD: // binOp
            case IR_ADD_ADDR:
            case IR_SUB:
            case IR_MUL:
            case IR_DIV:
                op = icptr->code->u.binOp.result;
                break;
            case IR_DEC: // dec, for function
                op = icptr->code->u.dec.op;
                break;
            default: // Should not reach here.
                op = nullptr;
                break;
            }
            if(op != nullptr){
                if(icptr->code->kind == IR_DEC){
                    varTable->sp -= icptr->code->u.dec.size + 4;
                    fprintf(fp, "  addi $sp, $sp, -%d\n", icptr->code->u.dec.size);
                    fprintf(fp, "  sw $sp, -4($sp)\n");
                    fprintf(fp, "  addi $sp, $sp, -4\n");
                    addVariable(varTable->varListMem, varTable->sp, op);
                    fprintf(fp, "    #allocate %d($gp) for array %s with base at %d($gp)\n", varTable->sp, op->u.name, varTable->sp + 4);
                }
                else{
                    // allocate stack space for the variable
                    fprintf(fp, "  addi $sp, $sp, -4\n");
                    varTable->sp -= 4;
                    addVariable(varTable->varListMem, varTable->sp, op);
                    fprintf(fp, "    #allocate %d($gp) for %s\n",varTable->sp, op->u.name);
                }
            }
            icptr = icptr->next;
        }

        
    }
    else if (kind == IR_GOTO)
    {
        debug_assem("IR_GOTO\n");
        fprintf(fp, "  j %s\n", interCode->u.oneOp.op->u.name);
    }
    else if (kind == IR_RETURN)
    {
        debug_assem("IR_RETURN\n");
        pOperand op = interCode->u.oneOp.op;
        int regNo = checkVariable(fp, varTable, registers, op);
        fprintf(fp, "  move $v0, %s\n", registers->regList[regNo]->name);
        fprintf(fp, "  jr $ra\n");
    }
    else if (kind == IR_ARG)
    {
    } // IR_ARGs are handled inside call
    else if (kind == IR_PARAM)
    {
    } // IR_PARAMs are handled inside function
    else if (kind == IR_READ)
    {
        debug_assem("IR_READ\n");
        fprintf(fp, "  addi $sp, $sp, -4\n");
        fprintf(fp, "  sw $ra, 0($sp)\n");
        fprintf(fp, "  jal read\n");
        fprintf(fp, "  lw $ra, 0($sp)\n");
        fprintf(fp, "  addi $sp, $sp, 4\n");
        int regNo = checkVariable(fp, varTable, registers, interCode->u.oneOp.op);
        fprintf(fp, "  move %s, $v0\n", registers->regList[regNo]->name);
        writeBackToStack(fp, regNo, varTable, interCode->u.oneOp.op);
    }
    else if (kind == IR_WRITE)
    {
        debug_assem("IR_WRITE\n");
        int regNo = checkVariable(fp, varTable, registers, interCode->u.oneOp.op);
        // caller save $a0 first
        fprintf(fp, "  addi $sp, $sp, -8\n");
        fprintf(fp, "  sw $a0, 0($sp)\n");
        fprintf(fp, "  sw $ra, 4($sp)\n");
        fprintf(fp, "  move $a0, %s\n", registers->regList[regNo]->name);
        fprintf(fp, "  jal write\n");
        fprintf(fp, "  lw $a0, 0($sp)\n");
        fprintf(fp, "  lw $ra, 4($sp)\n");
        fprintf(fp, "  addi $sp, $sp, 8\n");
    }
    else if (kind == IR_ASSIGN)
    {
        debug_assem("IR_ASSIGN\n");
        pOperand left = interCode->u.assign.left, right = interCode->u.assign.right;
        assert(left->kind == OP_VARIABLE);
        int leftRegNo = checkVariable(fp, varTable, registers, left);
        if (right->kind == OP_CONSTANT)
        {
            fprintf(fp, "  li %s, %d\n", registers->regList[leftRegNo]->name, interCode->u.assign.right->u.value);
        }
        else if (right->kind == OP_VARIABLE)
        {
            int rightRegNo = checkVariable(fp, varTable, registers, right);
            fprintf(fp, "  move %s, %s\n", registers->regList[leftRegNo]->name, registers->regList[rightRegNo]->name);
        }
        else
        {
            assert(0);
        }
        writeBackToStack(fp, leftRegNo, varTable, left);
    }
    else if (kind == IR_GET_ADDR)
    {
        debug_assem("IR_GET_ADDR\n");
        pOperand left = interCode->u.assign.left, right = interCode->u.assign.right;
        int leftRegNo = checkVariable(fp, varTable, registers, left);
        pVariable memTmp = varTable->varListMem->head;
        while(memTmp != nullptr){
            assert(memTmp->op->kind == OP_VARIABLE);
            if(!strcmp(memTmp->op->u.name, right->u.name)){
                break;
            }
            memTmp = memTmp->next;
        }
        assert(memTmp != nullptr);
        fprintf(fp, "  lw %s, %d($gp)\n", registers->regList[leftRegNo]->name, memTmp->index);
        writeBackToStack(fp, leftRegNo, varTable, left);
    }
    else if (kind == IR_READ_ADDR)
    {
        debug_assem("IR_READ_ADDR\n");
        pOperand left = interCode->u.assign.left, right = interCode->u.assign.right;
        int leftRegNo = checkVariable(fp, varTable, registers, left);
        int rightRegNo = checkVariable(fp, varTable, registers, right);
        fprintf(fp, "  lw %s, 0(%s)\n", registers->regList[leftRegNo]->name, registers->regList[rightRegNo]->name);
        writeBackToStack(fp, leftRegNo, varTable, left);
    }
    else if (kind == IR_WRITE_ADDR)
    {
        debug_assem("IR_WRITE_ADDR\n");
        pOperand left = interCode->u.assign.left, right = interCode->u.assign.right;
        int leftRegNo = checkVariable(fp, varTable, registers, left);
        int rightRegNo = checkVariable(fp, varTable, registers, right);
        fprintf(fp, "  sw %s, 0(%s)\n", registers->regList[rightRegNo]->name, registers->regList[leftRegNo]->name);
    }
    else if (kind == IR_CALL)
    {
        debug_assem("IR_CALL\n");
        pOperand left = interCode->u.assign.left, right = interCode->u.assign.right;
        assert(left->kind == OP_VARIABLE);
        pItem calledFunc = searchFirstTableItem(table, right->u.name + 2);
        assert(calledFunc != nullptr);
        int leftRegNo = checkVariable(fp, varTable, registers, left);
        // Preparations before a function call
        pusha(fp, varTable); // store T0 - T9
        debug_call("pusha\n");

        // handle arguments: IR_ARG
        pInterCodes arg = interCodes->prev;
        int argc = 0, tot_argc = calledFunc->field->type->u.func.argc;
        fprintf(fp, "  addi $sp, $sp, -%d\n", tot_argc * 4);
        varTable->sp -= tot_argc * 4;
        while (arg != nullptr && argc < tot_argc)
        {
            int argRegNo = checkVariable(fp, varTable, registers, arg->code->u.oneOp.op);
            if (arg->code->u.oneOp.op->kind == OP_ADDRESS)
            {
                fprintf(fp, "  lw %s, 0(%s)\n", registers->regList[argRegNo]->name, registers->regList[argRegNo]->name);
            }

            // All arguments are stored in stack.
            fprintf(fp, "  sw %s, %d($sp)\n", registers->regList[argRegNo]->name, 4 * argc);
            argc++;

            arg = arg->prev;
        }
        debug_call("handle arguments\n");
        // store fp and ra
        fprintf(fp, "  addi $sp, $sp, -8\n");
        varTable->sp -= 8;
        fprintf(fp, "  sw $ra, 0($sp)\n");
        fprintf(fp, "  sw $gp, 4($sp)\n");

        // function call
        fprintf(fp, "  jal %s\n", interCode->u.assign.right->u.name);

        // recover the top of stack
        fprintf(fp, "  move $sp, $gp\n");
        // recover $ra and $gp
        fprintf(fp, "  lw $ra, 0($sp)\n");
        fprintf(fp, "  lw $gp, 4($sp)\n");
        fprintf(fp, "  addi $sp, $sp, %d\n", 8 + tot_argc * 4);
        varTable->sp += 8 + tot_argc * 4;
        // recover T0-T9
        popa(fp, varTable);
        debug_call("popa\n");

        // assign return value
        fprintf(fp, "  move %s, $v0\n", registers->regList[leftRegNo]->name);

        writeBackToStack(fp, leftRegNo, varTable, left);
    }
    else if (kind == IR_ADD || kind == IR_ADD_ADDR)
    {
        debug_assem("IR_ADD\n");
        pOperand result = interCode->u.binOp.result;
        pOperand op1 = interCode->u.binOp.op1, op2 = interCode->u.binOp.op2;
        int resultRegNo = checkVariable(fp, varTable, registers, result);
        // constant and constant
        if (op1->kind == OP_CONSTANT && op2->kind == OP_CONSTANT)
        {
            fprintf(fp, "  li %s, %d\n",
                    registers->regList[resultRegNo]->name,
                    op1->u.value + op2->u.value);
        }
        // variable and constant
        else if (op1->kind != OP_CONSTANT && op2->kind == OP_CONSTANT)
        {
            int op1RegNo = checkVariable(fp, varTable, registers, op1);
            fprintf(fp, "  addi %s, %s, %d\n",
                    registers->regList[resultRegNo]->name,
                    registers->regList[op1RegNo]->name,
                    op2->u.value);
        }
        // constant and variable
        else if (op1->kind == OP_CONSTANT && op2->kind != OP_CONSTANT)
        {
            int op2RegNo = checkVariable(fp, varTable, registers, op2);
            fprintf(fp, "  addi %s, %s, %d\n",
                    registers->regList[resultRegNo]->name,
                    registers->regList[op2RegNo]->name,
                    op1->u.value);
        }
        // variable and variable
        else
        {
            int op1RegNo = checkVariable(fp, varTable, registers, op1);
            int op2RegNo = checkVariable(fp, varTable, registers, op2);
            fprintf(fp, "  add %s, %s, %s\n",
                    registers->regList[resultRegNo]->name,
                    registers->regList[op1RegNo]->name,
                    registers->regList[op2RegNo]->name);
        }
        writeBackToStack(fp, resultRegNo, varTable, result);
    }
    else if (kind == IR_SUB)
    {
        debug_assem("IR_SUB\n");
        pOperand result = interCode->u.binOp.result;
        pOperand op1 = interCode->u.binOp.op1, op2 = interCode->u.binOp.op2;
        int resultRegNo = checkVariable(fp, varTable, registers, result);
        // constant and constant
        if (op1->kind == OP_CONSTANT && op2->kind == OP_CONSTANT)
        {
            fprintf(fp, "  li %s, %d\n",
                    registers->regList[resultRegNo]->name,
                    op1->u.value - op2->u.value);
        }
        // variable and constant
        else if (op1->kind != OP_CONSTANT && op2->kind == OP_CONSTANT)
        {
            int op1RegNo = checkVariable(fp, varTable, registers, op1);
            fprintf(fp, "  addi %s, %s, %d\n",
                    registers->regList[resultRegNo]->name,
                    registers->regList[op1RegNo]->name,
                    -op2->u.value);
        }
        // variable and variable && constant and variable
        else
        {
            int op1RegNo = checkVariable(fp, varTable, registers, op1);
            int op2RegNo = checkVariable(fp, varTable, registers, op2);
            fprintf(fp, "  sub %s, %s, %s\n",
                    registers->regList[resultRegNo]->name,
                    registers->regList[op1RegNo]->name,
                    registers->regList[op2RegNo]->name);
        }
        writeBackToStack(fp, resultRegNo, varTable, result);
    }
    else if (kind == IR_MUL)
    {
        debug_assem("IR_MUL\n");
        pOperand result = interCode->u.binOp.result;
        pOperand op1 = interCode->u.binOp.op1, op2 = interCode->u.binOp.op2;
        int resultRegNo = checkVariable(fp, varTable, registers, result);
        int op1RegNo = checkVariable(fp, varTable, registers, op1);
        int op2RegNo = checkVariable(fp, varTable, registers, op2);
        fprintf(fp, "  mul %s, %s, %s\n", registers->regList[resultRegNo]->name,
                registers->regList[op1RegNo]->name,
                registers->regList[op2RegNo]->name);
        writeBackToStack(fp, resultRegNo, varTable, result);
    }
    else if (kind == IR_DIV)
    {
        debug_assem("IR_DIV\n");
        pOperand result = interCode->u.binOp.result;
        pOperand op1 = interCode->u.binOp.op1, op2 = interCode->u.binOp.op2;
        int resultRegNo = checkVariable(fp, varTable, registers, result);
        int op1RegNo = checkVariable(fp, varTable, registers, op1);
        int op2RegNo = checkVariable(fp, varTable, registers, op2);
        fprintf(fp, "  div %s, %s\n", registers->regList[op1RegNo]->name,
                registers->regList[op2RegNo]->name);
        fprintf(fp, "  mflo %s\n", registers->regList[resultRegNo]->name);
        writeBackToStack(fp, resultRegNo, varTable, result);
    }
    else if (kind == IR_DEC)
    {
    } // handle in IR_FUNCTION
    else if (kind == IR_IF_GOTO)
    { // GOTO statement
        debug_assem("IR_IF_GOTO\n");
        char *relopName = interCode->u.ifGoto.relop->u.name;
        pOperand x = interCode->u.ifGoto.x, y = interCode->u.ifGoto.y;
        int xRegNo = checkVariable(fp, varTable, registers, x);
        int yRegNo = checkVariable(fp, varTable, registers, y);
        if (!strcmp(relopName, "=="))
            fprintf(fp, "  beq %s, %s, %s\n", registers->regList[xRegNo]->name,
                    registers->regList[yRegNo]->name,
                    interCode->u.ifGoto.z->u.name);
        else if (!strcmp(relopName, "!="))
            fprintf(fp, "  bne %s, %s, %s\n", registers->regList[xRegNo]->name,
                    registers->regList[yRegNo]->name,
                    interCode->u.ifGoto.z->u.name);
        else if (!strcmp(relopName, ">"))
            fprintf(fp, "  bgt %s, %s, %s\n", registers->regList[xRegNo]->name,
                    registers->regList[yRegNo]->name,
                    interCode->u.ifGoto.z->u.name);
        else if (!strcmp(relopName, "<"))
            fprintf(fp, "  blt %s, %s, %s\n", registers->regList[xRegNo]->name,
                    registers->regList[yRegNo]->name,
                    interCode->u.ifGoto.z->u.name);
        else if (!strcmp(relopName, ">="))
            fprintf(fp, "  bge %s, %s, %s\n", registers->regList[xRegNo]->name,
                    registers->regList[yRegNo]->name,
                    interCode->u.ifGoto.z->u.name);
        else if (!strcmp(relopName, "<="))
            fprintf(fp, "  ble %s, %s, %s\n", registers->regList[xRegNo]->name,
                    registers->regList[yRegNo]->name,
                    interCode->u.ifGoto.z->u.name);
    }
}

void pusha(FILE *fp, pVarTable varTable)
{
    fprintf(fp, "  addi $sp, $sp, -72\n");
    varTable->sp -= 72;
    for (int i = T0; i <= T9; i++)
    {
        fprintf(fp, "  sw %s, %d($sp)\n", registers->regList[i]->name, (i - T0) * 4);
    }
}

void popa(FILE *fp, pVarTable varTable)
{
    for (int i = T0; i <= T9; i++)
    {
        fprintf(fp, "  lw %s, %d($sp)\n", registers->regList[i]->name, (i - T0) * 4);
    }
    fprintf(fp, "  addi $sp, $sp, 72\n");
    varTable->sp += 72;
}

void writeBackToStack(FILE* fp, int regNo, pVarTable varTable, pOperand op){
    assert(op != nullptr);
    //if(op->loopCond == 0) return;
    pVariable memTmp = varTable->varListMem->head;
    while(memTmp != nullptr){
        if(!strcmp(memTmp->op->u.name, op->u.name)){
            break;
        }
        memTmp = memTmp->next;
    }
    assert(memTmp != nullptr);
    fprintf(fp, "  sw %s, %d($gp)\n", registers->regList[regNo]->name, memTmp->index);
}
