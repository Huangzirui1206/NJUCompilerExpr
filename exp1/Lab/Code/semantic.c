#include "node.h"
#include "type.h"
#include "semantic.h"
#include "string.h"

// If the string serves as a parameter, it needs deep copy with newString.
static inline newString(char *src)
{
    assert(src != nullptr);
    char *dst = (char *)melloc(strlen(src) + 1), *ptr = dst;
    for (; *src; src++)
        *(ptr++) = *src;
    *ptr = '\0';
    return dst;
}

// Type functions
pType newType(Kind kind, ...)
{
    assert(kind == BASIC || kind == ARRAY || kind == FUNC || kind == STRUCT);
    pType p = (pType)malloc(sizeof(Type));
    assert(p != nullptr);
    p->kind = kind;
    va_list valist;
    if (kind == BASIC)
    {
        va_start(valist, 1);
        p->u.basic = va_arg(valist, BasicType);
    }
    else if (kind == ARRAY)
    {
        va_start(valist, 2);
        p->u.array.size = va_arg(valist, int);
        p->u.array.elem = va_arg(valist, pType);
    }
    else if (kind == STRUCT)
    {
        va_start(valist, 2);
        p->u.structure.structName = va_arg(valist, char *);
        p->u.structure.field = va_arg(valist, pFieldList);
    }
    else
    {
        va_start(valist, 4);
        p->u.func.state = va_arg(valist, FuncState);
        p->u.func.argc = va_arg(valist, int);
        p->u.func.argv = va_arg(valist, pFieldList);
        p->u.func.returnType = va_arg(valist, pType);
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
    else if (kind == STRUCT)
        return newType(kind, newString(src->u.structure.structName), copyFieldList(src->u.structure.field));
    else
        return newType(kind, src->u.func.state, src->u.func.argc, copyFieldList(src->u.func.argv), copyType(src->u.func.returnType));
}

// For debug
void deleteType(pType type)
{
    assert(type != nullptr);
    Kind kind = type->kind;
    assert(kind == BASIC || kind == ARRAY || kind == FUNC || kind == STRUCT);
    if (kind == ARRAY)
    {
        deleteType(type->u.array.elem);
        type->u.array.elem = nullptr;
    }
    else if (kind == STRUCT)
    {
        free(type->u.structure.structName);
        type->u.structure.structName = nullptr;
        while (type->u.structure.field)
        {
            pFieldList tmp = type->u.structure.field;
            type->u.structure.field = type->u.structure.field->tail;
            deleteFieldList(tmp);
        }
        type->u.structure.field = nullptr;
    }
    else
    {
        while (type->u.func.argv)
        {
            pFieldList tmp = type->u.func.argv;
            type->u.func.argv = type->u.func.argv->tail;
            deleteFieldList(tmp);
        }
        type->u.func.argv = nullptr;
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
#ifndef FUNC_DECLARE
    if (type1 == FUNC && type2 == FUNC)
        return false;
#else
    if (type1 == FUNC && type2 == FUNC)
    {
        if (type1->u.func.state == incorrect || type2->u.func.state == incorrect)
        {
            return false;
        }
        else if (type1->u.func.state == defined && type2->u.func.state == defined)
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
#endif
    if (type1->kind != type2->kind)
        return false;
    else
    {
        assert(type1->kind == BASIC || type1->kind == ARRAY || type1->kind == STRUCT);
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
#ifndef STRUCT_EQUIVALENT
            return !strcmp(type1->u.structure.structName,
                           type2->u.structure.structName);
#else
            assert(0);
#endif
        }
    }
}

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
    assert(src != nullptr);
    pFieldList p = newFieldList(newString(src->name), copyType(src->type));
    if (src->tail != nullptr)
        p->tail = copyFieldList(src->tail);
    return p;
}

void deleteFieldList(pFieldList fieldList)
{
    assert(fieldList != nullptr);
    if (fieldList->tail != nullptr)
        deleteFieldList(fieldList->tail);
    if (fieldList->name != nullptr)
        free(fieldList->name);
    fieldList = nullptr;
    if (fieldList->type != nullptr)
        deleteType(fieldList->type);
    free(fieldList);
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
    pItem p = (pItem)molloc(sizeof(TableItem));
    assert(p != nullptr);
    p->symbolDepth = symbolDepth;
    // assert(pfield != nullptr);
    p->field = pfield;
    p->nextHash = p->prevHash = p->nextSymbol = nullptr;
}

void deleteItem(pItem item)
{
    assert(item != nullptr);
    free(item);
}

boolean isStructDef(pItem src)
{
    assert(src != nullptr);
    return (src->field->type->kind == STRUCT) && (src->field->type->u.structure.structName == nullptr);
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
    assert(stack->curStackDepth - 1 >= HASH_TABLE_SIZE);
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
    return p;
}

void deleteTable(pTable table)
{
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
            if (prev->field->type->kind == STRUCT ||
                item->field->type->kind == STRUCT)
            {
                return true;
            }
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
        if (nextHash != nullptr)
            setHashHead(table->hash, idx, nextHash);
        else
            setHashHeadEmpty(table->hash, idx);
    }
    // Stack
    pItem nextSym = item->nextSymbol, prevSym = item->prevSymbol;
    if (nextSym != nullptr)
        nextSym->prevSymbol = prevSym;
    if (prevSym != nullptr)
        prevSym->nextSymbol = nextSym;
    else
    {
        if (nextSym != nullptr)
            setCurDepthStackHead(table->stack, nextSym);
        else
            setCurDepthStackHeadEmpty(table->stack);
    }
    // delete item
    deleteItem(item);
}

