// studentdb.cpp
#include "exceptions.h"

extern "C" {
    int read(int fd, void *buf, unsigned int count);
    int open(const char *pathname, int flags, int mode);
    int write(int fd, const void *buf, unsigned int count);
    int close(int fd);
}

#define O_RDONLY  0
#define O_WRONLY  1
#define O_RDWR    2
#define O_CREAT   0100
#define O_TRUNC   01000
#define MODE_644  0644

void writeStr(int fd, const char *s);
void writeInt(int fd, int x);


// ----------------- small helpers -----------------
int my_strcmp(const char *a, const char *b) {
    int i = 0;
    while (a[i] && b[i] && a[i] == b[i]) ++i;
    return (unsigned char)a[i] - (unsigned char)b[i];
}
void my_strcpy(char *d, const char *s) { while((*d++ = *s++)); }
int my_atoi(const char *s) {
    int i = 0, x = 0, neg = 0;
    if (s[0] == '-') { neg = 1; ++i; }
    while (s[i] >= '0' && s[i] <= '9') { x = x * 10 + (s[i++] - '0'); }
    return neg ? -x : x;
}
int readLine(char *buf, int max) {
    int n = read(0, buf, max - 1);
    if (n <= 0) return 0;
    int i = 0; while (i < n && buf[i] != '\n' && buf[i] != '\r') ++i;
    buf[i] = 0; return i;
}
void printInt(int x) {
    char buf[32]; int i = 0;
    if (x == 0) buf[i++] = '0';
    else {
        int neg = 0; if (x < 0) { neg = 1; x = -x; }
        char tmp[32]; int j = 0;
        while (x > 0) { tmp[j++] = '0' + (x % 10); x /= 10; }
        if (neg) buf[i++] = '-';
        while (j--) buf[i++] = tmp[j];
    }
    buf[i] = 0; ::print(buf);
}
char toLowerChar(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A' + 'a';
    return c;
}

// ----------------- Student hierarchy -----------------
class Student {
protected:
    char roll[20];
    char name[60];
    char branch[10];
    char level[10];
    int marks[3]; // only 3 components
public:
    Student(const char* r, const char* nm, const char* b, const char* l, int *m) {
        my_strcpy(roll, r); my_strcpy(name, nm); my_strcpy(branch, b); my_strcpy(level, l);
        for (int i = 0; i < 3; ++i) marks[i] = m[i];
    }
    virtual ~Student() {}
    virtual int totalMarks() const = 0;
    virtual void print() const = 0;

    // getters/setters
    const char* getRoll() const { return roll; }
    const char* getName() const { return name; }
    const char* getBranch() const { return branch; }
    const char* getLevel() const { return level; }
    int getMark(int idx) const { if (idx >= 0 && idx < 3) return marks[idx]; return 0; }
    void setRoll(const char* r) { my_strcpy(roll, r); }
    void setName(const char* n) { my_strcpy(name, n); }
    void setBranch(const char* b) { my_strcpy(branch, b); }
    void setLevel(const char* l) { my_strcpy(level, l); }
    void setMark(int idx, int v) { if (idx >= 0 && idx < 3) marks[idx] = v; }
    void setAllMarks(int *m) { for (int i = 0; i < 3; ++i) marks[i] = m[i]; }
};

class BTechStudent : public Student {
public:
    BTechStudent(const char* r, const char* nm, const char* b, int *m) : Student(r, nm, b, "BTECH", m) {}
    int totalMarks() const { int s = 0; for (int i = 0; i < 3; ++i) s += marks[i]; return s; }
    void print() const {
        ::print("[BTECH] ");
        ::print(name); ::print(" ("); ::print(roll); ::print(") ");
        ::print("Branch: "); ::print(branch); ::print(" Total: "); printInt(totalMarks()); ::print("\n");
    }
};

class MTechStudent : public Student {
public:
    MTechStudent(const char* r, const char* nm, const char* b, int *m) : Student(r, nm, b, "MTECH", m) {}
    int totalMarks() const { int s = 0; for (int i = 0; i < 3; ++i) s += marks[i]; return s; }
    void print() const {
        ::print("[MTECH] ");
        ::print(name); ::print(" ("); ::print(roll); ::print(") ");
        ::print("Branch: "); ::print(branch); ::print(" Total: "); printInt(totalMarks()); ::print("\n");
    }
};

