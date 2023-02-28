#include "node.h"
#include "type.h"
#include "semantic.h"
#include "string.h"

extern pTable table;

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

// Type functions
pType newType(Kind kind, ...)
{
    assert(kind == BASIC || kind == ARRAY || kind == FUNC || kind == STRUCTURE);
    pType p = (pType)malloc(sizeof(Type));
    assert(p != nullptr);
    p->kind = kind;
    va_list valist;
    if (kind == BASIC)
    {
        va_start(valist, kind);
        p->u.basic = va_arg(valist, BasicType);
    }
    else if (kind == ARRAY)
    {
        va_start(valist, kind);
        p->u.array.size = va_arg(valist, int);
        p->u.array.elem = va_arg(valist, pType);
    }
    else if (kind == STRUCTURE)
    {
        va_start(valist, kind);
        p->u.structure.structName = va_arg(valist, char *);
        p->u.structure.field = va_arg(valist, pFieldList);
    }
    else
    {
        va_start(valist, kind);
        p->u.func.state = va_arg(valist, FuncState);
        p->u.func.argc = va_arg(valist, int);
        p->u.func.argv = va_arg(valist, pFieldList);
        p->u.func.returnType = va_arg(valist, pType);
        // Only for declared-but-not-defined function.
        p->u.func.lineno = va_arg(valist, int);
    }
    return p;
}

pType copyType(pType src)
{
    Kind kind = src->kind;
    if (kind == BASIC)
        return newType(kind, src->u.basic);
    else if (kind == ARRAY)
        return newType(kind, src->u.array.size, copyType(src->u.array.elem));
    else if (kind == STRUCTURE)
        return newType(kind, newString(src->u.structure.structName), copyFieldList(src->u.structure.field));
    else
        return newType(kind, src->u.func.state, src->u.func.argc, copyFieldList(src->u.func.argv), copyType(src->u.func.returnType), src->u.func.lineno);
}

void deleteType(pType type)
{
    assert(type != nullptr);
    Kind kind = type->kind;
    assert(kind == BASIC || kind == ARRAY || kind == FUNC || kind == STRUCTURE);
    if (kind == ARRAY)
    {
        if (type->u.array.elem != nullptr)
            deleteType(type->u.array.elem);
        type->u.array.elem = nullptr;
    }
    else if (kind == STRUCTURE)
    {
        if (type->u.structure.structName != nullptr)
        {
            free(type->u.structure.structName);
            type->u.structure.structName = nullptr;
        }
        if (type->u.structure.field)
            deleteFieldList(type->u.structure.field);
        type->u.structure.field = nullptr;
    }
    else if (kind == FUNC)
    {
        if (type->u.func.argc)
            deleteFieldList(type->u.func.argv);
        type->u.func.argv = nullptr;
        if (type->u.func.returnType != nullptr)
            deleteType(type->u.func.returnType);
        type->u.func.returnType = nullptr;
    }
    free(type);
}

boolean checkType(pType type1, pType type2)
{
    if (type1 == nullptr && type2 == nullptr)
        return true;
    if (type1 == nullptr)
        return (type2->kind == BASIC && type2->u.basic == voidType);
    if (type2 == nullptr)
        return (type1->kind == BASIC && type1->u.basic == voidType);
    if (type1->kind == FUNC && type2->kind == FUNC)
    {
        if (type1->u.func.state == defined && type2->u.func.state == defined)
        {
            return false;
        }
        else
        {
            if (type1->u.func.argc != type2->u.func.argc)
                return false;
            if (!checkFieldList(type1->u.func.argv, type2->u.func.argv))
                return false;
            if (!checkType(type1->u.func.returnType, type2->u.func.returnType))
                return false;
            return true;
        }
    }
    if (type1->kind != type2->kind)
        return false;
    else
    {
        assert(type1->kind == BASIC || type1->kind == ARRAY || type1->kind == STRUCTURE);
        if (type1->kind == BASIC)
        {
            return type1->u.basic == type2->u.basic;
        }
        else if (type1->kind == ARRAY)
        {
            return checkType(type1->u.array.elem, type2->u.array.elem);
        }
        else
        {
#ifndef STRUCTURE_EQUIVALENT
            return !strcmp(type1->u.structure.structName,
                           type2->u.structure.structName);
#else
            pFieldList mem1 = type1->u.structure.field, mem2 = type2->u.structure.field;
            while (mem1 != nullptr && mem2 != nullptr)
            {
                if (!checkType(mem1->type, mem2->type))
                {
                    return false;
                }
                mem1 = mem1->tail;
                mem2 = mem2->tail;
            }
            if (mem1 != mem2)
            { // Should both be nullptr
                return false;
            }
            else
            {
                return true;
            }
#endif
        }
    }
}

// For debug
void printType(pType type)
{
    assert(0);
}

// FieldList functions
pFieldList newFieldList(char *newName, pType newType)
{
    pFieldList p = (pFieldList)malloc(sizeof(FieldList));
    assert(p != nullptr);
    p->name = newName;
    p->type = newType;
    p->tail = nullptr;
    return p;
}

pFieldList copyFieldList(pFieldList src)
{
    if (src == nullptr)
        return nullptr;
    pFieldList p = newFieldList(newString(src->name), copyType(src->type));
    p->isArg = src->isArg;
    if (src->tail != nullptr)
        p->tail = copyFieldList(src->tail);
    return p;
}

void deleteFieldList(pFieldList fieldList)
{
    assert(fieldList != nullptr);
    if (fieldList->tail != nullptr)
    {
        deleteFieldList(fieldList->tail);
        fieldList->tail = nullptr;
    }
    if (fieldList->name != nullptr)
    {
        free(fieldList->name);
        fieldList->name = nullptr;
    }
    if (fieldList->type != nullptr)
    {
        deleteType(fieldList->type);
        fieldList->type = nullptr;
    }
    free(fieldList);
    fieldList = nullptr;
}

void setFieldListName(pFieldList p, char *newName)
{
    assert(p != nullptr);
    if (p->name)
        free(p->name);
    p->name = newName;
}

boolean checkFieldList(pFieldList list1, pFieldList list2)
{
    if (list1 == nullptr && list2 == nullptr)
        return true;
    else if (list1 == nullptr || list2 == nullptr)
        return false;
    else if (checkType(list1->type, list2->type))
        return checkFieldList(list1->tail, list2->tail);
    else
        return false;
}

