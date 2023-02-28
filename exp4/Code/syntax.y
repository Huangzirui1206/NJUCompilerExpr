%{
    #include <stdio.h>
    #include "lex.yy.c"
    #include "type.h"
    #include "node.h"

    /*Declare types*/
    #define YYSTYPE pNode 

    /*Error flag*/
    extern int lexError;
    extern int syntaxError;

    pNode root;

    int yylex();
    void yyerror(char*);
    pNode newNode(int lineno, NodeType type, char* name, int argc, ...);
    pNode newTokenNode(int lineno, NodeType type, char* name, char* val);
    void delNode(pNode curr);
    void printSyntaxTree(pNode curr, int height);

%}

/* declared tokens */
%token INT
%token FLOAT
%token CHAR
%token ID
%token STRING
%token LC RC
%token STRUCT RETURN IF 
%token WHILE TYPE SEMI COMMA

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

/* operators */
%right ASSIGNOP
%left OR
%left AND
%left RELOP
%left PLUS MINUS
%left STAR DIV
%right NOT
%left LP RP LB RB DOT


%%
/* High-level Definitions */
Program:        ExtDefList                                      {
                                                                        $$ = newNode(@$.first_line, NOT_A_TOKEN, "Program", 1, $1);
                                                                        root = $$;
                                                                }
        ;
ExtDefList:     /* empty */                                     {$$ = NULL;}  
        |       ExtDef ExtDefList                               {$$ = newNode(@$.first_line, NOT_A_TOKEN, "ExtDefList", 2, $1, $2);}
        ;
ExtDef:         Specifier ExtDecList SEMI                       {$$ = newNode(@$.first_line, NOT_A_TOKEN, "ExtDef", 3, $1, $2, $3);}
        |       Specifier SEMI                                  {$$ = newNode(@$.first_line, NOT_A_TOKEN, "ExtDef", 2, $1, $2);}
        |       Specifier FunDec CompSt                         {$$ = newNode(@$.first_line, NOT_A_TOKEN, "ExtDef", 3, $1, $2, $3);}
        |       Specifier FunDec SEMI                           {$$ = newNode(@$.first_line, NOT_A_TOKEN, "ExtDef", 3, $1, $2, $3);}
        ;
ExtDecList:     VarDec                                          {$$ = newNode(@$.first_line, NOT_A_TOKEN, "ExtDecList", 1, $1);}
        |       VarDec COMMA ExtDecList                         {$$ = newNode(@$.first_line, NOT_A_TOKEN, "ExtDecList", 3, $1, $2, $3);}
        ;

/* Specifiers */
Specifier:      TYPE                                            {$$ = newNode(@$.first_line, NOT_A_TOKEN, "Specifier", 1, $1);}
        |       StructSpecifier                                 {$$ = newNode(@$.first_line, NOT_A_TOKEN, "Specifier", 1, $1);}
        ;
StructSpecifier:STRUCT OptTag LC DefList RC                     {$$ = newNode(@$.first_line, NOT_A_TOKEN, "StructSpecifier", 5, $1, $2, $3, $4, $5);}
        |       STRUCT Tag                                      {$$ = newNode(@$.first_line, NOT_A_TOKEN, "StructSpecifier", 2, $1, $2);}
        ;
OptTag: /* empty */                                             {$$ = NULL;}  
        |       ID                                              {$$ = newNode(@$.first_line, NOT_A_TOKEN, "OptTag", 1, $1);}
        ;
Tag:            ID                                              {$$ = newNode(@$.first_line, NOT_A_TOKEN, "Tag", 1, $1);}
        ;

/* Declarators */
VarDec:         ID                                              {$$ = newNode(@$.first_line, NOT_A_TOKEN, "VarDec", 1, $1);}
        |       STAR ID                                         {$$ = newNode(@$.first_line, NOT_A_TOKEN, "VarDec", 2, $1, $2);}
        |       VarDec LB INT RB                                {$$ = newNode(@$.first_line, NOT_A_TOKEN, "VarDec", 4, $1, $2, $3, $4);}
        |       error RB                                        {syntaxError = 1;}
        ;
FunDec:         ID LP VarList RP                                {$$ = newNode(@$.first_line, NOT_A_TOKEN, "FunDec", 4, $1, $2, $3, $4);}
        |       ID LP RP                                        {$$ = newNode(@$.first_line, NOT_A_TOKEN, "FunDec", 3, $1, $2, $3);}
        |       error RP                                        {syntaxError = 1;}
        ;
VarList:        ParamDec COMMA VarList                          {$$ = newNode(@$.first_line, NOT_A_TOKEN, "VarList", 3, $1, $2, $3);}
        |       ParamDec                                        {$$ = newNode(@$.first_line, NOT_A_TOKEN, "VarList", 1, $1);}
        ;
ParamDec:       Specifier VarDec                                {$$ = newNode(@$.first_line, NOT_A_TOKEN, "ParamDec", 2, $1, $2);}
        ;

/* Statement */
CompSt:         LC DefList StmtList RC                          {$$ = newNode(@$.first_line, NOT_A_TOKEN, "CompSt", 4, $1, $2, $3, $4);}
        |       error RC                                        {syntaxError = 1;}
        ;
StmtList:       /* empty */                                     {$$ = NULL;}       
        |       Stmt StmtList                                   {$$ = newNode(@$.first_line, NOT_A_TOKEN, "StmtList", 2, $1, $2);}
        ;
