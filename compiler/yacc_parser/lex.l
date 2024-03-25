%option noyywrap
%%
[ \f\n\r\t\v] {}
[\+\-\*\/\(\)] { return yytext[0]; }
[0-9]+ { return num; }
\$ { return 0; }
. { printf("unexpected character: %c\n", yytext[0]); exit(0); }