void printFieldList(pFieldList fieldList)
{
    assert(0);
}

// tableItem functions
pItem newItem(int symbolDepth, pFieldList pfield)
{
    pItem p = (pItem)malloc(sizeof(TableItem));
    assert(p != nullptr);
    p->icname = nullptr;
    p->symbolDepth = symbolDepth;
    p->field = pfield;
    p->nextHash = p->prevHash = p->nextSymbol = nullptr;
    return p;
}

void deleteItem(pItem item)
{
    assert(item != nullptr);
    if (item->field != nullptr)
        deleteFieldList(item->field);
    item->field = nullptr;
    item->nextHash = item->nextSymbol = item->prevHash = item->prevSymbol = nullptr;
    item->symbolDepth = 0;
    if (item->icname != nullptr)
    {
        free(item->icname);
        item->icname = nullptr;
    }
    free(item);
}

boolean isStructDef(pItem src)
{
    assert(src != nullptr);
    return (src->field->type->kind == STRUCTURE) && (src->field->type->u.structure.structName == nullptr);
}

// Hash functions
pHash newHash()
{
    pHash p = (pHash)malloc(sizeof(HashTable));
    assert(p != nullptr);
    p->hashArray = (pItem *)malloc(HASH_TABLE_SIZE * sizeof(pItem));
    assert(p->hashArray != nullptr);
    for (int i = 0; i < HASH_TABLE_SIZE; i++)
    {
        p->hashArray[i] = nullptr;
    }
    return p;
}

void deleteHash(pHash hash)
{
    assert(hash != nullptr);
    for (int i = 0; i < HASH_TABLE_SIZE; i++)
    {
        while (hash->hashArray[i] != nullptr)
        {
            pItem tmp = hash->hashArray[i];
            hash->hashArray[i] = tmp->nextHash;
            deleteItem(tmp);
            tmp = nullptr;
        }
        hash->hashArray[i] = nullptr;
    }
    free(hash->hashArray);
    hash->hashArray = nullptr;
    free(hash);
}

pItem getHashHead(pHash hash, int index)
{
    assert(hash != nullptr);
    assert(index >= 0 && index < HASH_TABLE_SIZE);
    return hash->hashArray[index];
}

void setHashHead(pHash hash, int index, pItem newVal)
{
    assert(hash != nullptr);
    assert(newVal != nullptr);
    pItem prev = getHashHead(hash, index);
    hash->hashArray[index] = newVal;
    newVal->nextHash = prev;
    if (prev != nullptr)
    {
        newVal->prevHash = prev->prevHash;
        prev->prevHash = newVal;
    }
}

// Carefully use!
void setHashHeadEmpty(pHash hash, int index)
{
    assert(hash != nullptr);
    assert(hash != nullptr);
    assert(index >= 0 && index < HASH_TABLE_SIZE);
    hash->hashArray[index] = nullptr;
}

// Stack Function
pStack newStack()
{
    pStack p = (pStack)malloc(sizeof(Stack));
    assert(p != nullptr);
    p->stackArray = (pItem *)malloc(HASH_TABLE_SIZE * sizeof(pItem));
    assert(p->stackArray != nullptr);
    for (int i = 0; i < HASH_TABLE_SIZE; i++)
    {
        p->stackArray[i] = nullptr;
    }
    return p;
}

void deleteStack(pStack stack)
{
    assert(stack != nullptr);
    free(stack->stackArray);
    stack->stackArray = nullptr;
    stack->curStackDepth = 0;
    free(stack);
}

void addStackDepth(pStack stack)
{
    assert(stack != nullptr);
    assert(stack->curStackDepth + 1 < HASH_TABLE_SIZE);
    stack->curStackDepth += 1;
}

void minusStackDepth(pStack stack)
{
    assert(stack != nullptr);
    assert(stack->curStackDepth - 1 >= 0);
    stack->curStackDepth -= 1;
}

pItem getCurDepthStackHead(pStack stack)
{
    assert(stack != nullptr);
    return stack->stackArray[stack->curStackDepth];
}

void setCurDepthStackHead(pStack stack, pItem newVal)
{
    assert(stack != nullptr);
    assert(newVal != nullptr);
    pItem prev = getCurDepthStackHead(stack);
    stack->stackArray[stack->curStackDepth] = newVal;
    newVal->nextSymbol = prev;
    if (prev != nullptr)
    {
        newVal->prevSymbol = prev->prevSymbol;
        prev->prevSymbol = newVal;
    }
    else
        assert(newVal->prevSymbol == nullptr);
}

// Carefully use!
void setCurDepthStackHeadEmpty(pStack stack)
{
    assert(stack != nullptr);
    stack->stackArray[stack->curStackDepth] = nullptr;
}

// Table functions
pTable initTable()
{
    pTable p = (pTable)malloc(sizeof(Table));
    assert(p != nullptr);
    p->hash = newHash();
    p->stack = newStack();
    p->unNamedStructNum = 0;
    pItem readFunc = newItem(0,
                             newFieldList(newString("read"),
                                          newType(FUNC, defined, 0, nullptr, newType(BASIC, intType))));
    readFunc->icname = newString("read");
    pItem writeFunc = newItem(0,
                              newFieldList(newString("write"),
                                           newType(FUNC, defined, 1,
                                                   newFieldList(newString("arg1"), newType(BASIC, intType)),
                                                   newType(BASIC, intType))));
    writeFunc->icname = newString("write");
    addTableItem(p, readFunc);
    addTableItem(p, writeFunc);
    return p;
}

void deleteTable(pTable table)
{
    // Before delete table, check whether there is not-defined function.
    pItem ptr = nullptr;
    for (int i = 0; i < HASH_TABLE_SIZE; i++)
    {
        ptr = getHashHead(table->hash, i);
        while (ptr != nullptr)
        {
            if (ptr->field->type->kind == FUNC && ptr->field->type->u.func.state == declared)
            {
                // The function is noly declared but not defined.
                char errorMsg[ERROR_MSG_SIZE];
                sprintf(errorMsg,
                        "The function \"%s\" is only declared but never defined.",
                        ptr->field->name);
                pError(only_declare_func, ptr->field->type->u.func.lineno, errorMsg);
            }
            ptr = ptr->nextHash;
        }
    }

    // delete Hash and Stack
    assert(table != nullptr);
    deleteHash(table->hash);
    deleteStack(table->stack);
    free(table);
    table->hash = nullptr;
    table->stack = nullptr;
    table->unNamedStructNum = 0;
}