Stmt:           Exp SEMI                                        {$$ = newNode(@$.first_line, NOT_A_TOKEN, "Stmt", 2, $1, $2);}
        |       CompSt                                          {$$ = newNode(@$.first_line, NOT_A_TOKEN, "Stmt", 1, $1);}
        |       RETURN Exp SEMI                                 {$$ = newNode(@$.first_line, NOT_A_TOKEN, "Stmt", 3, $1, $2, $3);}
        |       RETURN SEMI                                     {$$ = newNode(@$.first_line, NOT_A_TOKEN, "Stmt", 2, $1, $2);}
        |       IF LP Exp RP Stmt %prec LOWER_THAN_ELSE         {$$ = newNode(@$.first_line, NOT_A_TOKEN, "Stmt", 5, $1, $2, $3, $4, $5);}
        |       IF LP Exp RP Stmt ELSE Stmt                     {$$ = newNode(@$.first_line, NOT_A_TOKEN, "Stmt", 7, $1, $2, $3, $4, $5, $6, $7);}
        |       WHILE LP Exp RP Stmt                            {$$ = newNode(@$.first_line, NOT_A_TOKEN, "Stmt", 5, $1, $2, $3, $4, $5);}
        |       error RP Stmt %prec LOWER_THAN_ELSE             {syntaxError = 1;}
        |       error RP Stmt ELSE Stmt                         {syntaxError = 1;}
        |       error SEMI                                      {syntaxError = 1;}
        ;

/* Local Definitions */
DefList:        /* empty */                                     {$$ = NULL;}
        |       Def DefList                                     {$$ = newNode(@$.first_line, NOT_A_TOKEN, "DefList", 2, $1, $2);}
        ;
Def:            Specifier DecList SEMI                          {$$ = newNode(@$.first_line, NOT_A_TOKEN, "Def", 3, $1, $2, $3);}
        ;
DecList:        Dec                                             {$$ = newNode(@$.first_line, NOT_A_TOKEN, "DecList", 1, $1);}
        |       Dec COMMA DecList                               {$$ = newNode(@$.first_line, NOT_A_TOKEN, "DecList", 3, $1, $2, $3);}
        ;
Dec:            VarDec                                          {$$ = newNode(@$.first_line, NOT_A_TOKEN, "Dec", 1, $1);}
        |       VarDec ASSIGNOP Exp                             {$$ = newNode(@$.first_line, NOT_A_TOKEN, "Dec", 3, $1, $2, $3);}
        ;

/* Expressions */
Exp:            Exp ASSIGNOP Exp                                {$$ = newNode(@$.first_line, NOT_A_TOKEN, "Exp", 3, $1, $2, $3);}
        |       Exp AND Exp                                     {$$ = newNode(@$.first_line, NOT_A_TOKEN, "Exp", 3, $1, $2, $3);}
        |       Exp OR Exp                                      {$$ = newNode(@$.first_line, NOT_A_TOKEN, "Exp", 3, $1, $2, $3);}
        |       Exp RELOP Exp                                   {$$ = newNode(@$.first_line, NOT_A_TOKEN, "Exp", 3, $1, $2, $3);}
        |       Exp PLUS Exp                                    {$$ = newNode(@$.first_line, NOT_A_TOKEN, "Exp", 3, $1, $2, $3);}
        |       Exp MINUS Exp                                   {$$ = newNode(@$.first_line, NOT_A_TOKEN, "Exp", 3, $1, $2, $3);}
        |       Exp STAR Exp                                    {$$ = newNode(@$.first_line, NOT_A_TOKEN, "Exp", 3, $1, $2, $3);}
        |       Exp DIV Exp                                     {$$ = newNode(@$.first_line, NOT_A_TOKEN, "Exp", 3, $1, $2, $3);}
        |       LP Exp RP                                       {$$ = newNode(@$.first_line, NOT_A_TOKEN, "Exp", 3, $1, $2, $3);}
        |       MINUS Exp                                       {$$ = newNode(@$.first_line, NOT_A_TOKEN, "Exp", 2, $1, $2);}
        |       STAR Exp                                        {$$ = newNode(@$.first_line, NOT_A_TOKEN, "Exp", 2, $1, $2);}
        |       NOT Exp                                         {$$ = newNode(@$.first_line, NOT_A_TOKEN, "Exp", 2, $1, $2);}
        |       ID LP Args RP                                   {$$ = newNode(@$.first_line, NOT_A_TOKEN, "Exp", 4, $1, $2, $3, $4);}
        |       ID LP RP                                        {$$ = newNode(@$.first_line, NOT_A_TOKEN, "Exp", 3, $1, $2, $3);}
        |       Exp LB Exp RB                                   {$$ = newNode(@$.first_line, NOT_A_TOKEN, "Exp", 4, $1, $2, $3, $4);}
        |       Exp DOT ID                                      {$$ = newNode(@$.first_line, NOT_A_TOKEN, "Exp", 3, $1, $2, $3);}
        |       ID                                              {$$ = newNode(@$.first_line, NOT_A_TOKEN, "Exp", 1, $1);}
        |       INT                                             {$$ = newNode(@$.first_line, NOT_A_TOKEN, "Exp", 1, $1);}
        |       FLOAT                                           {$$ = newNode(@$.first_line, NOT_A_TOKEN, "Exp", 1, $1);}
        |       CHAR                                            {$$ = newNode(@$.first_line, NOT_A_TOKEN, "Exp", 1, $1);}
        ;
Args:           Exp COMMA Args                                  {$$ = newNode(@$.first_line, NOT_A_TOKEN, "Args", 3, $1, $2, $3);}
        |       Exp                                             {$$ = newNode(@$.first_line, NOT_A_TOKEN, "Args", 1, $1);}
        ;

            
%%
void yyerror(char* msg){
    if(lexError == 0) fprintf(stdout, "Error type B at Line %d: %s.\n", yylineno, msg);
}