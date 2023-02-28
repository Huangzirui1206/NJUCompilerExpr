#include "inter.h"

#define debug(a) //printf(a)

// If the string serves as a parameter, it needs deep copy with newString.
static inline char *newString(char *src)
{
    assert(src != nullptr);
    char *dst = (char *)malloc(strlen(src) + 1), *ptr = dst;
    for (; *src; src++)
        *(ptr++) = *src;
    *ptr = '\0';
    return dst;
}

static inline char *catString(char *src1, char *src2)
{
    assert(src1 != nullptr);
    assert(src2 != nullptr);
    char *dst = (char *)malloc(strlen(src1) + 1 + strlen(src2)), *ptr = dst;
    for (; *src1; src1++)
        *(ptr++) = *src1;
    for (; *src2; src2++)
        *(ptr++) = *src2;
    *ptr = '\0';
    return dst;
}

// Operand func
pOperand newOperand(int kind, ...)
{
    pOperand p = (pOperand)malloc(sizeof(Operand));
    assert(p != nullptr);
    p->kind = kind;
    assert(kind >= 0 && kind < 6);
    va_list vaList;
    va_start(vaList, kind);
    if (kind == OP_CONSTANT)
    {
        p->u.value = va_arg(vaList, int);
    }
    else
    {
        p->u.name = va_arg(vaList, char *); // name should be a new string.
    }
    p->elemType = nullptr;
    return p;
}

void deleteOperand(pOperand p)
{
    assert(p != nullptr);
    assert(p->kind >= 0 && p->kind < 6);
    if (p->kind != OP_CONSTANT)
    {
        if (p->u.name)
            free(p->u.name);
        p->u.name = nullptr;
    }
    else
    {
        p->u.value = 0;
    }
    p->elemType = nullptr;
    free(p);
}

void setOperand(pOperand p, int kind, ...)
{
    assert(p != nullptr);
    assert(kind >= 0 && kind < 6);
    p->kind = kind;
    va_list vaList;
    va_start(vaList, kind);
    if (kind == OP_CONSTANT)
    {
        p->u.value = va_arg(vaList, int);
    }
    else
    {
        if (p->u.name != nullptr)
            free(p->u.name);
        p->u.name = va_arg(vaList, char *);
    }
}

void setElemType(pOperand p, pType elementType)
{
    assert(p != nullptr);
    assert(p->elemType == nullptr);
    assert(elementType != nullptr);
    p->elemType = elementType;
}

void printOp(FILE *fp, pOperand op)
{
    assert(op != nullptr);
    if (fp == nullptr)
        fp = stdout;
    if (op->kind == OP_CONSTANT)
    {
        fprintf(fp, "#%d", op->u.value);
    }
    else
    {
        fprintf(fp, "%s", op->u.name);
    }
}

// InterCode func
pInterCode newInterCode(int kind, ...)
{
    assert(kind >= 0 && kind <= 20);
    va_list vaList;
    va_start(vaList, kind);
    pInterCode p = (pInterCode)malloc(sizeof(InterCode));
    p->kind = kind;
    switch (kind)
    {
    case IR_LABEL: // oneOp
    case IR_FUNCTION:
    case IR_ARG:
    case IR_ARG_ADDR:
    case IR_GOTO:
    case IR_RETURN:
    case IR_PARAM:
    case IR_READ:
    case IR_WRITE:
        p->u.oneOp.op = va_arg(vaList, pOperand);
        break;
    case IR_ASSIGN: // assign
    case IR_CALL:
    case IR_GET_ADDR:
    case IR_READ_ADDR:
    case IR_WRITE_ADDR:
        p->u.assign.left = va_arg(vaList, pOperand);
        p->u.assign.right = va_arg(vaList, pOperand);
        break;
    case IR_ADD: // binOp
    case IR_SUB:
    case IR_MUL:
    case IR_DIV:
    case IR_ADD_ADDR:
        p->u.binOp.result = va_arg(vaList, pOperand);
        p->u.binOp.op1 = va_arg(vaList, pOperand);
        p->u.binOp.op2 = va_arg(vaList, pOperand);
        break;
    case IR_IF_GOTO: // ifGoTo
        p->u.ifGoto.x = va_arg(vaList, pOperand);
        p->u.ifGoto.relop = va_arg(vaList, pOperand);
        p->u.ifGoto.y = va_arg(vaList, pOperand);
        p->u.ifGoto.z = va_arg(vaList, pOperand);
        break;
    case IR_DEC: // dec, for function
        p->u.dec.op = va_arg(vaList, pOperand);
        p->u.dec.size = va_arg(vaList, int);
        break;
    }
    return p;
}