pItem searchFirstTableItem(pTable table, char *name)
{
    assert(table != nullptr);
    assert(name != nullptr);
    unsigned idx = getHashCode(name);
    pItem p = getHashHead(table->hash, idx);
    while (p != nullptr && strcmp(p->field->name, name))
        p = p->nextHash;
    return p;
}

boolean checkTableItemConflict(pTable table, pItem item)
{
    assert(table != nullptr);
    assert(item != nullptr);
    /*Nesting function definition and declareation are not allowed.*/
    assert(!(table->stack->curStackDepth != 0 && item->field->type->kind == FUNC));
    pItem prev = searchFirstTableItem(table, item->field->name);
    if (prev == nullptr)
        return false;
    while (prev != nullptr)
    {
        if (!strcmp(prev->field->name, item->field->name))
        {
            if (prev->field->type->kind == STRUCTURE ||
                item->field->type->kind == STRUCTURE)
            { // The struct can always be defined once, on matter which field the def is in.
                return true;
            }
            // The check of two functions, don't use this function, specifically judged in FunDec instead.
            if (prev->symbolDepth == item->symbolDepth)
                return true;
        }
        prev = prev->nextHash;
    }
    return false;
}

void addTableItem(pTable table, pItem item)
{
    assert(table != nullptr);
    assert(item != nullptr);
    unsigned idx = getHashCode(item->field->name);
    setHashHead(table->hash, idx, item);
    setCurDepthStackHead(table->stack, item);
}

// Param 'item' must previously in the table!
void deleteTableItem(pTable table, pItem item)
{
    // Hash
    pItem nextHash = item->nextHash, prevHash = item->prevHash;
    if (nextHash != nullptr)
        nextHash->prevHash = prevHash;
    if (prevHash != nullptr)
        prevHash->nextHash = nextHash;
    else
    {
        unsigned idx = getHashCode(item->field->name);
        table->hash->hashArray[idx] = nextHash;
    }
    // Stack
    pItem nextSym = item->nextSymbol, prevSym = item->prevSymbol;
    if (nextSym != nullptr)
        nextSym->prevSymbol = prevSym;
    if (prevSym != nullptr)
        prevSym->nextSymbol = nextSym;
    else
    {
        table->stack->stackArray[table->stack->curStackDepth] = nextSym;
    }
    // delete item
    deleteItem(item);
    item = nullptr;
}

void clearCurDepthStackList(pTable table)
{
    pItem head = getCurDepthStackHead(table->stack), tmp = nullptr;
    // For inter code translation, no name confliction.
    // Delete in the stack, but not in the hash.
    while (head != nullptr)
    {
        tmp = head;
        head = tmp->nextSymbol;
        tmp->nextSymbol = nullptr;
        tmp->prevSymbol = nullptr;
        tmp = nullptr;
    }
    setCurDepthStackHeadEmpty(table->stack);
}

/*For debug*/
void printTable(pTable table)
{
    assert(0);
}

// Function Declaration Check
boolean checkFunDec(pItem prev, pItem curr)
{
    assert(prev != nullptr);
    assert(curr != nullptr);
    assert(prev->field->type->kind == FUNC);
    assert(curr->field->type->kind == FUNC);
    boolean funcMatch = true;
    if (!checkType(prev->field->type->u.func.returnType, curr->field->type->u.func.returnType))
    { // Check return type
        funcMatch = false;
    }
    else if (prev->field->type->u.func.argc != curr->field->type->u.func.argc)
    { // Check function argument number
        funcMatch = false;
    }
    else
    { // Check each argument
        pFieldList prevf = prev->field->type->u.func.argv, currf = curr->field->type->u.func.argv;
        while (prevf != nullptr && currf != nullptr && funcMatch)
        {
            if (!checkFieldList(prevf, currf))
                funcMatch = false;
            prevf = prevf->tail;
            currf = currf->tail;
        }
        if (prevf != currf)
        {
            // These two should both be nullptr
            funcMatch = false;
        }
    }
    return funcMatch;
}

// Traverse the abstract syntax tree.
void traverseTree(pNode node)
{
    if (node == nullptr)
        return;
    /*
    ExtDef → Specifier ExtDecList SEMI
            | Specifier SEMI
            | Specifier FunDec CompSt
    */
    if (!strcmp(node->name, "ExtDef"))
    {
        ExtDef(node);
    }

    traverseTree(node->next);
    traverseTree(node->children);
}

// Generate symbol table functions
void ExtDef(pNode node)
{
    /*
    ExtDef:     Specifier ExtDecList SEMI
            |   Specifier SEMI
            |   Specifier FunDec CompSt
            |   Specifier FunDec SEMI
    */
    assert(node != nullptr);
    assert(!strcmp(node->name, "ExtDef"));
    pNode child = node->children;
    /*First child must be a Specifier*/
    pType specifierType = Specifier(child);
    if (specifierType == nullptr)
    {
        return;
    }
    child = child->next;
    assert(child != nullptr);
    char *name = child->name;
    if (!strcmp(name, "ExtDecList"))
    {
        // ExtDef → Specifier ExtDecList SEMI
        ExtDecList(child, specifierType);
        assert(child->next != nullptr);
    }
    else if (!strcmp(name, "FunDec"))
    {
        // FunDec return item in table, CAN'T delete it.
        pNode childnext = child->next;
        FuncState funcState = declared;
        if (!strcmp(childnext->name, "CompSt"))
        {
            funcState = defined;
        }
        pItem item = FunDec(child, specifierType, funcState);
        if (item != nullptr)
        {
            if (!strcmp(childnext->name, "CompSt"))
            {
                CompSt(childnext, item);
            }
            else if (strcmp(childnext->name, "SEMI"))
            {
                assert(0);
            }
        }
    }
    else if (strcmp(name, "SEMI"))
    {
        assert(0);
    }
    if (specifierType != nullptr)
        deleteType(specifierType);
    specifierType = nullptr;
}