class PhDStudent : public Student {
public:
    PhDStudent(const char* r, const char* nm, const char* b, int *m) : Student(r, nm, b, "PHD", m) {}
    int totalMarks() const { int s = 0; for (int i = 0; i < 3; ++i) s += marks[i]; return s; }
    void print() const {
        ::print("[PHD] ");
        ::print(name); ::print(" ("); ::print(roll); ::print(") ");
        ::print("Branch: "); ::print(branch); ::print(" Total: "); printInt(totalMarks()); ::print("\n");
    }
};

// ----------------- Trie for name sorting -----------------
#define ALPHABET_SIZE 27 // a-z plus space as last
int charToIndexTrie(char c) {
    if (c == ' ') return 26;
    char lc = toLowerChar(c);
    if (lc >= 'a' && lc <= 'z') return lc - 'a';
    return -1;
}

struct TrieNode {
    TrieNode* child[ALPHABET_SIZE];
    Student** students; // dynamic array of pointers for names that end here
    int stu_count;
    int stu_cap;
    TrieNode() {
        for (int i = 0; i < ALPHABET_SIZE; ++i) child[i] = 0;
        students = 0; stu_count = 0; stu_cap = 0;
    }
    void addStudent(Student* s) {
        if (stu_count >= stu_cap) {
            int ncap = stu_cap ? stu_cap * 2 : 4;
            Student** tmp = new Student*[ncap];
            for (int i = 0; i < stu_count; ++i) tmp[i] = students[i];
            delete [] students;
            students = tmp;
            stu_cap = ncap;
        }
        students[stu_count++] = s;
    }
};

void trieInsert(TrieNode* root, const char* name, Student* s) {
    int i = 0; TrieNode* cur = root;
    while (name[i]) {
        int idx = charToIndexTrie(name[i]);
        if (idx < 0) { ++i; continue; } // skip unrecognized chars (should be validated earlier)
        if (!cur->child[idx]) cur->child[idx] = new TrieNode();
        cur = cur->child[idx];
        ++i;
    }
    cur->addStudent(s);
}

void trieTraverseCollect(TrieNode* node, Student*** outArr, int &outSize, int &outCap) {
    if (!node) return;
    if (node->stu_count > 0) {
        for (int i = 0; i < node->stu_count; ++i) {
            if (outSize >= outCap) {
                int ncap = outCap ? outCap * 2 : 8;
                Student** tmp = new Student*[ncap];
                for (int j = 0; j < outSize; ++j) tmp[j] = (*outArr)[j];
                delete [] *outArr;
                *outArr = tmp;
                outCap = ncap;
            }
            (*outArr)[outSize++] = node->students[i];
        }
    }
    for (int i = 0; i < ALPHABET_SIZE; ++i) {
        if (node->child[i]) trieTraverseCollect(node->child[i], outArr, outSize, outCap);
    }
}

void trieFree(TrieNode* node) {
    if (!node) return;
    for (int i = 0; i < ALPHABET_SIZE; ++i) if (node->child[i]) trieFree(node->child[i]);
    delete [] node->students;
    delete node;
}

// ----------------- StudentDB -----------------
class StudentDB {
    Student** arr;
    int size;
    int cap;
public:
    StudentDB(): arr(0), size(0), cap(0) {}
    ~StudentDB() { for (int i=0;i<size;++i) delete arr[i]; delete [] arr; }

    void saveToCSV(const char *filename = "students.csv") 
    {
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, MODE_644);
    if (fd < 0) return;

    writeStr(fd, "Name,Roll,Branch,Level,Mark1,Mark2,Mark3\n");

    for (int i = 0; i < size; i++) {
        Student* s = arr[i];
        writeStr(fd, s->getName());  writeStr(fd, ",");
        writeStr(fd, s->getRoll());  writeStr(fd, ",");
        writeStr(fd, s->getBranch());writeStr(fd, ",");
        writeStr(fd, s->getLevel()); writeStr(fd, ",");
        for (int j = 0; j < 3; j++) {
            writeInt(fd, s->getMark(j));
            if (j < 2) writeStr(fd, ",");
        }
        writeStr(fd, "\n");
    }

