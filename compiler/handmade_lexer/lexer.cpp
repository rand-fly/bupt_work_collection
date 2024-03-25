#include "lexer.h"
#include <cassert>
#include <cctype>
#include <iostream>
#include <set>

bool notascii(int c) {
    return c > 127;
}

// consumed "\", peeked "u" or "U"
bool Lexer::getUniversalCharacterName() {
    std::string tmps;
    int c = get();
    assert(c == 'u' || c == 'U');
    tmps += c;
    int len = c == 'u' ? 4 : 8;
    for (int i = 0; i < len; i++) {
        c = peek();
        if (!isxdigit(c)) {
            error("invalid universal character name");
            return false;
        }
        tmps += c;
        get();
    }
    return true;
}
Token Lexer::getToken() {
    enum {
        start,
        block_comment_1,
        block_comment_2,
        line_comment,
        dot,
        two_dot,
        plus,
        minus,
        star,
        slash,
        percent,
        caret,       // ^
        ampersand,   // &
        pipe,        // |
        equal,       // =
        exclamation, // !
        less,        // <
        greater,     // >
        left_shift,  // <<
        right_shift  // >>
    } state = start;
    token_.clear();
    beginLine_ = line_;
    beginRow_ = row_;
    for (;;) {
        int c = peek();
        if (state == start && c == EOF) return Token("EOF", "", 0, 0);
        switch (state) {
        case start:
            if (isspace(c)) {
                pass();
            } else if (isalpha(c) || c == '_' || c == '\\' || notascii(c)) {
                return getIdentifierToken();
            } else if (isdigit(c)) {
                return getNumericToken(false);
            } else {
                switch (c) {
                case '#':
                    pass();
                    state = line_comment;
                    break;
                case '\"':
                    return getStringToken();
                case '\'':
                    return getCharToken();
                case '.':
                    get();
                    state = dot;
                    break;
                case '+':
                    get();
                    state = plus;
                    break;
                case '-':
                    get();
                    state = minus;
                    break;
                case '*':
                    get();
                    state = star;
                    break;
                case '/':
                    get();
                    state = slash;
                    break;
                case '%':
                    get();
                    state = percent;
                    break;
                case '^':
                    get();
                    state = caret;
                    break;
                case '&':
                    get();
                    state = ampersand;
                    break;
                case '|':
                    get();
                    state = pipe;
                    break;
                case '=':
                    get();
                    state = equal;
                    break;
                case '!':
                    get();
                    state = exclamation;
                    break;
                case '<':
                    get();
                    state = less;
                    break;
                case '>':
                    get();
                    state = greater;
                    break;
                case '(':
                case ')':
                case '[':
                case ']':
                case '{':
                case '}':
                case ':':
                case ';':
                case '?':
                case '~':
                case ',':
                    get();
                    return makeToken("punctuator");
                default:
                    get();
                    error(std::string("unknown character ") + (char)c);
                    return makeToken("error");
                }
            }
            break;
        case line_comment:
            if (c == '\n') {
                state = start;
            } else {
                pass();
            }
            break;
        case block_comment_1:
            if (c == '*') {
                pass();
                state = block_comment_2;
            } else if (c == EOF) {
                error("unterminated block comment");
                state = start;
            } else {
                pass();
            }
            break;
        case block_comment_2:
            if (c == '/') {
                pass();
                state = start;
            } else if (c == EOF) {
                error("unterminated block comment");
                return makeToken("error");
            } else {
                pass();
                state = block_comment_1;
            }
            break;
        case dot:
            if (isdigit(c)) {
                return getNumericToken(true);
            } else if (c == '.') {
                get();
                state = two_dot;
            } else {
                return makeToken("punctuator");
            }
            break;
        case two_dot:
            if (c == '.') {
                get();
                return makeToken("punctuator");
            } else {
                error("invalid token ..");
                return makeToken("error");
            }
        case plus:
            if (c == '+' || c == '=') get();
            return makeToken("punctuator");
        case minus:
            if (c == '-' || c == '=' || c == '>') get();
            return makeToken("punctuator");
        case slash:
            if (c == '/') {
                get();
                token_.clear();
                state = line_comment;
            } else if (c == '*') {
                get();
                token_.clear();
                state = block_comment_1;
            } else {
                if (c == '=') get();
                return makeToken("punctuator");
            }
            break;
        case star:
        case percent:
        case caret:
        case equal:
        case left_shift:
        case right_shift:
        case exclamation:
            if (c == '=') get();
            return makeToken("punctuator");
        case ampersand:
            if (c == '&' || c == '=') get();
            return makeToken("punctuator");
        case pipe:
            if (c == '|' || c == '=') get();
            return makeToken("punctuator");
        case greater:
            if (c == '>') {
                get();
                state = right_shift;
            } else {
                if (c == '=') get();
                return makeToken("punctuator");
            }
            break;
        case less:
            if (c == '<') {
                get();
                state = left_shift;
            } else {
                if (c == '=') get();
                return makeToken("punctuator");
            }
            break;
        }
    }
}