void deleteInterCode(pInterCode p)
{
    assert(p != nullptr);
    switch (p->kind)
    {
    case IR_LABEL: // oneOp
    case IR_FUNCTION:
    case IR_ARG:
    case IR_GOTO:
    case IR_RETURN:
    case IR_PARAM:
    case IR_READ:
    case IR_WRITE:
        deleteOperand(p->u.oneOp.op);
        p->u.oneOp.op = nullptr;
        break;
    case IR_ASSIGN: // assign
    case IR_CALL:
    case IR_GET_ADDR:
    case IR_READ_ADDR:
    case IR_WRITE_ADDR:
        deleteOperand(p->u.assign.left);
        deleteOperand(p->u.assign.right);
        p->u.assign.left = nullptr;
        p->u.assign.right = nullptr;
        break;
    case IR_ADD: // binOp
    case IR_SUB:
    case IR_MUL:
    case IR_DIV:
        deleteOperand(p->u.binOp.result);
        deleteOperand(p->u.binOp.op1);
        deleteOperand(p->u.binOp.op2);
        p->u.binOp.result = nullptr;
        p->u.binOp.op1 = nullptr;
        p->u.binOp.op2 = nullptr;
        break;
    case IR_IF_GOTO: // ifGoTo
        deleteOperand(p->u.ifGoto.x);
        deleteOperand(p->u.ifGoto.relop);
        deleteOperand(p->u.ifGoto.y);
        deleteOperand(p->u.ifGoto.z);
        p->u.ifGoto.x = nullptr;
        p->u.ifGoto.relop = nullptr;
        p->u.ifGoto.y = nullptr;
        p->u.ifGoto.z = nullptr;
        break;
    case IR_DEC: // dec, for function
        deleteOperand(p->u.dec.op);
        p->u.dec.op = nullptr;
        p->u.dec.size = 0;
        break;
    }
    free(p);
}

void printInterCode(FILE *fp, pInterCodeList interCodeList)
{
    assert(interCodeList != nullptr);
    if (fp == nullptr)
        fp = stdout;
    pInterCodes p = interCodeList->head;
    while (p != nullptr)
    {
        assert(p->code->kind >= 0 && p->code->kind <= 20);
        switch (p->code->kind)
        {
        case IR_LABEL: // oneOp
            fprintf(fp, "LABEL ");
            assert(p->code->u.oneOp.op);
            printOp(fp, p->code->u.oneOp.op);
            fprintf(fp, " :");
            break;
        case IR_FUNCTION:
            fprintf(fp, "FUNCTION ");
            assert(p->code->u.oneOp.op);
            printOp(fp, p->code->u.oneOp.op);
            fprintf(fp, " :");
            break;
        case IR_ARG:
        case IR_ARG_ADDR:
            fprintf(fp, "ARG ");
            assert(p->code->u.oneOp.op);
            printOp(fp, p->code->u.oneOp.op);
            break;
        case IR_GOTO:
            fprintf(fp, "GOTO ");
            assert(p->code->u.oneOp.op);
            printOp(fp, p->code->u.oneOp.op);
            break;
        case IR_RETURN:
            fprintf(fp, "RETURN ");
            assert(p->code->u.oneOp.op);
            printOp(fp, p->code->u.oneOp.op);
            break;
        case IR_PARAM:
            fprintf(fp, "PARAM ");
            assert(p->code->u.oneOp.op);
            printOp(fp, p->code->u.oneOp.op);
            break;
        case IR_READ:
            fprintf(fp, "READ ");
            assert(p->code->u.oneOp.op);
            printOp(fp, p->code->u.oneOp.op);
            break;
        case IR_WRITE:
            fprintf(fp, "WRITE ");
            assert(p->code->u.oneOp.op);
            printOp(fp, p->code->u.oneOp.op);
            break;
        case IR_ASSIGN: // assign
            assert(p->code->u.assign.left && p->code->u.assign.right);
            printOp(fp, p->code->u.assign.left);
            fprintf(fp, " := ");
            printOp(fp, p->code->u.assign.right);
            break;
        case IR_CALL:
            assert(p->code->u.assign.left && p->code->u.assign.right);
            printOp(fp, p->code->u.assign.left);
            fprintf(fp, " := CALL ");
            printOp(fp, p->code->u.assign.right);
            break;
        case IR_GET_ADDR:
            assert(p->code->u.assign.left && p->code->u.assign.right);
            printOp(fp, p->code->u.assign.left);
            fprintf(fp, " := &");
            printOp(fp, p->code->u.assign.right);
            break;
        case IR_READ_ADDR:
            assert(p->code->u.assign.left && p->code->u.assign.right);
            printOp(fp, p->code->u.assign.left);
            fprintf(fp, " := *");
            printOp(fp, p->code->u.assign.right);
            break;
        case IR_WRITE_ADDR:
            assert(p->code->u.assign.left && p->code->u.assign.right);
            fprintf(fp, "*");
            printOp(fp, p->code->u.assign.left);
            fprintf(fp, " := ");
            printOp(fp, p->code->u.assign.right);
            break;
        case IR_ADD: // binOp
        case IR_ADD_ADDR:
            assert(p->code->u.binOp.result && p->code->u.binOp.op1 && p->code->u.binOp.op2);
            printOp(fp, p->code->u.binOp.result);
            fprintf(fp, " := ");
            printOp(fp, p->code->u.binOp.op1);
            fprintf(fp, " + ");
            printOp(fp, p->code->u.binOp.op2);
            break;
        case IR_SUB:
            assert(p->code->u.binOp.result && p->code->u.binOp.op1 && p->code->u.binOp.op2);
            printOp(fp, p->code->u.binOp.result);
            fprintf(fp, " := ");
            printOp(fp, p->code->u.binOp.op1);
            fprintf(fp, " - ");
            printOp(fp, p->code->u.binOp.op2);
            break;
        case IR_MUL:
            assert(p->code->u.binOp.result && p->code->u.binOp.op1 && p->code->u.binOp.op2);
            printOp(fp, p->code->u.binOp.result);
            fprintf(fp, " := ");
            printOp(fp, p->code->u.binOp.op1);
            fprintf(fp, " * ");
            printOp(fp, p->code->u.binOp.op2);
            break;
        case IR_DIV:
            assert(p->code->u.binOp.result && p->code->u.binOp.op1 && p->code->u.binOp.op2);
            printOp(fp, p->code->u.binOp.result);
            fprintf(fp, " := ");
            printOp(fp, p->code->u.binOp.op1);
            fprintf(fp, " / ");
            printOp(fp, p->code->u.binOp.op2);
            break;
        case IR_IF_GOTO: // ifGoTo
            assert(p->code->u.ifGoto.x && p->code->u.ifGoto.relop && p->code->u.ifGoto.y && p->code->u.ifGoto.z);
            fprintf(fp, "IF ");
            printOp(fp, p->code->u.ifGoto.x);
            fprintf(fp, " ");
            printOp(fp, p->code->u.ifGoto.relop);
            fprintf(fp, " ");
            printOp(fp, p->code->u.ifGoto.y);
            fprintf(fp, " GOTO ");
            printOp(fp, p->code->u.ifGoto.z);
            break;
        case IR_DEC: // dec, for function
            assert(p->code->u.dec.op);
            fprintf(fp, "DEC ");
            printOp(fp, p->code->u.dec.op);
            fprintf(fp, " %d", p->code->u.dec.size);
            break;
        default: // Should not reach here.
            assert(0);
        }
        fprintf(fp, "\n");
        p = p->next;
    }
}