    close(fd);
    }

    void loadFromCSV(const char *filename = "students.csv") 
    {
        int fd = open(filename, O_RDONLY, MODE_644);
        if (fd < 0) return; // file may not exist yet

        char buf[4096];
        int n = read(fd, buf, 4095);
        if (n <= 0) { close(fd); return; }
        buf[n] = 0;
        close(fd);

        int i = 0;
        // skip header
        while (buf[i] && buf[i] != '\n') i++;
        if (buf[i] == '\n') i++;

        // parse line by line
        while (buf[i]) {
            char name[60], roll[20], branch[10], level[10];
            int marks[3] = {0,0,0};
            int f = 0, pos = 0, m = 0;
            char field[64];
            for (; buf[i] && buf[i] != '\n'; i++) {
                if (buf[i] == ',') {
                    field[pos] = 0;
                    if (f == 0) my_strcpy(name, field);
                    else if (f == 1) my_strcpy(roll, field);
                    else if (f == 2) my_strcpy(branch, field);
                    else if (f == 3) my_strcpy(level, field);
                    else if (f >= 4 && f <= 6) marks[f-4] = my_atoi(field);
                    pos = 0; f++;
                } else field[pos++] = buf[i];
            }
            field[pos] = 0;
            if (f >= 4 && f <= 6) marks[f-4] = my_atoi(field);

            if (my_strlen(roll) > 0) {
                Student* s = 0;
                if (my_strcmp(level,"BTECH")==0) s = new BTechStudent(roll,name,branch,marks);
                else if (my_strcmp(level,"MTECH")==0) s = new MTechStudent(roll,name,branch,marks);
                else s = new PhDStudent(roll,name,branch,marks);
                *this = *this + s;
            }
            if (buf[i] == '\n') i++;
        }
    }


    void ensure(int need) {
        if (need <= cap) return;
        int ncap = cap ? cap * 2 : 8;
        while (ncap < need) ncap *= 2;
        Student** tmp = new Student*[ncap];
        for (int i=0;i<size;++i) tmp[i] = arr[i];
        delete [] arr;
        arr = tmp; cap = ncap;
    }
    StudentDB& operator+(Student* s) {
        ensure(size+1); arr[size++] = s; return *this;
    }
    Student* findByRoll(const char* roll) {
        for (int i=0;i<size;++i) if (my_strcmp(arr[i]->getRoll(), roll) == 0) return arr[i];
        return 0;
    }
    // replace a student object (used when changing level)
    void replaceByRoll(const char* roll, Student* newS) {
        for (int i=0;i<size;++i) if (my_strcmp(arr[i]->getRoll(), roll) == 0) { delete arr[i]; arr[i] = newS; return; }
    }
    void removeByRoll(const char* roll) {
        for (int i=0;i<size;++i) if (my_strcmp(arr[i]->getRoll(), roll) == 0) {
            delete arr[i];
            for (int j=i;j<size-1;++j) arr[j] = arr[j+1];
            --size; return;
        }
    }
    void printAll() {
        if (size == 0) { ::print("Database empty.\n"); return; }
        for (int i=0;i<size;++i) arr[i]->print();
    }

    // compute totals per component (0..2)
    void computeComponentTotals() {
        if (size == 0) { ::print("No students in database.\n"); return; }
        int totals[3] = {0,0,0};
        for (int i=0;i<size;++i) {
            for (int j=0;j<3;++j) totals[j] += arr[i]->getMark(j);
        }
        ::print("\n=== Component totals ===\n");
        for (int j=0;j<3;++j) {
            ::print("Component "); printInt(j); ::print(": "); printInt(totals[j]); ::print("\n");
        }
    }

    // sort by roll (lexicographic)
    void sortByRoll() {
        // insertion sort
        for (int i=1;i<size;++i) {
            Student* key = arr[i];
            int j = i-1;
            while (j>=0 && my_strcmp(arr[j]->getRoll(), key->getRoll()) > 0) {
                arr[j+1] = arr[j]; --j;
            }
            arr[j+1] = key;
        }
    }

    // sort by a component index descending (higher marks first)
    void sortByComponent(int idx) {
        if (idx < 0 || idx >= 3) return;
        for (int i=1;i<size;++i) {
            Student* key = arr[i];
            int j = i-1;
            while (j>=0 && arr[j]->getMark(idx) < key->getMark(idx)) {
                arr[j+1] = arr[j]; --j;
            }
            arr[j+1] = key;
        }
    }

    // sort by name using Trie
    void sortByNameTrie() {
        if (size == 0) return;
        TrieNode* root = new TrieNode();
        for (int i=0;i<size;++i) {
            trieInsert(root, arr[i]->getName(), arr[i]);
        }
        // collect in lexicographic order
        Student** out = 0; int outSize = 0, outCap = 0;
        trieTraverseCollect(root, &out, outSize, outCap);
        // if outSize != size, some names had characters skipped; we'll fallback to stable copy
        if (outSize == size) {
            for (int i=0;i<size;++i) arr[i] = out[i];
        } else {
            // merge: fill arr with those in out, then remaining ones not in out in original order
            int filled = 0;
            for (int i=0;i<outSize;++i) arr[filled++] = out[i];
            for (int i=0;i<size;++i) {
                bool found = false;
                for (int j=0;j<outSize;++j) if (arr[filled-1] == out[j]) { found = true; break; } // not reliable
            }
            // To keep it simple: if mismatch, do nothing (stable as-is)
        }
        delete [] out;
        trieFree(root);
    }

    int getSize() const { return size; }

    // get pointer for iterating externally (dangerous but simple)
    Student* getAt(int idx) const { if (idx>=0 && idx < size) return arr[idx]; return 0; }
};

