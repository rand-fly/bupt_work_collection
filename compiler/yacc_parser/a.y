%{
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#ifdef __WIN32
#include <windows.h>
#endif
int yylex();
void yyerror(const char *s);
int step;
%}
%start E
%token num
%%
E : E '+' T   { printf("(%d) E → E+T\n", ++step); }
  | E '-' T   { printf("(%d) E → E-T\n", ++step); }
  | T         { printf("(%d) E → T\n", ++step); }
  ;

T : T '*' F   { printf("(%d) T → T*F\n", ++step); }
  | T '/' F   { printf("(%d) T → T/F\n", ++step); }
  | F         { printf("(%d) T → F\n", ++step); }
  ;

F : '(' E ')' { printf("(%d) F → (E)\n", ++step); }
  | num       { printf("(%d) F → num\n", ++step); }
  ;
%%

int main() {
#ifdef __WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif
    if (yyparse()) {
        printf("Reject!");
    } else {
        printf("Accept!");
    }
    return 0;
}

#include "lex.yy.c"

void yyerror(const char *s) {}