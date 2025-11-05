// exceptions.cpp
#include "exceptions.h"

// system call declared in header: write(1,...)

int my_strlen(const char *s) {
    int n = 0;
    while (s && s[n]) ++n;
    return n;
}

void print(const char *s) {
    if (!s) return;
    write(1, s, my_strlen(s));
}

// Validation utilities
int hasSpace(const char* s) {
    for (int i = 0; s[i]; ++i) if (s[i] == ' ') return 1;
    return 0;
}

int hasDigitOrSymbol(const char* s) {
    for (int i = 0; s[i]; ++i) {
        char c = s[i];
        if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == ' ')) return 1;
    }
    return 0;
}

// Roll allowed: uppercase letters and digits only (you can loosen if required)
int isRollValid(const char* s) {
    for (int i = 0; s[i]; ++i) {
        char c = s[i];
        if (!((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))) return 0;
    }
    return 1;
}