void clearCurDepthStackList(pTable table)
{
    while (getCurDepthStackHead(table->stack) != nullptr)
    {
        deleteTableItem(table, getCurDepthStackHead(table->stack));
    }
}

/*For debug*/
void printTable(pTable table)
{
    assert(0);
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
    child = child->next;
    assert(child != nullptr);
    char *name = child->name;
    if (!strcmp(name, "ExtDecList"))
    {
        // ExtDef → Specifier ExtDecList SEMI
        ExtDecList(child, specifierType);
        assert(child->next == nullptr);
    }
    else if (!strcmp(name, "FunDec"))
    {
        // ExtDef → Specifier FunDec CompSt
        pItem item = FunDec(child, specifierType);
        child = child->next;
        assert(child != nullptr);
        item->field->type->u.func.state = defined;
        CompSt(child, specifierType);
    }
    else
    {
        assert(0);
    }
    deleteType(specifierType);
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
        pError(redef_var, child->lineno, "Redefined variable.");
    }
    else
    {
        addTableItem(table, item);
        child = child->next;
        assert(nullptr);
        child = child->next;
        if (child != nullptr)
        {
            ExtDecList(child, specifier);
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
    if (!strcmp(child->name, "TYPE"))
    {
        if (!strcmp(child->val, "int"))
        {
            return newType(BASIC, intType);
        }
        else if (!strcmp(child->val, "float"))
        {
            return newType(BASIC, floatType);
        }
        else if (!strcmp(child->val, "char"))
        {
            return newType(BASIC, charType);
        }
        else if (!strcmp(child->val, "void"))
        {
            return newType(BASIC, voidType);
        }
        else
        {
            assert(0);
        }
    }
    else if (!strcmp(child->name, "StructSpecifier"))
    {
        return StructSpecifier(child);
    }
    else
    {
        assert(0);
    }
}

pType StructSpecifier(pNode node)
{
    /*
    StructSpecifier:STRUCT OptTag LC DefList RC
        |       STRUCT Tag
        ;
    */
    assert(node != nullptr);
    assert(!strcmp(node->name, "StructSpecifier"));
    assert(node->next != nullptr);
    pNode child = node->children->next;
    boolean withName = false;
    assert(child != nullptr);
    if (!strcmp(child->name, "Tag"))
    { // The employment of structure.
        pNode id = child->children;
        assert(id != nullptr);
        pItem item = searchFirstTableItem(table, id->val);
        if (item == nullptr || isStructDef(item))
        {
            pError(undef_var, id->lineno, "Undefined variable or a struct definition.");
        }
        else
        {
            return newType(STRUCT, newString(item->field->name),
                           copyFieldList(item->field->type->u.structure.field));
        }
    }
    else if (!strcmp(child->name, "OptTag") || !strcmp(child->name, "LC"))
    { // The definition of strcture
        pItem structItem = newItem(table->stack->curStackDepth,
                                   newFieldList("", newType(STRUCT, nullptr, nullptr)));
        if (!strcmp(child->name, "OptTag"))
        {
            withName = true;
            // struct def with name
            pNode id = child->children;
            assert(id != nullptr);
            setFieldListName(structItem, newString(id->val));
            child = child->next;
            assert(child);
        }
        else
        {
            // struct def without name
            table->unNamedStructNum += 1;
            char structName[20] = {0}; // Set a name for it by counting
            sprintf(structName, "%d", table->unNamedStructNum);
            setFieldListName(structItem->field, newString(structName));
        }
        child = child->next;
        assert(child);
        addStackDepth(table->stack);
        // Go into the struct field
        DefList(child, structItem);
        // Go out of the struct field
        clearCurDepthStackList(table);
        minusStackDepth(table->stack);

        if (checkTableItemConflict(table, structItem))
        {
            pError(redef_struct, child->lineno, "Struct redefined.");
        }
        else
        {
            pNode ret = newType(STRUCT, newString(structItem->field->name),
                                copyFieldList(structItem->field->type->u.structure.field));
            if (withName)
            {
                addTableItem(table, structItem);
            }
            else
            {
                deleteItem(structItem);
            }

            return ret;
        }
    }
    else
    {
        assert(0);
    }
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
    if (!strcmp(child->name, "ID"))
    {
        return newItem(table->stack->curStackDepth, newFieldList(newString(child->val), copyType(specifier)));
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
            pError(not_int_array_idx, idx->lineno, "This is not an array variable.");
        }
        else
        {
            pType spf = newType(ARRAY, idx->val, copyType(specifier));
            pItem item = VarDec(child, spf);
            deleteType(spf);
            return item;
        }
        
    }
    else
    {
        assert(0);
    }
}