// InterCodes func
pInterCodes newInterCodes(pInterCode code)
{
    pInterCodes p = (pInterCodes)malloc(sizeof(InterCodes));
    assert(p != nullptr);
    p->code = code;
    p->prev = nullptr;
    p->next = nullptr;
    return p;
}

void deleteInterCodes(pInterCodes p)
{
    assert(p != nullptr);
    deleteInterCode(p->code);
    p->code = nullptr;
    p->prev = nullptr;
    p->next = nullptr;
    free(p);
}

// Arg and ArgList func
pArg newArg(pOperand op)
{
    pArg p = (pArg)malloc(sizeof(Arg));
    assert(p != nullptr);
    p->op = op;
    p->next = nullptr;
    return p;
}

pArgList newArgList()
{
    pArgList p = (pArgList)malloc(sizeof(ArgList));
    assert(p != nullptr);
    p->head = nullptr;
    p->cur = nullptr;
}

void deleteArg(pArg p)
{
    assert(p != nullptr);
    deleteOperand(p->op);
    p->op = nullptr;
    free(p);
}

void deleteArgList(pArgList p)
{
    assert(p != nullptr);
    pArg ptr = p->head;
    while (p->head)
    {
        ptr = p->head;
        p->head = ptr->next;
        deleteArg(ptr);
    }
    p->cur = p->head = nullptr;
    free(p);
}

void addArg(pArgList argList, pArg arg)
{
    assert(argList != nullptr);
    assert(arg != nullptr);
    if (argList->head == nullptr)
    {
        argList->head = arg;
        argList->cur = arg;
    }
    else
    {
        assert(argList->cur != nullptr);
        argList->cur->next = arg;
        argList->cur = arg;
    }
}

// InterCodeList func
pInterCodeList newInterCodeList()
{
    pInterCodeList p = (pInterCodeList)malloc(sizeof(InterCodeList));
    assert(p != nullptr);
    p->cur = nullptr;
    p->head = nullptr;
    p->labelNum = 0;
    p->tmpVarNum = 0;
    p->lastArrayElem = nullptr;
    return p;
}

void deleteInterCodeList(pInterCodeList p)
{
    assert(p != nullptr);
    pInterCodes ptr = p->head;
    while (p->head != nullptr)
    {
        ptr = p->head;
        p->head = ptr->next;
        deleteInterCodes(ptr);
    }
    p->head = p->cur = nullptr;
    free(p);
}

void addInterCode(pInterCodeList interCodeList, pInterCodes newCode)
{
    assert(interCodeList != nullptr);
    assert(newCode != nullptr);
    if (interCodeList->head == nullptr)
    {
        interCodeList->head = newCode;
        interCodeList->cur = newCode;
    }
    else
    {
        assert(interCodeList->cur != nullptr);
        interCodeList->cur->next = newCode;
        interCodeList->cur = newCode;
    }
}

// traverse func
pOperand newTmp()
{
    char tName[TLEN] = {'\0'};
    sprintf(tName, "t%d", interCodeList->tmpVarNum);
    interCodeList->tmpVarNum += 1;
    pOperand p = newOperand(OP_VARIABLE, newString(tName));
    return p;
}

pOperand newLabel()
{
    char tName[TLEN] = {'\0'};
    sprintf(tName, "label%d", interCodeList->labelNum);
    interCodeList->labelNum += 1;
    pOperand p = newOperand(OP_LABEL, newString(tName));
    return p;
}

int getSize(pType type)
{
    // Get the memory size of a variable for array operation.
    assert(type != nullptr);
    if (type->kind == BASIC)
    {
        return 4;
    }
    else if (type->kind == ARRAY)
    {
        assert(type->u.array.size > 0);
        return type->u.array.size * getSize(type->u.array.elem);
    }
    else if (type->kind == STRUCTURE)
    {
        int cnt = 0;
        pFieldList p = type->u.structure.field;
        while (p != nullptr)
        {
            cnt += getSize(p->type);
            p = p->tail;
        }
        return cnt;
    }
    else
    {
        // Ignore pointer here, and function should appear here.
        assert(0);
    }
}

pType getElement(pType type)
{
    assert(type != nullptr);
    if (type->kind == ARRAY)
    {
        return getElement(type->u.array.elem);
    }
    else
    {
        return type;
    }
}

