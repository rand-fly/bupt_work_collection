#include <cstdio>
#include <map>
#include <string>

const char *token_names[] = {"EOF",
                             "error",
                             "identifier",
                             "keyword",
                             "punctuator",
                             "integer constant",
                             "floating constant",
                             "char constant",
                             "string literal"};

int yylex(void);
void yyset_in(FILE *_in_str);
extern char *yytext;
extern int start_line, start_column, char_cnt;

std::map<std::string, std::map<std::string, int>> stat;

int main(int argc, char **argv) {
    FILE *fin;
    if (argc > 1) {
        fin = fopen(argv[1], "r");
        if (fin == NULL) {
            perror("cannot open file");
            return 1;
        }
        yyset_in(fin);
    }
    int token;
    while ((token = yylex()) != 0) {
        const char *type = token_names[token];
        stat[type][yytext]++;
        printf("%d:%d: <%s, %s>\n", start_line, start_column, type, yytext);
    }
    printf("\n");
    printf("stat:\ntotal: %d characters, %d lines\n", char_cnt, start_line);
    printf("tokens:\n");
    int sum1 = 0;
    for (auto &kv : stat) {
        printf("  %s:\n", kv.first.c_str());
        int sum2 = 0;
        for (auto &kv2 : kv.second) {
            printf("    %-20s %d\n", kv2.first.c_str(), kv2.second);
            sum2 += kv2.second;
        }
        printf("    %-20s %d\n", "(total)", sum2);
        sum1 += sum2;
    }
    printf("  %-22s %d\n", "(total)", sum1);
    if (fin) {
        fclose(fin);
    }
    return 0;
}