pItem FunDec(pNode node, pType returnType)
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
    pItem funcItem = newItem(table->stack->curStackDepth,
                             newFieldList(child->val, newType(FUNC, declared, 0, nullptr, copyType(returnType))));
    int funcLine = child->lineno;
    child = child->next;
    assert(child != nullptr);
    child = child->next;
    if (!strcmp(child->name, "VarList"))
    {
        VarList(child, funcItem);
    }

    if (checkTableItemConflict(table, funcItem))
    {
        pError(redef_func, funcLine, "The function is redifined");
    }
    else
    {
        addTableItem(table, funcItem);
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
    pNode child = node->children;
    pFieldList prev = func->field->type->u.func.argv;
    func->field->type->u.func.argv = ParamDec(child);
    func->field->type->u.func.argv->tail = prev;
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
    pItem item = VarDec(child->next, specifierType);
    pFieldList ret = newFieldList(newString(item->field->name), copyType(item->field->type));
    deleteType(specifierType);
    deleteItem(item);
    return ret;
}

void CompSt(pNode node, pType returnType)
{
    /*
    CompSt:         LC DefList StmtList RC
            ;
    */
    assert(node != nullptr);
    assert(!strcmp(node->name, "CompSt"));
    pNode child = node->children; /// LC
    assert(child != nullptr);
    child = child->next;    // DefList
    assert(child->next != nullptr);
    DefList(child, nullptr); // For function, so no structItem here.
    boolean retFlag = StmtList(child->next, returnType); // StmtList
    if (retFlag == false)
    {
        if (!checkType(returnType, nullptr))
        {
            pError(dismatch_return, node->lineno, "Void type does not match function return value type.");
            assert(0);
        }
    }
}