void genInterCodes(pNode node)
{
    if (node == nullptr)
        return;
    else if (!strcmp(node->name, "ExtDefList"))
    {
        // ExtDefList servers as the entry of the semantic tree traverse.
        translateExtDefList(node);
    }
    else
    {
        genInterCodes(node->children);
        genInterCodes(node->next);
    }
}

void genInterCode(int kind, ...)
{
    // Generate inter code for a specific semantic tree node and insert it into interCodeList
    va_list vaList;
    pOperand tmp = nullptr;
    pOperand result = nullptr, op1 = nullptr, op2 = nullptr, relop = nullptr;
    int size = 0;
    pInterCodes newCode = nullptr;
    assert(kind >= 0 && kind <= 20);
    va_start(vaList, kind);
    switch (kind)
    {
    case IR_LABEL: // oneOp
    case IR_FUNCTION:
    case IR_GOTO:
    case IR_RETURN:
    case IR_PARAM:
    case IR_READ:
    case IR_ARG:
    case IR_WRITE:
        op1 = va_arg(vaList, pOperand);
        assert(op1);
        if (op1->kind == OP_ADDRESS)
        {
            tmp = newTmp();
            genInterCode(IR_READ_ADDR, tmp, op1);
            op1 = tmp;
        }
        newCode = newInterCodes(newInterCode(kind, op1));
        addInterCode(interCodeList, newCode);
        break;
    case IR_ARG_ADDR: // one op, but don't read address
        op1 = va_arg(vaList, pOperand);
        assert(op1);
        newCode = newInterCodes(newInterCode(kind, op1));
        addInterCode(interCodeList, newCode);
        break;
    case IR_ASSIGN: // assign
    case IR_CALL:
    case IR_GET_ADDR:
    case IR_READ_ADDR:
    case IR_WRITE_ADDR:
        op1 = va_arg(vaList, pOperand);
        op2 = va_arg(vaList, pOperand);
        assert(op1 && op2);
        if (kind == IR_ASSIGN &&
            (op1->kind == OP_ADDRESS || op2->kind == OP_ADDRESS))
        {
            if (op1->kind == OP_ADDRESS && op2->kind == OP_ADDRESS)
            {
                // a[i] = b[j];
                tmp = newTmp();
                genInterCode(IR_READ_ADDR, tmp, op2);
                genInterCode(IR_WRITE_ADDR, op1, tmp);
            }
            else if (op1->kind == OP_ADDRESS)
            {
                // a[i] = x;
                genInterCode(IR_WRITE_ADDR, op1, op2);
            }
            else if (op2->kind == OP_ADDRESS)
            {
                // x = a[i];
                genInterCode(IR_READ_ADDR, op1, op2);
            }
            else
                assert(0);
        }
        else
        {
            // x = y;
            newCode = newInterCodes(newInterCode(kind, op1, op2));
            addInterCode(interCodeList, newCode);
        }
        break;
    case IR_ADD: // binOp
    case IR_SUB:
    case IR_MUL:
    case IR_DIV:
        result = va_arg(vaList, pOperand);
        op1 = va_arg(vaList, pOperand);
        op2 = va_arg(vaList, pOperand);
        assert(result && op1 && op2);
        if (op1->kind == OP_ADDRESS)
        {
            tmp = newTmp();
            genInterCode(IR_READ_ADDR, tmp, op1);
            op1 = tmp;
        }
        if (op2->kind == OP_ADDRESS)
        {
            tmp = newTmp();
            genInterCode(IR_READ_ADDR, tmp, op2);
            op2 = tmp;
        }
        assert(op1 && op2);
        newCode = newInterCodes(newInterCode(kind, result, op1, op2));
        addInterCode(interCodeList, newCode);
        break;
    case IR_ADD_ADDR:
        result = va_arg(vaList, pOperand);
        op1 = va_arg(vaList, pOperand);
        op2 = va_arg(vaList, pOperand);
        assert(result && op1 && op2);
        newCode = newInterCodes(newInterCode(kind, result, op1, op2));
        addInterCode(interCodeList, newCode);
        break;
    case IR_IF_GOTO: // ifGoTo
        result = va_arg(vaList, pOperand);
        relop = va_arg(vaList, pOperand);
        op1 = va_arg(vaList, pOperand);
        op2 = va_arg(vaList, pOperand);
        assert(result && op1 && op2 && relop);
        newCode = newInterCodes(newInterCode(kind, result, relop, op1, op2));
        addInterCode(interCodeList, newCode);
        break;
    case IR_DEC: // dec, for function call
        op1 = va_arg(vaList, pOperand);
        size = va_arg(vaList, int);
        assert(size && op1);
        newCode = newInterCodes(newInterCode(kind, op1, size));
        addInterCode(interCodeList, newCode);
        break;
    default:
        assert(0);
    }
}

// Construction of inter code on the semantic tree
void translateExtDefList(pNode node)
{
    /*
    ExtDefList:     e
        |       ExtDef ExtDefList
    */
    assert(node != nullptr);
    assert(!strcmp(node->name, "ExtDefList"));
    debug("translateExtDefList\n");
    pNode child = node->children;
    translateExtDef(child);
    if (child->next != nullptr)
        translateExtDefList(child->next);
}

void translateExtDef(pNode node)
{
    /*
    ExtDef:     Specifier ExtDecList SEMI
        |       Specifier SEMI
        |       Specifier FunDec CompSt
        |       Specifier FunDec SEMI
    */
    assert(node != nullptr);
    assert(!strcmp(node->name, "ExtDef"));
    debug("translateExtDef\n");
    // Since there is no global variable, we only care about "ExtDef -> Specifier FunDec CompSt"
    if (!strcmp(node->children->next->name, "FunDec"))
    {
        translateFunDec(node->children->next);
        translateCompSt(node->children->next->next);
    }
}

