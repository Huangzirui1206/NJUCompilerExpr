%{
    /*Syntax Tree include*/
    #include "type.h"
    #include "node.h"

    /*Declare vvlval type*/
    #define YYSTYPE pNode

    /*Bison include*/
    #include "syntax.tab.h"

    #include <string.h>

    int yycolumn = 1;

    /*Error flag*/
    extern int lexError;

    #define YY_USER_ACTION\
        yylloc.first_line = yylloc.last_line = yylineno;\
        yylloc.first_column = yycolumn;\
        yylloc.last_column = yycolumn + yyleng - 1;\
        yycolumn += yyleng;

    /*Function*/
    int commandHandler();
%}

digit [0-9]
letter_ [a-zA-Z_]

DEC (0)|([1-9]{digit}*)
OCT 0[0-7]+
HEX (0x|0X)[0-9A-Fa-f]+
INT {DEC}|{OCT}|{HEX}
/*Lexical Error -- INT*/
ERRI_OCT 0[0-7]*[89]{digit}*
ERRI_HEX (0x|0X)[0-9A-Fa-f]*[G-Zg-z][0-9A-Fa-f]*

NOR_FLOAT {digit}+\.{digit}+ 
EXP_FLOAT (({digit}+\.{digit}*)|(\.{digit}+))[eE][+-]?{digit}+
FLOAT {NOR_FLOAT}|{EXP_FLOAT}
/*Lexical Error -- FLOAT*/
ERRF_DOT_AT_END {digit}+\.
ERRF_DOT_AT_HEAD \.{digit}+
ERRF_TOO_MANY_DOTS {digit}+\.{digit}+\.({digit}|(\.))*
ERRF_EXP (({digit}+\.{digit}*)|(\.{digit}+))[eE][+-]?(({digit}+\.{digit}*)|(\.{digit}+))
ERRF_BASE ({digit}+|({ERRF_TOO_MANY_DOTS}))[eE][+-]?{digit}+
ERRF_E_AT_END (({digit}+\.{digit}*)|(\.{digit}+))[eE][+-]?
ERRF {ERRF_DOT_AT_END}|{ERRF_DOT_AT_HEAD}|{ERRF_TOO_MANY_DOTS}|{ERRF_EXP}|{ERRF_BASE}|{ERRF_E_AT_END}

ID {letter_}({digit}|{letter_}){0,30}

SEMI ;
COMMA ,
ASSIGNOP =
RELOP >|<|>=|<=|==|!=
PLUS \+
MINUS \-
STAR \*
DIV \/
AND &&
OR \|\| 
DOT \.
NOT !
TYPE int|float|char|void|bool
LP \(
RP \)
LB \[
RB \]
LC \{
RC \}
STRUCT struct
RETURN return
IF if
ELSE else
WHILE while

WHITE [\r\n\t ]+

COMMAND_LINE \/\/[^\n]*
COMMAND_BLOCK_L \/\*
COMMAND_BLOCK_R \*\/

STRING \"((\\.)|([^\"]))*\"
DQUO \"

CHAR '[^']'
/*Lexical error -- CHAR*/
ERRC '[^']{2,}'
SQUO '

%option yylineno


%%
\n {yycolumn = 1;}