boolean StmtList(pNode node, pType returnType)
{
    /*
    StmtList:       //empty
        |       Stmt StmtList
        ;
    */
    assert(node != nullptr);
    assert(!strcmp(node->name, "StmtList"));
    pNode child = node->children;
    if (child == nullptr)
        return nullptr;
    else
    {
        boolean retFlag = Stmt(child, returnType);
        retFlag = StmtList(child->next, returnType) || retFlag;
        return retFlag;
    }
}

boolean Stmt(pNode node, pType returnType)
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
        CompSt(child, nullptr);
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
        if (!strcmp(child->next, "Exp"))
        {
            boolean lvalue = false;
            tmpType = Exp(child, &lvalue);
            if (!checkType(tmpType, returnType))
            {
                pError(dismatch_return, returnLine, "The return value type does not match that of declaration");
            }
            retFlag = true;
        }
        else if (strcmp(child->next, "SEMI"))
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
        // boolean ifFalg = true;
        child = child->next;
        assert(child != nullptr);
        child = child->next; // Exp
        boolean lvalue = false;
        tmpType = Exp(child, &lvalue);
        // Seriously speaking, the ifFlag depend on the behaviour of tmpType
        // Here consider every if-branch should has a return;
        assert(child->next != nullptr);
        child = child->next->next;
        // Here, once return appears in the function, return true;
        // ifFalg &= Stmt(child, returnType);
        retFlag |= Stmt(child, returnType);
        child = child->next;
        if (child)
        {
            // Here, once return appears in the function, return true;
            // ifFalg &= Stmt(child->next, returnType);
            // retFlag = ifFalg || retFlag;
            retFlag |= Stmt(child->next, returnType);
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
        assert(child->next != nullptr);
        // Here, once return appears in the function, return true;
        retFlag = Stmt(child->next->next, returnType);
    }
    else
    {
        assert(0);
    }
    if (tmpType != nullptr)
        deleteType(tmpType);
    return retFlag;
}