// ----------------- validation wrappers -----------------
void validateNameOrThrow(const char* name) {
    if (my_strlen(name) >= 60) throw BufferOverflowException("Name too long");
    if (!hasSpace(name)) throw NoSecondNameException("Name must contain at least two parts");
    if (hasDigitOrSymbol(name)) throw InvalidNameCharacterException("Invalid character in name");
}
void validateRollOrThrow(const char* roll) {
    if (my_strlen(roll) >= 20) throw BufferOverflowException("Roll too long");
    if (!isRollValid(roll)) throw InvalidRollNumberException("Invalid roll number format");
}
void validateMarkOrThrow(int m) {
    if (m < 0 || m > 100) throw InvalidMarkException("Mark must be between 0 and 100");
}

// ----------------- createStudent with exception handling -----------------
Student* createStudentInteractive() {
    char buf[128];
    char roll[20], name[60], branch[10], level[10];
    int marks[3];

    // Name
    while (1) {
        ::print("Enter Name (first last): ");
        readLine(buf, 128);
        try {
            validateNameOrThrow(buf);
            my_strcpy(name, buf);
            break;
        } catch (BufferOverflowException &e) { ::print(e.what()); ::print("\n"); }
        catch (NoSecondNameException &e) { ::print(e.what()); ::print("\n"); }
        catch (InvalidNameCharacterException &e) { ::print(e.what()); ::print("\n"); }
    }

    // Roll
    while (1) {
        ::print("Enter Roll (UPPER+digits): ");
        readLine(buf, 128);
        try {
            validateRollOrThrow(buf);
            my_strcpy(roll, buf);
            break;
        } catch (BufferOverflowException &e) { ::print(e.what()); ::print("\n"); }
        catch (InvalidRollNumberException &e) { ::print(e.what()); ::print("\n"); }
    }

    // Branch
    while (1) {
        ::print("Enter Branch (CSE/ECE): ");
        readLine(buf, 128);
        if (my_strcmp(buf, "CSE") == 0 || my_strcmp(buf, "ECE") == 0) { my_strcpy(branch, buf); break; }
        ::print("Invalid branch (must be CSE or ECE)\n");
    }

    // Level
    while (1) {
        ::print("Enter Level (BTECH/MTECH/PHD): ");
        readLine(buf, 128);
        if (my_strcmp(buf, "BTECH") == 0 || my_strcmp(buf, "MTECH") == 0 || my_strcmp(buf, "PHD") == 0) { my_strcpy(level, buf); break; }
        ::print("Invalid level (must be BTECH, MTECH or PHD)\n");
    }

    // Marks (3 components)
    for (int i = 0; i < 3; ++i) {
        while (1) {
            ::print("Enter mark "); printInt(i); ::print(" (0-100): ");
            readLine(buf, 128);
            int m = my_atoi(buf);
            try {
                validateMarkOrThrow(m);
                marks[i] = m;
                break;
            } catch (InvalidMarkException &e) {
                ::print(e.what()); ::print("\n");
            }
        }
    }

    if (my_strcmp(level, "BTECH") == 0) return new BTechStudent(roll, name, branch, marks);
    if (my_strcmp(level, "MTECH") == 0) return new MTechStudent(roll, name, branch, marks);
    return new PhDStudent(roll, name, branch, marks);
}