void Lexer::printStat() {
    printf("stat:\ntotal: %d characters, %d lines\n", charCount_, line_);
    printf("tokens:\n");
    int sum1 = 0;
    for (auto &kv : stat_) {
        printf("  %s:\n", kv.first.c_str());
        int sum2 = 0;
        for (auto &kv2 : kv.second) {
            printf("    %-30s %d\n", kv2.first.c_str(), kv2.second);
            sum2 += kv2.second;
        }
        printf("    %-30s %d\n", "(total)", sum2);
        sum1 += sum2;
    }
    printf("  %-32s %d\n", "(total)", sum1);
}

// (possibly consumed .) peeked 0-9
Token Lexer::getNumericToken(bool readDot) {
    enum {
        zero,
        int_decimal,
        int_octal,
        int_octal_invalid,
        int_hex_x,
        int_hex,
        float_decimal_fraction,
        float_decimal_e,
        float_decimal_sign,
        float_decimal_exp,
        float_hex_fraction,
        float_hex_p,
        float_hex_sign,
        float_hex_exp,
    } state;
    if (readDot) {
        state = float_decimal_fraction;
    } else if (peek() == '0') {
        state = zero;
    } else {
        state = int_decimal;
    }
    get();
    for (;;) {
        int c = peek();
        switch (state) {
        case zero:
            if (c == 'x' || c == 'X') {
                get();
                state = int_hex_x;
            } else if (c == '.') {
                get();
                state = float_decimal_fraction;
            } else if (c >= '0' && c <= '7') {
                get();
                state = int_octal;
            } else if (c == '8' || c == '9') {
                get();
                state = int_octal_invalid;
            } else if (c == 'e' || c == 'E') {
                get();
                state = float_decimal_e;
            } else {
                if (getIntSuffix()) return makeToken("integer constant");
                else return makeToken("error");
            }
            break;
        case int_decimal:
            if (isdigit(c)) {
                get();
                state = int_decimal;
            } else if (c == '.') {
                get();
                state = float_decimal_fraction;
            } else if (c == 'e' || c == 'E') {
                get();
                state = float_decimal_e;
            } else {
                if (getIntSuffix()) return makeToken("integer constant");
                else return makeToken("error");
            }
            break;
        case int_octal:
            if (c >= '0' && c <= '7') {
                get();
                state = int_octal;
            } else if (c == '8' || c == '9') {
                get();
                state = int_octal_invalid;
            } else if (c == '.') {
                get();
                state = float_decimal_fraction;
            } else if (c == 'e' || c == 'E') {
                get();
                state = float_decimal_e;
            } else {
                if (getIntSuffix()) return makeToken("integer constant");
                else return makeToken("error");
            }
            break;
        case int_octal_invalid:
            if (isdigit(c)) {
                get();
                state = int_octal_invalid;
            } else if (c == '.') {
                get();
                state = float_decimal_fraction;
            } else if (c == 'e' || c == 'E') {
                get();
                state = float_decimal_e;
            } else {
                getIntSuffix();
                error("invalid octal integer constant");
                return makeToken("error");
            }
            break;
        case int_hex_x:
            if (isxdigit(c)) {
                get();
                state = int_hex;
            } else {
                error("invalid hex integer constant");
                return makeToken("error");
            }
            break;
        case int_hex:
            if (isxdigit(c)) {
                get();
                state = int_hex;
            } else if (c == '.') {
                get();
                state = float_hex_fraction;
            } else if (c == 'p' || c == 'P') {
                get();
                state = float_hex_p;
            } else {
                if (getIntSuffix()) return makeToken("integer constant");
                else return makeToken("error");
            }
            break;
        case float_decimal_fraction:
            if (isdigit(c)) {
                get();
                state = float_decimal_fraction;
            } else if (c == 'e' || c == 'E') {
                get();
                state = float_decimal_e;
            } else {
                if (getFloatSuffix()) return makeToken("floating constant");
                else return makeToken("error");
            }
            break;
        case float_decimal_e:
            if (isdigit(c)) {
                get();
                state = float_decimal_exp;
            } else if (c == '+' || c == '-') {
                get();
                state = float_decimal_sign;
            } else {
                error("invalid float number");
                return makeToken("error");
            }
            break;
        case float_decimal_sign:
            if (isdigit(c)) {
                get();
                state = float_decimal_exp;
            } else {
                error("invalid float number");
                return makeToken("error");
            }
            break;
        case float_decimal_exp:
            if (isdigit(c)) {
                get();
                state = float_decimal_exp;
            } else {
                if (getFloatSuffix()) return makeToken("floating constant");
                else return makeToken("error");
            }
            break;
        case float_hex_fraction:
            if (isxdigit(c)) {
                get();
                state = float_hex_fraction;
            } else if (c == 'p' || c == 'P') {
                get();
                state = float_hex_p;
            } else {
                error("invalid hex float number");
                return makeToken("error");
            }
            break;
        case float_hex_p:
            if (isdigit(c)) {
                get();
                state = float_hex_exp;
            } else if (c == '+' || c == '-') {
                get();
                state = float_hex_sign;
            } else {
                error("invalid hex float number");
                return makeToken("error");
            }
            break;
        case float_hex_sign:
            if (isdigit(c)) {
                get();
                state = float_hex_exp;
            } else {
                error("invalid hex float number");
                return makeToken("error");
            }
            break;
        case float_hex_exp:
            if (isdigit(c)) {
                get();
                state = float_hex_exp;
            } else {
                if (getFloatSuffix()) return makeToken("floating constant");
                else return makeToken("error");
            }
            break;
        }
    }
}