void translateFunDec(pNode node)
{
    assert(node != nullptr);
    assert(!strcmp(node->name, "FunDec"));
    debug("translateFuncDec\n");
    /*
    FunDec:     ID LP VarList RP
        |       ID LP RP
    */
    pItem item = searchFirstTableItem(table, node->children->val);
    assert(item != nullptr);
    assert(item->icname == nullptr);
    if (!strcmp(node->children->val, "main"))
    {
        item->icname = newString("main");
    }
    else
    {
        item->icname = catString("f_", node->children->val);
    }
    genInterCode(IR_FUNCTION, newOperand(OP_FUNCTION, newString(item->icname)));
    pItem funcItem = searchFirstTableItem(table, node->children->val);
    assert(funcItem != nullptr);
    pFieldList tmp = funcItem->field->type->u.func.argv;
    while (tmp != nullptr)
    {
        item = searchFirstTableItem(table, tmp->name);
        assert(item != nullptr);
        assert(item->icname == nullptr);
        item->icname = catString("v_", tmp->name);
        pOperand param = newOperand(OP_VARIABLE, newString(item->icname));
        genInterCode(IR_PARAM, param);
        tmp = tmp->tail;
    }
}

void translateCompSt(pNode node)
{
    assert(node != nullptr);
    assert(!strcmp(node->name, "CompSt"));
    debug("translateCompSt\n");
    /*
    CompSt:    LC DefList StmtList RC
    */
    pNode child = node->children->next;
    if (!strcmp(child->name, "DefList"))
    {
        translateDefList(child);
        child = child->next;
    }
    if (!strcmp(child->name, "StmtList"))
    {
        translateStmtList(child);
    }
}

void translateDefList(pNode node)
{
    assert(node != nullptr);
    assert(!strcmp(node->name, "DefList"));
    debug("translateDefList\n");
    /*
    DefList:    Def DefList
            |   e
    */
    pNode child = node->children;
    if (child != nullptr)
    {
        translateDef(child);
        if (child->next != nullptr)
            translateDefList(child->next);
    }
}

void translateDef(pNode node)
{
    assert(node != nullptr);
    assert(!strcmp(node->name, "Def"));
    debug("translateDef\n");
    /*
    Def:            Specifier DecList SEMI
    */
    translateDecList(node->children->next);
}

void translateDecList(pNode node)
{
    /*
    DecList:        Dec
            |       Dec COMMA DecList
    */
    assert(node != nullptr);
    assert(!strcmp(node->name, "DecList"));
    debug("translateDecList\n");
    pNode child = node->children;
    translateDec(child);
    if (child->next != nullptr)
    {
        translateDecList(child->next->next);
    }
}

void translateDec(pNode node)
{
    assert(node != nullptr);
    assert(!strcmp(node->name, "Dec"));
    debug("translateDec\n");
    /*
    Dec:            VarDec
            |       VarDec ASSIGNOP Exp
    */
    pNode child = node->children;
    if (child->next == nullptr)
    {
        // Dec -> VarDec
        translateVarDec(child, nullptr);
    }
    else
    {
        // Dec -> VarDec ASSIGNOP Exp
        pOperand t1 = newTmp();
        translateVarDec(child, t1);
        pOperand t2 = newTmp();
        translateExp(child->next->next, t2);
        genInterCode(IR_ASSIGN, t1, t2);
    }
}

void translateVarDec(pNode node, pOperand place)
{
    assert(node != nullptr);
    assert(!strcmp(node->name, "VarDec"));
    debug("translateVarDec\n");
    /*
    VarDec:         ID
            |       VarDec LB INT RB
    */
    pNode child = node->children;
    if (!strcmp(child->name, "ID"))
    {
        // VarDec -> ID
        pItem item = searchFirstTableItem(table, child->val);
        assert(item != nullptr);
        assert(item->icname == nullptr);
        item->icname = catString("t_", child->val);
        pType type = item->field->type;
        if (type->kind == BASIC)
        {
            if (place)
            {
                assert(place->u.name != nullptr);
                interCodeList->tmpVarNum -= 1;
                setOperand(place, OP_VARIABLE, newString(item->icname));
            }
        }
        else if (type->kind == ARRAY || type->kind == STRUCTURE)
        {
            genInterCode(IR_DEC,
                         newOperand(OP_VARIABLE, newString(item->icname)),
                         getSize(type));
        }
        else
        {
            assert(0);
        }
    }
    else
    {
        // VarDec -> VarDec LB INT RB
        translateVarDec(child, place);
    }
}

void translateStmtList(pNode node)
{
    assert(node != nullptr);
    assert(!strcmp(node->name, "StmtList"));
    debug("translateStmtList\n");
    /*
    StmtList:       e
            |       Stmt StmtList
    */
    pNode child = node->children;
    if (child != nullptr)
    {
        translateStmt(child);
        if (child->next != nullptr)
            translateStmtList(child->next);
    }
}