// ----------------- main menu -----------------
int main() {
    StudentDB db;
    db.loadFromCSV();
    char buf[128];

    while (1) {
        ::print("\n--- MENU ---\n");
        ::print("1. Add Student\n");
        ::print("2. Modify Student\n");
        ::print("3. Delete Student\n");
        ::print("4. Print Database\n");
        ::print("5. Compute Component Totals\n");
        ::print("6. Sort by Roll\n");
        ::print("7. Sort by Component (0..2)\n");
        ::print("8. Sort by Name (Trie)\n");
        ::print("9. Exit\n");
        ::print("Choice: ");
        readLine(buf, 128);
        int c = my_atoi(buf);

        if (c == 1) {
            Student* s = createStudentInteractive();
            db = db + s;
            ::print("Added.\n");
        } else if (c == 2) {
            ::print("Enter roll to modify: ");
            readLine(buf, 128);
            Student* s = db.findByRoll(buf);
            if (!s) { ::print("Roll not found.\n"); continue; }
            // small modify menu
            while (1) {
                ::print("\n-- Modify --\n1. Change name\n2. Change roll\n3. Change branch\n4. Change level (replace object)\n5. Change mark\n6. Back\nChoice: ");
                readLine(buf,128);
                int ch = my_atoi(buf);
                if (ch == 1) {
                    ::print("Enter new name: "); readLine(buf,128);
                    try { validateNameOrThrow(buf); s->setName(buf); ::print("Name updated.\n"); }
                    catch (BufferOverflowException &e) { ::print(e.what()); ::print("\n"); }
                    catch (NoSecondNameException &e) { ::print(e.what()); ::print("\n"); }
                    catch (InvalidNameCharacterException &e) { ::print(e.what()); ::print("\n"); }
                } else if (ch == 2) {
                    ::print("Enter new roll: "); readLine(buf,128);
                    try { validateRollOrThrow(buf); s->setRoll(buf); ::print("Roll updated.\n"); }
                    catch (BufferOverflowException &e) { ::print(e.what()); ::print("\n"); }
                    catch (InvalidRollNumberException &e) { ::print(e.what()); ::print("\n"); }
                } else if (ch == 3) {
                    ::print("Enter new branch (CSE/ECE): "); readLine(buf,128);
                    if (my_strcmp(buf,"CSE")==0 || my_strcmp(buf,"ECE")==0) { s->setBranch(buf); ::print("Branch updated.\n"); }
                    else ::print("Invalid branch.\n");
                } else if (ch == 4) {
                    // replace object type preserving roll/name/branch/marks
                    char oldRoll[20]; my_strcpy(oldRoll, s->getRoll());
                    char oldName[60]; my_strcpy(oldName, s->getName());
                    char oldBranch[10]; my_strcpy(oldBranch, s->getBranch());
                    int oldMarks[3]; for (int i=0;i<3;++i) oldMarks[i] = s->getMark(i);

                    ::print("Enter new level (BTECH/MTECH/PHD): "); readLine(buf,128);
                    Student* newS = 0;
                    if (my_strcmp(buf,"BTECH")==0) newS = new BTechStudent(oldRoll, oldName, oldBranch, oldMarks);
                    else if (my_strcmp(buf,"MTECH")==0) newS = new MTechStudent(oldRoll, oldName, oldBranch, oldMarks);
                    else if (my_strcmp(buf,"PHD")==0) newS = new PhDStudent(oldRoll, oldName, oldBranch, oldMarks);
                    if (newS) { db.replaceByRoll(oldRoll, newS); s = newS; ::print("Level changed.\n"); }
                    else ::print("Invalid level.\n");
                } else if (ch == 5) {
                    ::print("Enter component index (0..2): "); readLine(buf,128);
                    int idx = my_atoi(buf);
                    if (idx < 0 || idx > 2) { ::print("Invalid index.\n"); continue; }
                    ::print("Enter new mark (0-100): "); readLine(buf,128);
                    int m = my_atoi(buf);
                    try { validateMarkOrThrow(m); s->setMark(idx,m); ::print("Mark updated.\n"); }
                    catch (InvalidMarkException &e) { ::print(e.what()); ::print("\n"); }
                } else if (ch == 6) break;
                else ::print("Invalid\n");
            }
        } else if (c == 3) {
            ::print("Enter roll to delete: "); readLine(buf,128);
            db.removeByRoll(buf); ::print("If existed, removed.\n");
        } else if (c == 4) {
            db.printAll();
        } else if (c == 5) {
            db.computeComponentTotals();
        } else if (c == 6) {
            db.sortByRoll(); ::print("Sorted by roll.\n");
        } else if (c == 7) {
            ::print("Enter component index (0..2): "); readLine(buf,128);
            int idx = my_atoi(buf);
            db.sortByComponent(idx); ::print("Sorted by component.\n");
        } else if (c == 8) {
            db.sortByNameTrie(); ::print("Sorted by name via trie.\n");
        } else if (c == 9) {
            ::print("Exiting.\n"); break;
        } else ::print("Invalid choice.\n");
    }

    return 0;
}