void ExtDecList(pNode node, pType specifier)
{
    assert(node != nullptr);
    assert(specifier != nullptr);
    assert(!strcmp(node->name, "ExtDecList"));
    pNode child = node->children;
    /*
    ExtDecList:     VarDec
            |       VarDec COMMA ExtDecList
            ;
    */
    pItem item = VarDec(child, specifier);
    if (checkTableItemConflict(table, item))
    {
        char errorMsg[ERROR_MSG_SIZE];
        sprintf(errorMsg,
                "The variable \"%s\" has already been defined.",
                item->field->name);
        pError(redef_var, child->lineno, errorMsg);
    }
    else
    {
        addTableItem(table, item);
        child = child->next; // COMMA or empty
        if (child != nullptr)
        {
            ExtDecList(child->next, specifier);
        }
    }
}

pType Specifier(pNode node)
{
    /*
    Specifier:      TYPE
        |       StructSpecifier
        ;
    */
    assert(node != nullptr);
    assert(!strcmp(node->name, "Specifier"));
    pNode child = node->children;
    assert(child != nullptr);
    pType retType = nullptr;
    if (!strcmp(child->name, "TYPE"))
    {
        if (!strcmp(child->val, "int"))
        {
            retType = newType(BASIC, intType);
        }
        else if (!strcmp(child->val, "float"))
        {
            retType = newType(BASIC, floatType);
        }
        else if (!strcmp(child->val, "char"))
        {
            retType = newType(BASIC, charType);
        }
        else if (!strcmp(child->val, "void"))
        {
            retType = newType(BASIC, voidType);
        }
        else
        {
            assert(0);
        }
    }
    else if (!strcmp(child->name, "StructSpecifier"))
    {
        retType = StructSpecifier(child);
    }
    else
    {
        assert(0);
    }
    return retType;
}

pType StructSpecifier(pNode node)
{
    /*
    StructSpecifier:STRUCTURE OptTag LC DefList RC
        |       STRUCTURE Tag
        ;
    */
    assert(node != nullptr);
    assert(!strcmp(node->name, "StructSpecifier"));
    assert(node->children != nullptr);
    pNode child = node->children->next;
    boolean withName = false;
    pType retType = nullptr;
    assert(child != nullptr);
    if (!strcmp(child->name, "Tag"))
    { // The employment of structure.
        pNode id = child->children;
        assert(id != nullptr);
        pItem item = searchFirstTableItem(table, id->val);
        if (item == nullptr || !isStructDef(item))
        {
            char errorMsg[ERROR_MSG_SIZE];
            sprintf(errorMsg,
                    "Using undefined struct \"%s\"",
                    id->val);
            pError(undef_struct, id->lineno, errorMsg);
        }
        else
        {
            retType = newType(STRUCTURE, newString(item->field->name),
                              copyFieldList(item->field->type->u.structure.field));
        }
    }
    else if (!strcmp(child->name, "OptTag") || !strcmp(child->name, "LC"))
    { // The definition of strcture
        pItem structItem = nullptr;
        if (!strcmp(child->name, "OptTag"))
        { // struct def with Tag
            withName = true;
            // struct def with name
            pNode id = child->children;
            assert(id != nullptr);
            structItem = newItem(table->stack->curStackDepth,
                                 newFieldList(newString(id->val), newType(STRUCTURE, nullptr, nullptr)));
            child = child->next;
            assert(child);
        }
        else
        {
            // struct def without Tag
            table->unNamedStructNum += 1;
            char structName[20] = {0}; // Set a name for it by counting
            sprintf(structName, "%d", table->unNamedStructNum);
            structItem = newItem(table->stack->curStackDepth,
                                 newFieldList(newString(structName), newType(STRUCTURE, nullptr, nullptr)));
        }
        child = child->next;
        assert(child);
        addStackDepth(table->stack);
        // Go into the struct field
        if (child != nullptr && !strcmp(child->name, "DefList"))
            DefList(child, structItem);
        // Go out of the struct field
        clearCurDepthStackList(table);
        minusStackDepth(table->stack);

        if (checkTableItemConflict(table, structItem))
        {
            char errorMsg[ERROR_MSG_SIZE];
            sprintf(errorMsg,
                    "The struct \"%s\" has already been defined.",
                    structItem->field->name);
            pError(redef_struct, node->lineno, errorMsg);
        }
        else
        {
            retType = newType(STRUCTURE, newString(structItem->field->name),
                              copyFieldList(structItem->field->type->u.structure.field));
            if (withName)
            {
                addTableItem(table, structItem);
            }
            else
            {
                deleteItem(structItem);
                structItem = nullptr;
            }
        }
    }
    else
    {
        assert(0);
    }
    return retType;
}

pItem VarDec(pNode node, pType specifier)
{
    /*
    VarDec:         ID                                              {$$ = newNode(@$.first_line, NOT_A_TOKEN, "VarDec", 1, $1);}
            |       STAR ID                                         {$$ = newNode(@$.first_line, NOT_A_TOKEN, "VarDec", 2, $1, $2);}
            |       VarDec LB INT RB                                {$$ = newNode(@$.first_line, NOT_A_TOKEN, "VarDec", 4, $1, $2, $3, $4);}
            |       error RB                                        {syntaxError = 1;}
            ;
    */
    assert(node != nullptr);
    assert(specifier != nullptr);
    pNode child = node->children;
    pItem retItem = nullptr;
    if (!strcmp(child->name, "ID"))
    {
        retItem = newItem(table->stack->curStackDepth, newFieldList(newString(child->val), copyType(specifier)));
    }
    else if (!strcmp(child->name, "STAR"))
    {
        // implement pointer
        assert(0);
    }
    else if (!strcmp(child->name, "VarDec"))
    {
        assert(child->next != nullptr && child->next->next != nullptr);
        pNode idx = child->next->next;
        if (strcmp(idx->name, "INT"))
        {
            pError(not_int_array_idx, idx->lineno, "The array index is not a integer.");
        }
        else
        {
            pItem item = VarDec(child, specifier);
            assert(item != nullptr);
            if (item != nullptr)
            {
                int cur_size = atoi(idx->val);
                if (item->field->type->kind == ARRAY)
                {
                    int prev_size = item->field->type->u.array.size;
                    item->field->type->u.array.size = cur_size;
                    cur_size = prev_size;
                }
                retItem = newItem(table->stack->curStackDepth,
                                  newFieldList(newString(item->field->name), newType(ARRAY, cur_size, copyType(item->field->type))));
                deleteItem(item);
                item = nullptr;
            }
        }
    }
    else
    {
        assert(0);
    }
    return retItem;
}