Token Lexer::makeToken(const std::string &type) {
    stat_[type][token_]++;
    return Token(type, token_, beginLine_, beginRow_);
}

Token Lexer::getIdentifierToken() {
    int c = peek();
    if (c == 'L' || c == 'u' || c == 'U') {
        get();
        int nx = peek();
        if (nx == '\"') return getStringToken();
        else if (nx == '\'') return getCharToken();
        else if (c == 'u' && nx == '8') {
            get();
            c = peek();
            if (c == '\"') return getStringToken();
            else if (c == '\'') return getCharToken();
        }
    }
    bool valid = true;
    for (;;) {
        int c = peek();
        if (c == '\\') {
            get();
            c = peek();
            if (c == 'u' || c == 'U') {
                if (!getUniversalCharacterName()) valid = false;
            } else {
                error(std::string("expected u or U, got ") + (char)c);
                valid = false;
            }
        } else if (isalnum(c) || c == '_' || notascii(c)) {
            get();
        } else {
            static std::set<std::string> keywords = {
                "auto",           "break",        "case",     "char",     "const",      "continue",
                "default",        "do",           "double",   "else",     "enum",       "extern",
                "float",          "for",          "goto",     "if",       "inline",     "int",
                "long",           "register",     "restrict", "return",   "short",      "signed",
                "sizeof",         "static",       "struct",   "switch",   "typedef",    "union",
                "unsigned",       "void",         "volatile", "while",    "_Alignas",   "_Alignof",
                "_Atomic",        "_Bool",        "_Complex", "_Generic", "_Imaginary", "_Noreturn",
                "_Static_assert", "_Thread_local"};
            if (valid) {
                if (keywords.find(token_) != keywords.end()) return makeToken("keyword");
                else return makeToken("identifier");
            } else {
                return makeToken("error");
            }
        }
    }
}

