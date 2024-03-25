%option yylineno
%option noyywrap
%{
enum {
    T_EOF,
    T_ERROR,
    T_IDENTIFIER,
    T_KEYWORD,
    T_PUNCTUATOR,
    T_INTEGER_CONSTANT,
    T_FLOATING_CONSTANT,
    T_CHAR_CONSTANT,
    T_STRING_LITERAL,
};

int yycolumn = 1, char_cnt = 0;
int start_line, start_column;
int prev_yylineno = yylineno;

#define YY_USER_ACTION                                                   \
  start_line = prev_yylineno; start_column = yycolumn;                   \
  char_cnt += yyleng;                                                    \
  if (yylineno == prev_yylineno) yycolumn += yyleng;                     \
  else {                                                                 \
    for (yycolumn = 1; yytext[yyleng - yycolumn] != '\n'; ++yycolumn) {} \
    prev_yylineno = yylineno;                                            \
  }
%}

prep "#"[^\n]*
line_comment \/\/[^\n]*
block_comment \/\*.*\*\/
ws [ \f\n\r\t\v]
letter [a-zA-Z]
l [_a-zA-Z]
d [0-9]
hexd [0-9a-fA-F]
octd [0-7]
ucn \\u{hexd}{4}|\\U{hexd}{8}
punctuator ("{"|"}"|"["|"]"|"("|")"|";"|":"|"..."|"?"|"."|"->"|"~"|"!"|"+"|"-"|"*"|"/"|"%"|"^"|"&"|"|"|"="|"+="|"-="|"*="|"/="|"%="|"^="|"&="|"|="|"=="|"!="|"<"|">"|"<="|">="|"&&"|"||"|"<<"|">>"|"<<="|">>="|"++"|"--"|","|")")
keyword auto|break|case|char|const|continue|default|do|double|else|enum|extern|float|for|goto|if|inline|int|long|register|restrict|return|short|signed|sizeof|static|struct|switch|typedef|union|unsigned|void|volatile|while|_Alignas|_Alignof|_Atomic|_Bool|_Complex|_Generic|_Imaginary|_Noreturn|_Static_assert|_Thread_local
identifier ({l}|{ucn})({l}|{d}|{ucn})*
unsigned_suffix u|u
long_suffix l|L|ll|LL
int_suffix {unsigned_suffix}{long_suffix}?|{long_suffix}{unsigned_suffix}?
decimal_int [1-9]{d}*{int_suffix}?
octal_int 0{octd}+{int_suffix}?
hex_int (0x|0X){hexd}+{int_suffix}?
float_suffix f|F|l|L
decimal_exp (e|E)[+-]?{d}+
decimal_float (({d}*\.{d}+)|({d}+\.){decimal_exp}?|{d}*{decimal_exp}){float_suffix}?
hex_exp (p|P)[+-]?{d}+
hex_float (0x|0X)(({hexd}*\.{hexd}+)|({hexd}+\.?)){hex_exp}{float_suffix}?
escape_sequence \\([abfnrtv'"?]|{octd}{1,3}|x[hexd]+)|{ucn}
c_char [^'\\\n]|{escape_sequence}
char_constant (L|u|U|u8)?'{c_char}+'
s_char [^"\\\n]|{escape_sequence}
string_literal (L|u|U|u8)?\"{s_char}*\"

%%

{ws}|{prep}|{line_comment}|{block_comment} {}
{keyword} { return T_KEYWORD; }
{identifier} { return T_IDENTIFIER; }
{punctuator} { return T_PUNCTUATOR; }
{decimal_int}|{octal_int}|{hex_int} { return T_INTEGER_CONSTANT; }
{decimal_float}|{hex_float} { return T_FLOATING_CONSTANT; }
{char_constant} { return T_CHAR_CONSTANT; }
{string_literal} { return T_STRING_LITERAL; }
. { return T_ERROR; }