pItem FunDec(pNode node, pType returnType, FuncState funcState)
{
    /*
    FunDec:         ID LP VarList RP
            |       ID LP RP
            |       error RP
            ;
    */
    assert(node != nullptr);
    assert(returnType != nullptr);
    assert(!strcmp(node->name, "FunDec"));
    pNode child = node->children;
    assert(child != nullptr);
    assert(!strcmp(child->name, "ID"));
    if (table->stack->curStackDepth != 0)
    {
        // Handle nesting function definition.
        pError(nest_func_def, node->lineno, "Nesting function definition is not allowed.\n");
        return nullptr;
    }
    // item is part of the table, CAN'T delete here!
    pItem item = searchFirstTableItem(table, child->val), funcItem = nullptr;
    if (item == nullptr)
    { // The function name hasn't appeared before.
        funcItem = newItem(table->stack->curStackDepth,
                           newFieldList(newString(child->val), newType(FUNC, funcState, 0, nullptr, copyType(returnType), child->lineno)));
        child = child->next;
        assert(child != nullptr);
        child = child->next;
        if (!strcmp(child->name, "VarList"))
        {
            VarList(child, funcItem);
        }
        addTableItem(table, funcItem);
    }
    else
    { // There is(are) former function declarations or definitions.
        if (item->field->type->kind != FUNC)
        {
            // The func name has already been defined as a non-func variable.
            char errorMsg[ERROR_MSG_SIZE];
            sprintf(errorMsg,
                    "The variable \"%s\" has already been defined.",
                    child->val);
            pError(redef_func, child->lineno, errorMsg);
            funcItem = nullptr;
        }
        else if (item->field->type->u.func.state == defined && funcState == defined)
        { // The function has already been defined above, and this is a def again.
            char errorMsg[ERROR_MSG_SIZE];
            sprintf(errorMsg,
                    "The function \"%s\" has already been defined.",
                    child->val);
            pError(redef_func, child->lineno, errorMsg);
            funcItem = nullptr;
        }
        else
        {
            funcItem = newItem(table->stack->curStackDepth,
                               newFieldList(newString(child->val), newType(FUNC, funcState, 0, nullptr, copyType(returnType), child->lineno)));
            child = child->next;
            assert(child != nullptr);
            child = child->next;
            if (!strcmp(child->name, "VarList"))
            {
                VarList(child, funcItem);
            }
            // Compare the function declaration (or definition).
            boolean funcMatch = checkFunDec(item, funcItem);
            deleteItem(funcItem);
            if (funcMatch == true)
            {
                funcItem = item;
                if (funcState == declared)
                {
                    // check with item, and do not addTableItem multiple times.
                    // do nothing, avoid multiple-time add func-declaration item
                }
                else if (funcState == defined)
                {
                    // check argv with item, and devise item directly.
                    funcItem->field->type->u.func.state = defined;
                }
            }
            else
            {
                funcItem = nullptr;
                item->field->type->u.func.state = funcState;
                char errorMsg[ERROR_MSG_SIZE];
                sprintf(errorMsg,
                        "The function \"%s\" definition dismatch the delaration.",
                        item->field->name);
                pError(dismatch_declare_func, child->lineno, errorMsg);
            }
        }
    }
    return funcItem;
}

void VarList(pNode node, pItem func)
{
    /*
    VarList:        ParamDec COMMA VarList
            |       ParamDec
            ;
    */
    assert(node != nullptr);
    assert(func != nullptr);
    assert(!strcmp(node->name, "VarList"));
    // The order matters, because we need to match the Arg matter.
    pNode child = node->children;
    pFieldList newParam = ParamDec(child), curr = func->field->type->u.func.argv, prev = nullptr;
    while (curr != nullptr)
    {
        prev = curr;
        curr = curr->tail;
    }
    if (prev == nullptr)
        func->field->type->u.func.argv = newParam;
    else
        prev->tail = newParam;
    func->field->type->u.func.argc += 1;

    child = child->next;
    if (child != nullptr)
    {
        child = child->next;
        VarList(child, func);
    }
}

pFieldList ParamDec(pNode node)
{
    /*
    ParamDec:       Specifier VarDec
            ;
    */
    assert(node != nullptr);
    assert(!strcmp(node->name, "ParamDec"));
    pNode child = node->children;
    pType specifierType = Specifier(child);
    if (specifierType == nullptr) // If error occurs in Specifier.
        return nullptr;
    pItem item = VarDec(child->next, specifierType);
    pFieldList ret = newFieldList(newString(item->field->name), copyType(item->field->type));
    if (specifierType != nullptr)
        deleteType(specifierType);
    specifierType = nullptr;
    // if (item != nullptr)
    //     deleteItem(item);
    // item = nullptr;
    // assert(ret->tail == nullptr);
    if (item != nullptr)
    {
        item->field->isArg = true;
        addTableItem(table, item);
        item = searchFirstTableItem(table, item->field->name);
    }
    return ret;
}

void CompSt(pNode node, pItem funcItem)
{
    /*
    CompSt:         LC DefList StmtList RC
            ;
    */
    assert(node != nullptr);
    assert(!strcmp(node->name, "CompSt"));
    pNode child = node->children; // LC
    pType returnType = nullptr;
    addStackDepth(table->stack);
    if (funcItem != nullptr)
    {
        assert(funcItem->field->type->kind == FUNC);
        returnType = funcItem->field->type->u.func.returnType;
        // In inter code translation, all param is already in the table since no name confliction.
        /*
        pFieldList param = funcItem->field->type->u.func.argv;
        while (param != nullptr)
        {
            addTableItem(table, newItem(table->stack->curStackDepth, copyFieldList(param)));
            param = param->tail;
        }
        */
    }
    assert(child != nullptr);
    child = child->next; // DefList
    if (child != nullptr && !strcmp(child->name, "DefList"))
    {
        DefList(child, nullptr); // For function, so no structItem here.
        child = child->next;
    }
    boolean retFlag = false;
    if (child != nullptr && !strcmp(child->name, "StmtList"))
    {
        retFlag = StmtList(child, funcItem); // StmtList
    }
    // Ignore the situation of non-void functions without return.
    /*if (retFlag == false)
    {
        if (!checkType(returnType, nullptr))
        {
            pError(dismatch_return, node->lineno, "Void type does not match function return value type.");
        }
    }*/
    clearCurDepthStackList(table);
    minusStackDepth(table->stack);
}

