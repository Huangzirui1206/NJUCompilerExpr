#ifndef NODE_H
#define NODE_H

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "type.h"

#define true 1
#define false 0
#define nullptr NULL

typedef struct node{
    int lineno;
    char* name;
    NodeType type;
    char* val;
    struct node* children;
    struct node* next; /* next sibling */
} Node;
typedef Node* pNode;

inline pNode newNode(int lineno, NodeType type, char* name, int argc, ...){
    pNode curr = nullptr;
    int nameLen = strlen(name) + 1;
    curr = (pNode)malloc(sizeof(Node));
    assert(curr);

    curr->lineno = lineno;
    curr->type = type;
    curr->name = (char*)malloc(nameLen * sizeof(char));
    assert(curr->name);
    strcpy(curr->name, name);
    curr->val = nullptr;

    if(argc > 0){
        va_list ap;
        va_start(ap, argc);
        curr->children = (pNode)va_arg(ap, pNode);
        pNode prev = curr->children;
        for(int i = 1; i < argc; i++){
            assert(prev);
            prev->next = (pNode)va_arg(ap, pNode);
            if(prev->next) prev = prev->next;
        }
        va_end(ap);
    }
    else{
        curr->children = nullptr;
    }
    curr->next = nullptr;

    return curr;
}

inline pNode newTokenNode(int lineno, NodeType type, char* name, char* val){
    pNode curr = nullptr;
    int nameLen = strlen(name) + 1;
    int valLen = strlen(val) + 1;
    curr = (pNode)malloc(sizeof(Node));
    assert(curr);

    curr->lineno = lineno;
    curr->type = type;
    curr->name = (char*)malloc(nameLen * sizeof(char));
    assert(curr->name);
    strcpy(curr->name, name);
    curr->val = (char*)malloc(valLen * sizeof(char));
    assert(curr->val);
    strcpy(curr->val, val);

    curr->children = nullptr;
    curr->next = nullptr;

    return curr;
}

inline void delNode(pNode curr){
    if(!curr) return;
    pNode cptr = curr->children, sptr = curr->next;
    free(curr->name);
    free(curr->val);
    free(curr);
    curr->name = nullptr;
    curr->val = nullptr;
    curr = nullptr;
    delNode(cptr);
    delNode(sptr);
}

inline void printSyntaxTree(pNode curr, int height){
    if(curr == nullptr) return;
    for(int i = 0; i < height; ++i) printf("  ");

    printf("%s", curr->name);
    switch (curr->type)
    {
    case TOKEN_ID:
    case TOKEN_TYPE:{
        printf(": %s\n", curr->val);
        break;
    }
    case TOKEN_STRING:{
        printf(": \"%s\"\n", curr->val);
        break;
    }
    case TOKEN_CHAR:{
        printf(": \'%s\'\n", curr->val);
        break;
    }
    case TOKEN_INT:{
        int res = 0;
        if(curr->val[0] != '0') res = atoi(curr->val);
        else if(curr->val[0] != '\0' && curr->val[1] != 'x' && curr->val[1] != 'x'){
            int i = 1;
            while(curr->val[i]){
                res = res * 8 + curr->val[i] - '0';
                i++;
            }
        }
        else{
            int i = 2;
            while(curr->val[i]){
                char c = curr->val[i];
                if(c >= '0' && c <= '9') res = res * 16 + c - '0';
                else if(c >= 'A' && c <= 'F') res = res * 16 + c - 'A' + 10;
                else if(c >= 'a' && c <= 'f') res = res * 16 + c - 'a' + 10;
                i++;
            }
        }
        printf(": %d\n", res);
        break;
    }
    case TOKEN_FLOAT:{
        printf(": %f\n", (float)atof(curr->val));
        break;
    }
    case NOT_A_TOKEN:{
        printf(" (%d)\n", curr->lineno);
        break;
    }
    default:{
        printf("\n");
        break;
    }
    }
    printSyntaxTree(curr->children, height + 1);
    printSyntaxTree(curr->next, height);
}

#endif