void translateStmt(pNode node)
{
    assert(node != nullptr);
    assert(!strcmp(node->name, "Stmt"));
    debug("translateStmt\n");
    /*
    Stmt:           Exp SEMI
            |       CompSt
            |       RETURN Exp SEMI
            |       RETURN SEMI
            |       IF LP Exp RP Stmt %prec LOWER_THAN_ELSE
            |       IF LP Exp RP Stmt ELSE Stmt
            |       WHILE LP Exp RP Stmt
    */
    pNode child = node->children;
    // Stmt -> Exp SEMI
    if (!strcmp(child->name, "Exp"))
    {
        translateExp(child, nullptr);
    }
    // Stmt -> Compt
    else if (!strcmp(child->name, "CompSt"))
    {
        translateCompSt(child);
    }
    // Stmt -> RETURN Exp SEMI || Stmt -> RETURN SEMI
    else if (!strcmp(child->name, "RETURN"))
    {
        pNode exp = child->next;
        if (!strcmp(exp->name, "Exp"))
        {
            pOperand t1 = newTmp();
            translateExp(exp, t1);
            genInterCode(IR_RETURN, t1);
        }
    }
    // Stmt -> IF LP Exp RP Stmt
    // Stmt -> IF LP Exp RP Stmt ELSE Stmt
    else if (!strcmp(child->name, "IF"))
    {
        pNode exp = child->next->next;
        pNode stmt = exp->next->next;
        pOperand label1 = newLabel();
        pOperand label2 = newLabel();
        translateCond(exp, label1, label2);
        genInterCode(IR_LABEL, label1);
        translateStmt(stmt);
        // Stmt -> IF LP Exp RP Stmt
        if (stmt->next == nullptr)
        {
            genInterCode(IR_LABEL, label2);
        }
        else
        {
            pOperand label3 = newLabel();
            genInterCode(IR_GOTO, label3);
            genInterCode(IR_LABEL, label2);
            translateStmt(stmt->next->next);
            genInterCode(IR_LABEL, label3);
        }
    }
    // Stmt -> WHILE LP Exp RP Stmt
    else if (!strcmp(child->name, "WHILE"))
    {
        pOperand label1 = newLabel();
        pOperand label2 = newLabel();
        pOperand label3 = newLabel();
        pNode exp = child->next->next;
        genInterCode(IR_LABEL, label1);
        translateCond(exp, label2, label3);
        genInterCode(IR_LABEL, label2);
        translateStmt(exp->next->next);
        genInterCode(IR_GOTO, label1);
        genInterCode(IR_LABEL, label3);
    }
    else
    {
        assert(0);
    }
}