boolean StmtList(pNode node, pItem funcItem)
{
    /*
    StmtList:       //empty
        |       Stmt StmtList
        ;
    */
    if (node == nullptr)
    {
        return false;
    }
    assert(!strcmp(node->name, "StmtList"));
    pNode child = node->children;
    if (child == nullptr)
        return false;
    else
    {
        boolean retFlag = Stmt(child, funcItem);
        retFlag = StmtList(child->next, funcItem) || retFlag;
        return retFlag;
    }
}

boolean Stmt(pNode node, pItem funcItem)
{
    /*
    Stmt:           Exp SEMI
            |       CompSt
            |       RETURN Exp SEMI
            |       RETURN SEMI
            |       IF LP Exp RP Stmt %prec LOWER_THAN_ELSE
            |       IF LP Exp RP Stmt ELSE Stmt
            |       WHILE LP Exp RP Stmt
            ;
    */
    assert(node != nullptr);
    pNode child = node->children;
    assert(child != nullptr);
    assert(!strcmp(node->name, "Stmt"));
    pType tmpType = nullptr;
    boolean retFlag = false;
    pType returnType = nullptr;
    if (funcItem != nullptr)
    {
        returnType = copyType(funcItem->field->type->u.func.returnType);
    }
    if (!strcmp(child->name, "Exp"))
    {
        /*
        Stmt:   Exp SEMI
                ;
        */
        boolean lvalue = false;
        tmpType = Exp(child, &lvalue);
    }
    else if (!strcmp(child->name, "CompSt"))
    {
        /*
        Stmt:   CompSt
                ;
        */
        CompSt(child, funcItem);
    }
    else if (!strcmp(child->name, "RETURN"))
    {
        /*
        Stmt:           RETURN Exp SEMI
                |       RETURN SEMI
                ;
        */
        int returnLine = child->lineno;
        child = child->next;
        assert(child != nullptr);
        if (!strcmp(child->name, "Exp"))
        {
            boolean lvalue = false;
            tmpType = Exp(child, &lvalue);
            if (tmpType == nullptr)
            {
                // Do nothing
            }
            else if (!checkType(tmpType, returnType))
            {
                assert(returnType != nullptr);
                pError(dismatch_return, returnLine, "The return value type does not match that of declaration");
            }
            retFlag = true;
        }
        else if (strcmp(child->name, "SEMI"))
        {
            assert(0);
        }
    }
    else if (!strcmp(child->name, "IF"))
    {
        /*
        Stmt:           IF LP Exp RP Stmt %prec LOWER_THAN_ELSE
                |       IF LP Exp RP Stmt ELSE Stmt
                ;
        */
        child = child->next;
        assert(child != nullptr);
        child = child->next; // Exp
        boolean lvalue = false;
        tmpType = Exp(child, &lvalue);
        if (tmpType != nullptr)
        {
            // Seriously speaking, the ifFlag depend on the behaviour of tmpType
            // Here consider every if-branch should has a return;
            assert(child->next != nullptr);
            child = child->next->next;
            // Here, once return appears in the function, return true;
            // ifFalg &= Stmt(child, funcItem);
            retFlag |= Stmt(child, funcItem);
            child = child->next;
            if (child)
            {
                // Here, once return appears in the function, return true;
                // ifFalg &= Stmt(child->next, funcItem);
                // retFlag = ifFalg || retFlag;
                retFlag |= Stmt(child->next, funcItem);
            }
        }
    }
    else if (!strcmp(child->name, "WHILE"))
    {
        /*
        Stmt:   WHILE LP Exp RP Stmt
                ;
        */
        assert(child->next != nullptr);
        child = child->next->next;
        boolean lvalue = false;
        tmpType = Exp(child, &lvalue);
        if (tmpType == nullptr)
        {
            // Do nothing
        }
        else
        {
            assert(child->next != nullptr);
            // Here, once return appears in the function, return true;
            retFlag = Stmt(child->next->next, funcItem);
        }
    }
    else
    {
        assert(0);
    }
    if (tmpType != nullptr)
        deleteType(tmpType);
    tmpType = nullptr;
    if (returnType != nullptr)
        deleteType(returnType);
    returnType = nullptr;
    return retFlag;
}

void DefList(pNode node, pItem structInfo)
{
    /*
    DefList:        //empty
            |       Def DefList
            ;
    */
    if (node == nullptr)
    {
        return;
    }
    assert(!strcmp(node->name, "DefList"));
    pNode child = node->children;
    if (child == nullptr)
        return;
    else
    {
        Def(child, structInfo);
        DefList(child->next, structInfo);
    }
}

void Def(pNode node, pItem structInfo)
{
    /*
    Def:            Specifier DecList SEMI
        ;
    */
    assert(node != nullptr);
    assert(!strcmp(node->name, "Def"));
    pNode child = node->children;
    pType specifierType = Specifier(child);
    if (specifierType != nullptr)
        DecList(child->next, specifierType, structInfo);
}

void DecList(pNode node, pType specifier, pItem structInfo)
{
    /*
    DecList:        Dec
        |       Dec COMMA DecList
        ;
    */
    assert(node != nullptr);
    assert(!strcmp(node->name, "DecList"));
    pNode child = node->children;
    Dec(child, specifier, structInfo);
    if (child->next != nullptr)
        DecList(child->next->next, specifier, structInfo);
}