// peeked "
Token Lexer::getStringToken() {
    bool valid = true;
    get();
    for (;;) {
        int c = peek();
        if (c == '\\') {
            if (!getEscapeSequence()) valid = false;
        } else if (c == '\"') {
            get();
            if (valid) return makeToken("string literal");
            else return makeToken("error string literal");
        } else if (c == '\n' || c == EOF) {
            error("unterminated string literal");
            return makeToken("error");
        } else {
            get();
        }
    }
}

// peeked '
Token Lexer::getCharToken() {
    get();
    if (peek() == '\'') {
        get();
        error("empty char constant");
        return makeToken("error");
    }
    for (;;) {
        int c = peek();
        if (c == '\\') {
            if (!getEscapeSequence()) return makeToken("error");
        } else if (c == '\'') {
            get();
            return makeToken("char constant");
        } else if (c == '\n' || c == EOF) {
            error("unterminated char constant");
            return makeToken("error");
        } else {
            get();
        }
    }
}

bool Lexer::getIntSuffix() {
    std::string suffix;
    int c;
    while (c = peek(), isalpha(c)) {
        get();
        suffix += c;
    }
    static std::set<std::string> valid_suffix = {
        "",   "u",  "U",  "l",   "L",   "ll",  "LL",  "ul",  "uL",  "Ul",  "UL", "lu",
        "lU", "Lu", "LU", "ull", "uLL", "Ull", "ULL", "llu", "llU", "LLu", "LLU"};

    if (valid_suffix.find(suffix) == valid_suffix.end()) {
        error("invalid integer suffix " + suffix);
        return false;
    }
    return true;
}

bool Lexer::getFloatSuffix() {
    std::string suffix;
    int c;
    while (c = peek(), isalpha(c)) {
        get();
        suffix += c;
    }
    if (suffix != "" && suffix != "f" && suffix != "F" && suffix != "l" && suffix != "L") {
        error("invalid float number suffix " + suffix);
        return false;
    }
    return true;
}

// peeked "\"
bool Lexer::getEscapeSequence() {
    int c = get();
    assert(c == '\\');
    c = peek();
    // octal escape sequence
    if (c >= '0' && c <= '7') {
        get();
        for (int i = 0; i < 2; i++) {
            c = peek();
            if (c >= '0' && c <= '7') {
                get();
            } else {
                break;
            }
        }
        return true;
    }
    // hex escape sequence
    if (c == 'x') {
        get();
        while (isxdigit(peek())) {
            get();
        }
        return true;
    }
    if (c == 'u' || c == 'U') {
        return getUniversalCharacterName();
    }
    switch (c) {
    case '\'':
    case '\"':
    case '?':
    case '\\':
    case 'a':
    case 'b':
    case 'f':
    case 'n':
    case 'r':
    case 't':
    case 'v':
        get();
        return true;
    default:
        get();
        warning(std::string("unknown escape sequence \\") + (char)c);
        return true;
    }
}

int Lexer::peek() {
    return in_.peek();
}

int Lexer::get() {
    int c = in_.get();
    if (c != EOF) {
        charCount_++;
        if (c == '\n') {
            line_++;
            row_ = 1;
        } else {
            row_++;
        }
        token_.push_back(c);
    }
    return c;
}

int Lexer::pass() {
    int c = in_.get();
    if (c != EOF) {
        charCount_++;
        if (c == '\n') {
            line_++;
            row_ = 1;
        } else {
            row_++;
        }
    }
    beginLine_ = line_;
    beginRow_ = row_;
    return c;
}

void Lexer::error(const std::string &msg) {
    std::cerr << line_ << ":" << row_ << ": ";
    if (colored_) std::cerr << "\033[1;31m";
    std::cerr << "error: ";
    if (colored_) std::cerr << "\033[0m";
    std::cerr << msg << std::endl;
}

void Lexer::warning(const std::string &msg) {
    std::cerr << line_ << ":" << row_ << ": ";
    if (colored_) std::cerr << "\033[1;33m";
    std::cerr << "warning: ";
    if (colored_) std::cerr << "\033[0m";
    std::cerr << msg << std::endl;
}
