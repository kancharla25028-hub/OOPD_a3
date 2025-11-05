// exceptions.h
#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

extern "C" {
    int write(int fd, const void *buf, unsigned int count);
}

int my_strlen(const char *s);
void print(const char *s);

// ========== Exceptions ==========
class BufferOverflowException {
    const char* msg;
public:
    BufferOverflowException(const char* m): msg(m) {}
    const char* what() const { return msg; }
};

class NoSecondNameException {
    const char* msg;
public:
    NoSecondNameException(const char* m): msg(m) {}
    const char* what() const { return msg; }
};

class InvalidNameCharacterException {
    const char* msg;
public:
    InvalidNameCharacterException(const char* m): msg(m) {}
    const char* what() const { return msg; }
};

class InvalidRollNumberException {
    const char* msg;
public:
    InvalidRollNumberException(const char* m): msg(m) {}
    const char* what() const { return msg; }
};

class InvalidMarkException {
    const char* msg;
public:
    InvalidMarkException(const char* m): msg(m) {}
    const char* what() const { return msg; }
};

// ========== Validation prototypes ==========
int hasSpace(const char* s);
int hasDigitOrSymbol(const char* s);
int isRollValid(const char* s);

#endif