void Dec(pNode node, pType specifier, pItem structInfo)
{
    /*
    Dec:            VarDec
        |       VarDec ASSIGNOP Exp
        ;
    */
    assert(node != nullptr);
    assert(!strcmp(node->name, "Dec"));
    pNode child = node->children;
    pItem varItem = VarDec(child, specifier);
    assert(varItem != nullptr);
    if (child->next == nullptr)
    { // Dec -> VarDec
        if (structInfo != nullptr)
        { // inside a struct definition
            pFieldList structField = structInfo->field->type->u.structure.field, prev = nullptr;
            while (structField != nullptr)
            {
                if (!strcmp(structField->name, varItem->field->name))
                {
                    char errorMsg[ERROR_MSG_SIZE];
                    sprintf(errorMsg,
                            "The member \"%s\" has already been defined in the struct.",
                            varItem->field->name);
                    pError(redef_struct_field, child->lineno, errorMsg);
                    return;
                }
                else
                {
                    prev = structField;
                    structField = structField->tail;
                }
            }
            if (prev == nullptr)
                structInfo->field->type->u.structure.field = copyFieldList(varItem->field);
            else
                prev->tail = copyFieldList(varItem->field);
            if (varItem != nullptr)
                deleteItem(varItem);
            varItem = nullptr;
            assert(structInfo->field->type->u.structure.field);
        }
        else
        {
            // Outside of a struct
            if (checkTableItemConflict(table, varItem))
            {
                char errorMsg[ERROR_MSG_SIZE];
                sprintf(errorMsg,
                        "The variable \"%s\" has already been defined.",
                        varItem->field->name);
                pError(redef_var, child->lineno, errorMsg);
            }
            else
            {
                addTableItem(table, varItem);
            }
        }
    }
    else
    { // Dec -> VarDec ASSIGNOP Exp
        if (structInfo != nullptr)
        {
            pError(init_struct_field, child->lineno, "Variable initialization inside a struct is not allowed");
        }
        else
        {
            boolean lvalue = false;
            assert(child->next != nullptr);
            pType expType = Exp(child->next->next, &lvalue);
            if (expType == nullptr)
            {
                // Do nothing.
            }
            // First check whether the var is already defined.
            else if (checkTableItemConflict(table, varItem))
            {
                char errorMsg[ERROR_MSG_SIZE];
                sprintf(errorMsg,
                        "Using redefiend variable \"%s\"",
                        varItem->field->name);
                pError(redef_var, child->next->next->lineno, errorMsg);
            }
            // Then check if assignment to non-basic type
            // But here is a problem about printer, no handle here.
            else if (varItem->field->type->kind != BASIC && varItem->field->type->kind != STRUCTURE)
            {
                pError(rvalue_assign, node->lineno, "A right-hand value can not be assigned.");
            }
            // Last check whther the assignment type matches.
            else if (!checkType(expType, varItem->field->type))
            {
                pError(dismatch_assign, node->lineno, "Assignment type does not match.");
            }
            else
            {
                addTableItem(table, varItem);
            }
            if (expType != nullptr)
                deleteType(expType);
            expType = nullptr;
        }
    }
}