void translateExp(pNode node, pOperand place)
{
    assert(node != nullptr);
    assert(!strcmp(node->name, "Exp"));
    debug("translateExp\n");
    /*
    Exp:            LP Exp RP
            |       NOT Exp
            |       Exp AND Exp
            |       Exp OR Exp
            |       Exp RELOP Exp
            |       Exp ASSIGNOP Exp
            |       Exp PLUS Exp
            |       Exp MINUS Exp
            |       Exp STAR Exp
            |       Exp DIV Exp
            |       Exp LB Exp RB
            |       Exp DOT ID
            |       MINUS Exp
            |       ID LP Args RP
            |       ID LP RP
            |       ID
            |       INT
            |       FLOAT   // In this stage float is not considered.
    */
    pNode child = node->children;
    // Exp -> LP Exp RP
    if (!strcmp(child->name, "LP"))
    {
        debug("\tExp -> LP Exp RP\n");
        translateExp(child->next, place);
    }
    // Exp -> Exp ...
    else if (!strcmp(child->name, "Exp") ||
             !strcmp(child->name, "NOT"))
    {
        pNode op = child->next;
        // Exp -> Exp AND Exp
        // Exp -> Exp OR ID
        // Exp -> Exp RELOP Exp
        // Exp -> NOT Exp
        // For boolean value
        if (!strcmp(child->name, "NOT") ||
            !strcmp(op->name, "AND") ||
            !strcmp(op->name, "OR") ||
            !strcmp(op->name, "RELOP"))
        {
            if (place == nullptr)
                return;
            debug("\tExp -> Exp <bool> Exp\n");
            pOperand label1 = newLabel();
            pOperand label2 = newLabel();
            pOperand trueNum = newOperand(OP_CONSTANT, 1);
            pOperand falseNum = newOperand(OP_CONSTANT, 0);
            genInterCode(IR_ASSIGN, place, falseNum);
            translateCond(node, label1, label2);
            genInterCode(IR_LABEL, label1);
            genInterCode(IR_ASSIGN, place, trueNum);
            genInterCode(IR_LABEL, label2);
        }
        // Exp -> Exp ASSIGNOP Exp
        else if (!strcmp(op->name, "ASSIGNOP"))
        {
            debug("\tExp -> Exp ASSIGNOP Exp\n");
            pOperand t2 = newTmp();
            translateExp(op->next, t2);
            pOperand t1 = newTmp();
            translateExp(child, t1);
            genInterCode(IR_ASSIGN, t1, t2);
        }
        // Exp -> Exp PLUS Exp
        // Exp -> Exp MINUS Exp
        // Exp -> Exp STAR Exp
        // Exp -> Exp DIV Exp
        else if (!strcmp(op->name, "PLUS") ||
                 !strcmp(op->name, "MINUS") ||
                 !strcmp(op->name, "STAR") ||
                 !strcmp(op->name, "DIV"))
        {
            debug("\tExp -> Exp <cal> Exp\n");
            if (place == nullptr)
                return;
            pOperand t2 = newTmp();
            translateExp(op->next, t2);
            pOperand t1 = newTmp();
            translateExp(child, t1);
            if (!strcmp(op->name, "PLUS"))
            {
                genInterCode(IR_ADD, place, t1, t2);
            }
            else if (!strcmp(op->name, "MINUS"))
            {
                genInterCode(IR_SUB, place, t1, t2);
            }
            else if (!strcmp(op->name, "STAR"))
            {
                genInterCode(IR_MUL, place, t1, t2);
            }
            else
            {
                genInterCode(IR_DIV, place, t1, t2);
            }
        }
        // Exp -> Exp1 DOT ID
        else if (!strcmp(op->name, "DOT"))
        {
            if (place == nullptr)
                return;
            debug("\tExp -> Exp DOT ID\n");
            pOperand tmp = newTmp();
            translateExp(child, tmp);
            pOperand target = nullptr; // target should be the struct beginning address.
            if (tmp->kind == OP_ADDRESS)
            {
                // If Exp1 is struct in array or nesting strcut or struct argument, tmp will be just address
                target = newOperand(tmp->kind, newString(tmp->u.name));
            }
            else
            {
                // Otherwise need to get address first.
                target = newTmp();
                genInterCode(IR_GET_ADDR, target, tmp);
            }
            char *idname = op->next->val;
            pOperand id = newTmp();
            int offset = 0;
            // The tmp->u.name should be t_<id_name> or v_<param_name>
            pItem structItem = strlen(tmp->u.name) > 2 ? searchFirstTableItem(table, tmp->u.name + 2) : nullptr;
            pType structType = structItem == nullptr ? getElement(interCodeList->lastArrayElem) : getElement(structItem->field->type);
            pFieldList ptr = structType->u.structure.field;
            while (ptr)
            {
                if (!strcmp(ptr->name, idname))
                    break;
                offset += getSize(ptr->type);
                ptr = ptr->tail;
            }
            pOperand toffset = newOperand(OP_CONSTANT, offset);
            genInterCode(IR_ADD_ADDR, place, target, toffset);
            setOperand(place, OP_ADDRESS, newString(id->u.name));
            if(ptr->type->kind == ARRAY){
                place->elemType = ptr->type->u.array.elem;
            }
        }
        // Exp -> Exp LB Exp RB
        else if (!strcmp(op->name, "LB"))
        {
            if (place == nullptr)
                return;
            debug("\tExp -> Exp LB Exp RB\n");
            pOperand idx = newTmp();
            translateExp(op->next, idx);
            pOperand base = newTmp();
            char *oldBaseName = newString(base->u.name);
            translateExp(child, base);
            pOperand width = nullptr;
            pOperand offset = newTmp();
            pOperand target = nullptr;
            if(base->elemType == nullptr){
                printf("%s\n", base->u.name);
            }
            assert(base->elemType != nullptr);
            width = newOperand(OP_CONSTANT, getSize(base->elemType));
            genInterCode(IR_MUL, offset, idx, width);
            if (base->kind == OP_VARIABLE)
            {
                // ID[Exp]
                target = newTmp();
                genInterCode(IR_GET_ADDR, target, base);
            }
            else
            {
                // Exp.ID[Exp], ID[Exp][Exp]
                assert(base->kind == OP_ADDRESS);
                target = base;
            }
            genInterCode(IR_ADD_ADDR, place, target, offset);
            place->kind = OP_ADDRESS;
            if (base->elemType->kind == ARRAY)
                setElemType(place, base->elemType->u.array.elem);
            if (strcmp(base->u.name, oldBaseName))
            {
                interCodeList->lastArrayElem = base->elemType;
            }
            free(oldBaseName);
            oldBaseName = nullptr;
        }
        else
        {
            assert(0);
        }
    }
    // Exp -> MINUS Exp
    else if (!strcmp(child->name, "MINUS"))
    {
        if (place == nullptr)
            return;
        debug("\tExp -> MINUS\n");
        pOperand t1 = newTmp();
        translateExp(child->next, t1);
        pOperand zero = newOperand(OP_CONSTANT, 0);
        genInterCode(IR_SUB, place, zero, t1);
    }
    // Exp -> ID LP Args RP
    //      | ID LP RP
    else if (!strcmp(child->name, "ID") && child->next != nullptr)
    {
        debug("\tExp -> ID LP <...> RP\n");
        pItem item = searchFirstTableItem(table, child->val);
        assert(item != nullptr);
        assert(item->icname != nullptr);
        pOperand funcTmp = newOperand(OP_FUNCTION, newString(item->icname));
        // Exp -> ID LP Args RP
        if (!strcmp(child->next->next->name, "Args"))
        {
            pArgList argList = newArgList();
            translateArgs(child->next->next, argList);
            if (!strcmp(child->val, "write"))
            {
                genInterCode(IR_WRITE, argList->head->op);
            }
            else
            {
                pArg argTmp = argList->head;
                while (argTmp != nullptr)
                {
                    if (argTmp->op->kind == OP_VARIABLE)
                    {
                        pItem item = searchFirstTableItem(table, argTmp->op->u.name);
                        if (item &&
                            (item->field->type->kind == STRUCTURE || item->field->type->kind == ARRAY))
                        { // Parameters of array and structure as address
                            pOperand varTmp = newTmp();
                            genInterCode(IR_GET_ADDR, varTmp, argTmp->op);
                            pOperand varTmpCopy = newOperand(OP_ADDRESS, varTmp->u.name);
                            genInterCode(IR_ARG_ADDR, varTmpCopy);
                        }
                        else
                        {
                            genInterCode(IR_ARG, argTmp->op);
                        }
                    }
                    else if (argTmp->op->kind == OP_ADDRESS)
                    {
                        genInterCode(IR_ARG_ADDR, argTmp->op);
                    }
                    else
                    {
                        genInterCode(IR_ARG, argTmp->op);
                    }
                    argTmp = argTmp->next;
                }
                if (place)
                {
                    genInterCode(IR_CALL, place, funcTmp);
                }
                else
                {
                    pOperand tmp = newTmp();
                    genInterCode(IR_CALL, tmp, funcTmp);
                }
            }
        }
        // Exp -> ID LP RP
        else
        {
            if (!strcmp(child->val, "read"))
            {
                genInterCode(IR_READ, place);
            }
            else
            {
                if (place)
                {
                    genInterCode(IR_CALL, place, funcTmp);
                }
                else
                {
                    pOperand tmp = newTmp();
                    genInterCode(IR_CALL, tmp, funcTmp);
                }
            }
        }
    }
    // Exp -> ID
    else if (!strcmp(child->name, "ID"))
    {
        debug("\tExp -> ID\n");
        if (place == nullptr)
            return;
        pItem item = searchFirstTableItem(table, child->val);
        assert(item != nullptr);
        // Before the reduction that Exp -> ID, place value should be a tmp value.
        interCodeList->tmpVarNum -= 1;
        assert(item->icname != nullptr);
        // Do inter-code translation after semantic check, so ID must pre-exit.
        if (item->field->isArg &&
            (item->field->type->kind == STRUCTURE || item->field->type->kind == ARRAY))
        {
            setOperand(place, OP_ADDRESS, newString(item->icname));
        }
        else
        {
            setOperand(place, OP_VARIABLE, newString(item->icname));
        }
        if (item->field->type->kind == ARRAY)
        {
            setElemType(place, item->field->type->u.array.elem);
        }
        // printf("%s\n", place->u.name);
    }
    // Exp -> INT
    else if (!strcmp(child->name, "INT"))
    {
        debug("\tExp -> INT\n");
        if (place == nullptr)
            return;
        setOperand(place, OP_CONSTANT, atoi(child->val));
    }
    else
    {
        // Exception, should not reach here.
        assert(0);
    }
}