{SEMI}              {yylval = newNode(yylineno, TOKEN_SYMBOL, "SEMI", 0);return SEMI;}
{COMMA}             {yylval = newNode(yylineno, TOKEN_SYMBOL, "COMMA", 0);return COMMA;}
{ASSIGNOP}          {yylval = newNode(yylineno, TOKEN_SYMBOL, "ASSIGNOP", 0);return ASSIGNOP;}
{RELOP}             {yylval = newNode(yylineno, TOKEN_SYMBOL, "RELOP", 0);return RELOP;}
{PLUS}              {yylval = newNode(yylineno, TOKEN_SYMBOL, "PLUS", 0);return PLUS;}
{MINUS}             {yylval = newNode(yylineno, TOKEN_SYMBOL, "MINUS", 0);return MINUS;}
{STAR}              {yylval = newNode(yylineno, TOKEN_SYMBOL, "STAR", 0);return STAR;}
{DIV}               {yylval = newNode(yylineno, TOKEN_SYMBOL, "DIV", 0);return DIV;}
{AND}               {yylval = newNode(yylineno, TOKEN_SYMBOL, "AND", 0);return AND;}
{OR}                {yylval = newNode(yylineno, TOKEN_SYMBOL, "OR", 0);return OR;}
{DOT}               {yylval = newNode(yylineno, TOKEN_SYMBOL, "DOT", 0);return DOT;}
{NOT}               {yylval = newNode(yylineno, TOKEN_SYMBOL, "NOT", 0);return NOT;}
{TYPE}              {yylval = newTokenNode(yylineno, TOKEN_TYPE, "TYPE", yytext);return TYPE;}
{LP}                {yylval = newNode(yylineno, TOKEN_SYMBOL, "LP", 0);return LP;}
{RP}                {yylval = newNode(yylineno, TOKEN_SYMBOL, "RP", 0);return RP;}
{LB}                {yylval = newNode(yylineno, TOKEN_SYMBOL, "LB", 0);return LB;}
{RB}                {yylval = newNode(yylineno, TOKEN_SYMBOL, "RB", 0);return RB;}
{LC}                {yylval = newNode(yylineno, TOKEN_SYMBOL, "LC", 0);return LC;}
{RC}                {yylval = newNode(yylineno, TOKEN_SYMBOL, "RC", 0);return RC;}
{STRUCT}            {yylval = newNode(yylineno, TOKEN_SYMBOL, "STRUCT", 0);return STRUCT;}
{RETURN}            {yylval = newNode(yylineno, TOKEN_SYMBOL, "RETURN", 0);return RETURN;}
{IF}                {yylval = newNode(yylineno, TOKEN_SYMBOL, "IF", 0);return IF;}
{ELSE}              {yylval = newNode(yylineno, TOKEN_SYMBOL, "ELSE", 0);return ELSE;}
{WHILE}             {yylval = newNode(yylineno, TOKEN_SYMBOL, "WHILE", 0);return WHILE;}
{WHITE}             {;}

{INT}               {yylval = newTokenNode(yylineno, TOKEN_INT, "INT", yytext);return INT;}
{FLOAT}             {yylval = newTokenNode(yylineno, TOKEN_FLOAT, "FLOAT", yytext);return FLOAT;}
{ID}                {yylval = newTokenNode(yylineno, TOKEN_ID, "ID", yytext);return ID;}

{ERRI_OCT}          {
                        lexError = 1;
                        printf("Error type A at Line %d: Illegal octal number \"%s\"\n", yylineno, yytext);
                    }
{ERRI_HEX}          {
                        lexError = 1;
                        printf("Error type A at Line %d: Illegal hexadecimal number \"%s\"\n", yylineno, yytext);
                    }
{ERRF}              {
                        lexError = 1;
                        printf("Error type A at Line %d: Illegal floating point number \"%s\"\n", yylineno, yytext);
                    }


{CHAR}              {
                        yytext[2] = '\0';
                        yylval = newTokenNode(yylineno, TOKEN_CHAR, "CHAR", yytext + 1);
                        return CHAR;
                    }
{SQUO}              {
                        lexError = 1;
                        printf("Error type A at Line %d: Missing Terminating ''' character.\n",yylineno);
                    }
{ERRC}              {
                        lexError = 1;
                        printf("Error type A at Line %d: Too many characters in ''.\n",yylineno);
                    }

{STRING}            {
                        yytext[strlen(yytext) - 1] = '\0'; 
                        yylval = newTokenNode(yylineno, TOKEN_STRING, "STRING", yytext + 1);
                        return STRING;
                    }
{DQUO}              {
                        lexError = 1;
                        printf("Error type A at Line %d: Missing Terminating '\"' character.\n",yylineno);
                    }

{COMMAND_LINE}      {;}
{COMMAND_BLOCK_L}   {
                        int ret = commandHandler();
                        if(!ret) {
                            lexError = 1;
                            printf("Error type A at Line %d: Missing Terminating '*/'.\n",yylineno);
                        }
                    }

.                   {
                        lexError = 1;
                        printf("Error type A at Line %d: Mysterious characters \'%s\'\n",
                        yylineno, yytext);
                    }
%%


int commandHandler(){
    int state = 0;
    while(state != 2){
        char c = input();
        if(c == 0) return 0;
        else if(c == '*') state = 1;
        else if(state == 1 && c == '/') state = 2;
        else state = 0;
    }
    return 1;
}