pType Exp(pNode node, pboolean lvalue)
{
    assert(node != nullptr);
    assert(!strcmp(node->name, "Exp"));
    pNode child = node->children;
    assert(child != nullptr);
    boolean exp1_lvalue = false, exp2_lvalue = false;
    pType exp1 = nullptr, exp2 = nullptr, retType = nullptr;
    if (!strcmp(child->name, "Exp"))
    {
        /*
        Exp:            Exp ASSIGNOP Exp
                |       Exp AND Exp
                |       Exp OR Exp
                |       Exp RELOP Exp
                |       Exp PLUS Exp
                |       Exp MINUS Exp
                |       Exp STAR Exp
                |       Exp DIV Exp
                |       Exp LB Exp RB
                |       Exp DOT ID
                ;
        */
        exp1 = Exp(child, &exp1_lvalue);
        child = child->next;
        assert(child != nullptr);
        if (exp1 == nullptr)
        {
            // The exp1 encounters error, do nothing.
        }
        else if (!strcmp(child->name, "LB"))
        {
            if (exp1->kind != ARRAY)
            {
                pError(non_array, child->lineno, "The variable is not an array.");
            }
            else
            {
                exp2 = Exp(child->next, &exp2_lvalue);
                if (exp2 == nullptr)
                {
                    // Do nothing
                }
                else if (!(exp2->kind == BASIC && exp2->u.basic == intType))
                {
                    pError(not_int_array_idx, child->lineno, "The array index is not an integer.");
                }
                Kind retKind = exp1->u.array.elem->kind;
                if (retKind == ARRAY || retKind == FUNC)
                {
                    *lvalue = false;
                }
                else
                {
                    *lvalue = true;
                }
                retType = copyType(exp1->u.array.elem);
            }
        }
        else if (!strcmp(child->name, "DOT"))
        {
            if (exp1->kind != STRUCTURE)
            {
                pError(dismatch_dot, child->lineno, "The variable is not a struct.");
            }
            else
            {
                child = child->next;
                assert(child != nullptr);
                pFieldList ptr = exp1->u.structure.field;
                while (ptr != nullptr)
                {
                    if (!strcmp(child->val, ptr->name))
                    {
                        break;
                    }
                    ptr = ptr->tail;
                }
                if (ptr == nullptr)
                {
                    char errorMsg[ERROR_MSG_SIZE];
                    sprintf(errorMsg,
                            "There is no member \"%s\" in the struct.",
                            child->val);
                    pError(undefined_field, child->lineno, errorMsg);
                }
                else
                {
                    Kind retKind = ptr->type->kind;
                    if (retKind == ARRAY || retKind == FUNC)
                    {
                        *lvalue = false;
                    }
                    else
                    {
                        *lvalue = true;
                    }
                    retType = copyType(ptr->type);
                }
            }
        }
        else if (!strcmp(child->name, "ASSIGNOP"))
        {
            child = child->next;
            exp2 = Exp(child, &exp2_lvalue);
            if (exp2 == nullptr)
            {
                // Do nothing.
            }
            else if (!exp1_lvalue)
            {
                pError(rvalue_assign, child->lineno, "A right-hand value cannot be assigned 1.");
            }
            else if (exp1->kind == ARRAY || exp1->kind == FUNC)
            {
                pError(rvalue_assign, child->lineno, "A right-hand value cannot be assigned.");
            }
            else if (!checkType(exp1, exp2))
            {
                pError(dismatch_assign, child->lineno, "Types for assignment dismatch.");
            }
            else
            {
                retType = copyType(exp1);
            }
            *lvalue = false;
        }
        else
        {
            /*
            Exp:        Exp AND Exp
                |       Exp OR Exp
                |       Exp RELOP Exp
                |       Exp PLUS Exp
                |       Exp MINUS Exp
                |       Exp STAR Exp
                |       Exp DIV Exp
            */
            char *opname = child->name;
            int opline = child->lineno;
            child = child->next;
            *lvalue = false;
            exp2 = Exp(child, &exp2_lvalue);
            if (exp2 == nullptr)
            {
                // do nothing.
            }
            else if (exp1->kind == ARRAY || exp1->kind == STRUCTURE || exp1->kind == FUNC || !checkType(exp1, exp2))
            {
                pError(dismatch_op, opline, "Type mismatched for operands.");
            }
            else if (!strcmp(opname, "AND") ||
                     !strcmp(opname, "OR") ||
                     !strcmp(opname, "RELOP"))
            {
                retType = newType(BASIC, intType);
            }
            else
            {
                retType = copyType(exp1);
            }
        }
    }
    else if (!strcmp(child->name, "LP"))
    {
        /*
        Exp:            LP Exp RP
                ;
        */
        boolean islvalue = false;
        retType = Exp(child->next, &islvalue);
    }
    else if (!strcmp(child->name, "MINUS") || !strcmp(child->name, "NOT"))
    {
        /*
        Exp:            MINUS Exp
                |       NOT Exp
                ;
        */
        exp1 = Exp(child->next, lvalue);
        if (exp1 == nullptr)
        {
            // Do nothing
        }
        else if (exp1->kind == BASIC && exp1->u.basic != voidType)
        {
            *lvalue = false;
            if (!strcmp(child->name, "MINUS"))
                retType = copyType(exp1);
            else
                retType = newType(BASIC, intType);
        }
        else
        {
            pError(dismatch_op, child->lineno, "The operands do not match the operator.");
        }
    }
    else if (!strcmp(child->name, "STAR"))
    {
        /*
        Exp:            STAR Exp
                ;
        */
        exp1 = Exp(child->next, lvalue);
        if (exp1 == nullptr)
        {
            // Do nothing
        }
        else if (exp1->kind == ARRAY)
        {
            retType = copyType(exp1->u.array.elem);
        }
        else if (exp1->kind == POINTER)
        {
            retType = copyType(exp1->u.pointer.elem);
        }
        else
        {
            pError(dismatch_op, child->lineno, "The operands do not match the operator.");
        }
        if (retType->kind == BASIC)
        {
            *lvalue = true;
        }
        else
        {
            *lvalue = false;
        }
    }
    else if (!strcmp(child->name, "ID"))
    {
        /*
        Exp:            ID LP Args RP
                |       ID LP RP
                |       ID
                ;
        */
        char *idName = newString(child->val);
        int idline = child->lineno;
        pItem item = searchFirstTableItem(table, idName); // item is part of table, CAN'T DELETE!
        child = child->next;                              // child -> LP or empty
        if (item == nullptr || isStructDef(item))
        {
            if (child == nullptr)
            { // ID
                *lvalue = true;
                char errorMsg[ERROR_MSG_SIZE] = {'\0'};
                sprintf(errorMsg,
                        "Using undefined variable \"%s\".",
                        idName);
                pError(undef_var, idline, errorMsg);
            }
            else
            { // ID LR (Args) RP
                *lvalue = true;
                char errorMsg[ERROR_MSG_SIZE] = {'\0'};
                sprintf(errorMsg,
                        "Call an undefined function \"%s\".",
                        idName);
                pError(undef_func, idline, errorMsg);
            }
        }
        else if (child == nullptr) // ID
        {
            *lvalue = true;
            retType = copyType(item->field->type);
        }
        else // ID LP (Args) RP
        {
            if (item->field->type->kind != FUNC)
            {
                char errorMsg[ERROR_MSG_SIZE] = {'\0'};
                sprintf(errorMsg,
                        "Call a non-function variable \"%s\".",
                        item->field->name);
                pError(non_func, idline, errorMsg);
            }
            else
            {
                *lvalue = false;
                child = child->next; // Args or RP
                assert(child);
                pNode args = nullptr;
                if (!strcmp(child->name, "Args")) // ID LP Args RP
                {
                    args = child;
                }
                Args(args, item->field->type->u.func.argv, idline);
                retType = copyType(item->field->type->u.func.returnType);
            }
        }
        if (idName != nullptr)
        {
            free(idName);
            idName = nullptr;
        }
    }
    else if (!strcmp(child->name, "INT"))
    {
        /*
        Exp:            INT
                ;
        */
        *lvalue = false;
        retType = newType(BASIC, intType);
    }
    else if (!strcmp(child->name, "FLOAT"))
    {
        /*
        Exp:            FLOAT
                ;
        */
        *lvalue = false;
        retType = newType(BASIC, floatType);
    }
    else if (!strcmp(child->name, "CHAR"))
    {
        /*
        Exp:            CHAR
                ;
        */
        *lvalue = false;
        retType = newType(BASIC, charType);
    }
    else
    {
        assert(0);
    }
    if (exp1 != nullptr)
    {
        deleteType(exp1);
        exp1 = nullptr;
    }
    if (exp2 != nullptr)
    {
        deleteType(exp2);
        exp2 = nullptr;
    }
    return retType;
}

void Args(pNode node, pFieldList funcArgInfo, int lineno)
{
    /*
    Args:           Exp COMMA Args
            |       Exp
            ;
    */
    if (node == nullptr && funcArgInfo == nullptr)
    {
        return;
    }
    else if (node == nullptr || funcArgInfo == nullptr)
    {
        pError(dismatch_para, lineno, "The number of parameter(s) don't match that in the declaration.");
    }
    else
    {
        assert(!strcmp(node->name, "Args"));
        boolean lvalue = false;
        pNode child = node->children;
        pType exp = Exp(child, &lvalue);
        if (exp == nullptr)
        {
            // Do nothing
        }
        if (!checkType(exp, funcArgInfo->type))
        {
            pError(dismatch_para, lineno, "The parameter(s) don't match that in the declaration.");
        }
        else
        {
            if (child->next != nullptr)
                child = child->next->next;
            else
                child = child->next;
            Args(child, funcArgInfo->tail, lineno);
        }
    }
}