void DefList(pNode node, pItem structInfo)
{
    /*
    DefList:        //empty
            |       Def DefList
            ;
    */
    assert(node != nullptr);
    assert(!strcmp(node->name, "DefList"));
    pNode child = node->children;
    if (child == nullptr)
        return;
    else
    {
        Def(child, structInfo);
        DefList(child, structInfo);
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
            pFieldList structField = structInfo->field, prev = nullptr;
            while (structField != nullptr)
            {
                if (!strcmp(structField->name, varItem->field->name))
                {
                    pError(redef_struct_field, child->lineno, "Variable redefined inside the struct");
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
            deleteItem(varItem);
        }
        else
        {
            // Outside of a struct
            if (checkTableItemConflict(table, varItem))
            {
                pError(redef_var, child->lineno, "The variable is redefined.");
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
            // First check whether the var is already defined.
            if (!checkTableItemConflict(table, expType))
            {
                pError(undef_var, child->next->next->lineno, "Using undefined variable.");
            }
            // Then check whther the assignment type matches.
            if (!checkType(expType, varItem->field->type))
            {
                pError(dismatch_assign, node->lineno, "Assignment type does not match.");
            }
            // Last check if assignment to non-basic type
            // But here is a problem about printer, no handle here.
            if (varItem->field->type->kind != BASIC)
            {
                pError(rvalue_assign, node->lineno, "Right value can not be assigned.");
            }
            addTableItem(table, varItem);
            if (expType != nullptr)
            {
                deleteType(expType);
            } 
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
        exp1 = Exp(child, exp1_lvalue);
        child = child->next;
        assert(child != nullptr);
        if (!strcmp(child->name, "LB"))
        {
            if (!exp1->kind != ARRAY)
            {
                pError(non_array, child->lineno, "The variable is not an array.");
            }
            exp2 = Exp(child->next, exp2_lvalue);
            if (!(exp2->kind == BASIC && exp2->u.basic != intType))
            {
                pError(not_int_array_idx, child->lineno, "The array index is not an integer.");
            }
            Kind retKind = exp1->u.array.elem->kind;
            if (retKind == ARRAY || retKind == FUNC || retKind == STRUCT)
            {
                *lvalue = false;
            }
            else
            {
                *lvalue = true;
            }
            retType = copyType(exp1->u.array.elem);
        }
        else if (!strcmp(child->name, "DOT"))
        {
            if (exp1->kind != STRUCT)
            {
                pError(dismatch_dot, child->name, "The variable is not a struct.");
            }
            child = child->next;
            assert(child != nullptr);
            pFieldList ptr = exp1->u.structure.field;
            while (ptr != nullptr)
            {
                if (!strcmp(child->val, ptr->name))
                {
                    break;
                }
            }
            if (ptr == nullptr)
            {
                pError(undefined_field, child->lineno, "Using undefined member in the struct.");
            }
            Kind retKind = ptr->type->kind;
            if (retKind == ARRAY || retKind == FUNC || retKind == STRUCT)
            {
                *lvalue = false;
            }
            else
            {
                *lvalue = true;
            }
            retType = copyType(ptr->type);
        }
        else if (!strcmp(child->name, "ASSIGNOP"))
        {
            if (!exp1_lvalue)
            {
                pError(rvalue_assign, child->name, "The right value cannot be assigned.");
            }
            child = child->next;
            exp2 = Exp(child, &exp2_lvalue);
            if (exp1->kind == ARRAY || exp1->kind == STRUCT || exp1->kind == FUNC)
            {
                pError(rvalue_assign, child->name, "The right value cannot be assigned.");
            }
            else if (!checkType(exp1, exp2))
            {
                pError(dismatch_assign, child->lineno, "The type of assignment does not match.");
            }
            *lvalue = false;
            retType = copyType(exp1);
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
            exp2 = Exp(child, &exp2_lvalue);
            if (exp1->kind == ARRAY || exp1->kind == STRUCT || exp1->kind == FUNC)
            {
                pError(dismatch_op, opline, "The operands do not match the operator.");
            }
            *lvalue = false;
            if (!strcmp(opname, "AND") ||
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
        if (exp1->kind == BASIC && exp1->u.basic != voidType)
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
        if (exp1->kind == ARRAY)
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
        pItem item = searchFirstTableItem(table, idName);
        child = child->next;
        if (item == nullptr || isStructDef(item))
        {
            pError(undef_var, idline, "Using undefined variable.");
        }
        else if (child == nullptr)
        {
            *lvalue = true;
            retType = copyType(item->field->type);
        }
        else
        {
            if (item->field->type->kind != FUNC)
            {
                pError(non_func, idline, "Call a non-function variable.");
            }
            else
            {
                *lvalue = false;
                child = child->next;
                assert(child);
                pNode args = nullptr;
                if (!strcmp(child->name, "Args"))
                {
                    args = child->next;
                    args = args->children;
                }
                Args(args, item->field->type->u.func.argv);
                retType = copyType(item->field->type->u.func.returnType);
            }
        }
        free(idName);
        deleteItem(item);
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
        deleteType(exp1);
    if (exp2 != nullptr)
        deleteType(exp2);
    return retType;
}

void Args(pNode node, pFieldList funcArgInfo)
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
        pError(dismatch_para, node->lineno, "The parameter(s) don't match that in the declaration.");
    }
    assert(!strcmp(node->name, "Args"));
    boolean lvalue = false;
    pType exp = Exp(node, &lvalue);
    if (!checkType(exp, funcArgInfo->type))
    {
        pError(dismatch_para, node->lineno, "The parameter(s) don't match that in the declaration.");
    }
    if (node->next != nullptr)
        node = node->next->next;
    else
        node = node->next;
    Args(node, funcArgInfo->tail);
}