void translateCond(pNode node, pOperand labelTrue, pOperand labelFalse)
{
    assert(node != nullptr);
    assert(labelTrue != nullptr);
    assert(labelFalse != nullptr);
    assert(!strcmp(node->name, "Exp"));
    debug("translateCond\n");
    /*
    Exp -> Exp AND Exp
          | Exp OR Exp
          | Exp RELOP Exp
          | NOT Exp
    */
    /*
    trnslate condition control
    */
    pNode child = node->children;
    // Exp -> NOT Exp
    assert(child != nullptr);
    if (!strcmp(child->name, "NOT"))
    {
        debug("\tNOT\n");
        translateCond(child->next, labelFalse, labelTrue);
    }
    // Exp -> Exp RELOP Exp
    else if (child->next != nullptr && !strcmp(child->next->name, "RELOP"))
    {
        debug("\tRELOP\n");
        pOperand t1 = newTmp();
        pOperand t2 = newTmp();
        translateExp(child, t1);
        translateExp(child->next->next, t2);
        pOperand relop = newOperand(OP_RELOP, newString(child->next->val));
        if (t1->kind == OP_ADDRESS)
        {
            pOperand tmp = newTmp();
            genInterCode(IR_READ_ADDR, tmp, t1);
            t1 = tmp;
        }
        if (t2->kind == OP_ADDRESS)
        {
            pOperand tmp = newTmp();
            genInterCode(IR_READ_ADDR, tmp, t2);
            t2 = tmp;
        }
        genInterCode(IR_IF_GOTO, t1, relop, t2, labelTrue);
        genInterCode(IR_GOTO, labelFalse);
    }
    // Exp -> Exp AND Exp
    else if (child->next != nullptr && !strcmp(child->next->name, "AND"))
    {
        debug("\tAND\n");
        pOperand label1 = newLabel();
        translateCond(child, label1, labelFalse);
        genInterCode(IR_LABEL, label1);
        translateCond(child->next->next, labelTrue, labelFalse);
    }
    // Exp -> Exp OR Exp
    else if (child->next != nullptr && !strcmp(child->next->name, "OR"))
    {
        debug("\tOR\n");
        pOperand label1 = newLabel();
        translateCond(child, labelTrue, label1);
        genInterCode(IR_LABEL, label1);
        translateCond(child->next->next, labelTrue, labelFalse);
    }
    // other cases
    else
    {
        debug("\toter class\n");
        pOperand t1 = newTmp();
        translateExp(node, t1);
        pOperand t2 = newOperand(OP_CONSTANT, 0);
        pOperand relop = newOperand(OP_RELOP, newString("!="));
        if (t1->kind == OP_ADDRESS)
        {
            pOperand tmp = newTmp();
            genInterCode(IR_READ_ADDR, tmp, t1);
            t1 = tmp;
        }
        genInterCode(IR_IF_GOTO, t1, relop, t2, labelTrue);
        genInterCode(IR_GOTO, labelFalse);
    }
}

void translateArgs(pNode node, pArgList argList)
{
    assert(node != nullptr);
    assert(!strcmp(node->name, "Args"));
    debug("translateArgs\n");
    /*
    Args -> Exp COMMA Args
          | Exp
    */
    pNode child = node->children;
    // The sequence of args are inversed.
    if (child->next != nullptr)
    {
        translateArgs(child->next->next, argList);
    }
    //    Args -> Exp
    pArg tmp = newArg(newTmp());
    translateExp(child, tmp->op);
    addArg(argList, tmp);
}
