/*
 * PBL: Structure-oriented Persistent Data Management
 * Student, Course Allocation and Result Management System (C)
 *
 * Data structures:
 *   - struct Student   : ID, NAME, FEES, course, result  (singly linked list in memory)
 *   - struct Course    : code, name  (static lookup table: DS, AI, ML, Web)
 *   - struct User      : username + hashed password (login system)
 * Persistence:
 *   - students.dat (binary records), users.dat (binary records)
 *
 * Rules:
 *   - A course can be allotted only after fees >= MIN_FEES are paid.
 *   - A result can be declared only after a course is allotted.
 *
 * Compile : gcc student_mgmt.c -o student_mgmt
 * Default login (created on first run): admin / admin123
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

#define NAME_LEN   50
#define USER_LEN   20
#define MIN_FEES   5000.0f
#define STUD_FILE  "students.dat"
#define USER_FILE  "users.dat"

/* ---------- Data definitions ---------- */
typedef enum { C_NONE = 0, C_DS, C_AI, C_ML, C_WEB } CourseId;
typedef enum { R_NONE = 0, R_EXCELLENT, R_GOOD, R_FAIL } ResultId;

static const char *COURSE_NAME[] = { "Not Allotted", "DS", "AI", "ML", "Web" };
static const char *RESULT_NAME[] = { "Pending", "Excellent", "Good", "Fail" };

typedef struct {            /* record stored on disk */
    int      id;
    char     name[NAME_LEN];
    float    fees;
    int      course;        /* CourseId */
    int      result;        /* ResultId */
} StudentRec;

typedef struct Node {       /* in-memory linked list node */
    StudentRec data;
    struct Node *next;
} Node;

typedef uint32_t HashValue;

typedef struct {
    char username[USER_LEN];
    HashValue hash;
} User;

static Node *head = NULL;

/* ---------- Input helpers ---------- */
static void readLine(const char *prompt, char *buf, int n) {
    printf("%s", prompt);
    if (!fgets(buf, n, stdin)) { buf[0] = '\0'; return; }
    buf[strcspn(buf, "\r\n")] = '\0';
}

static int readInt(const char *prompt) {
    char b[64]; char *end;
    for (;;) {
        readLine(prompt, b, sizeof b);
        long v = strtol(b, &end, 10);
        if (end != b && *end == '\0') return (int)v;
        printf("Invalid number, try again.\n");
    }
}

static float readFloat(const char *prompt) {
    char b[64]; char *end;
    for (;;) {
        readLine(prompt, b, sizeof b);
        float v = strtof(b, &end);
        if (end != b && *end == '\0' && v >= 0) return v;
        printf("Invalid amount, try again.\n");
    }
}

/* ---------- Password hashing (djb2) ---------- */
static HashValue hashPwd(const char *s) {
    HashValue h = 5381u;
    while (*s) h = (HashValue)((h * 33u) + (unsigned char)*s++);
    return h;
}

/* ---------- Users ---------- */
static void ensureDefaultUser(void) {
    FILE *f = fopen(USER_FILE, "rb");
    if (f) { fclose(f); return; }

    f = fopen(USER_FILE, "wb");
    if (!f) return;

    User u; memset(&u, 0, sizeof u);
    strncpy(u.username, "admin", sizeof(u.username) - 1);
    u.username[sizeof(u.username) - 1] = '\0';
    u.hash = hashPwd("admin123");
    fwrite(&u, sizeof u, 1, f);
    fclose(f);
}

static int login(void) {
    for (int attempt = 1; attempt <= 3; attempt++) {
        char un[USER_LEN + 10], pw[64];
        readLine("Username: ", un, sizeof un);
        readLine("Password: ", pw, sizeof pw);

        FILE *f = fopen(USER_FILE, "rb");
        User u;
        int ok = 0;
        if (f) {
            while (fread(&u, sizeof u, 1, f) == 1) {
                if (strcmp(u.username, un) == 0 && u.hash == hashPwd(pw)) {
                    ok = 1;
                    break;
                }
            }
            fclose(f);
        }

        if (ok) {
            printf("Login successful. Welcome, %s!\n", un);
            return 1;
        }
        printf("Invalid credentials (%d/3 attempts used).\n", attempt);
    }
    printf("Too many failed attempts. Exiting.\n");
    return 0;
}

static void addUser(void) {
    User u; char pw[64];
    memset(&u, 0, sizeof u);
    readLine("New username: ", u.username, USER_LEN);
    if (strlen(u.username) == 0) { printf("Username cannot be empty.\n"); return; }
    FILE *f = fopen(USER_FILE, "rb");
    if (f) {
        User t;
        while (fread(&t, sizeof t, 1, f) == 1)
            if (strcmp(t.username, u.username) == 0) {
                fclose(f); printf("User already exists.\n"); return;
            }
        fclose(f);
    }
    readLine("New password: ", pw, sizeof pw);
    if (strlen(pw) < 4) { printf("Password must be at least 4 characters.\n"); return; }
    u.hash = hashPwd(pw);
    f = fopen(USER_FILE, "ab");
    if (!f) { printf("Cannot open user file.\n"); return; }
    fwrite(&u, sizeof u, 1, f);
    fclose(f);
    printf("User '%s' created.\n", u.username);
}

/* ---------- Persistence for students ---------- */
static Node *findStudent(int id) {
    for (Node *p = head; p; p = p->next) if (p->data.id == id) return p;
    return NULL;
}

static void appendNode(StudentRec r) {
    Node *n = malloc(sizeof *n);
    if (!n) { printf("Out of memory.\n"); exit(1); }
    n->data = r; n->next = NULL;
    if (!head) head = n;
    else { Node *p = head; while (p->next) p = p->next; p->next = n; }
}

static void loadStudents(void) {
    FILE *f = fopen(STUD_FILE, "rb");
    if (!f) return;
    StudentRec r;
    while (fread(&r, sizeof r, 1, f) == 1) appendNode(r);
    fclose(f);
}

static void saveStudents(void) {
    FILE *f = fopen(STUD_FILE, "wb");
    if (!f) { printf("Error: cannot write %s\n", STUD_FILE); return; }
    for (Node *p = head; p; p = p->next) fwrite(&p->data, sizeof p->data, 1, f);
    fclose(f);
}

static void freeList(void) {
    while (head) { Node *t = head; head = head->next; free(t); }
}

/* ---------- Operations ---------- */
static void printHeader(void) {
    printf("%-6s %-20s %-10s %-14s %-10s\n", "ID", "NAME", "FEES", "COURSE", "RESULT");
    printf("------------------------------------------------------------------\n");
}
static void printRow(const StudentRec *s) {
    printf("%-6d %-20s %-10.2f %-14s %-10s\n", s->id, s->name, s->fees,
           COURSE_NAME[s->course], RESULT_NAME[s->result]);
}

static void registerStudent(void) {
    StudentRec r; memset(&r, 0, sizeof r);
    r.id = readInt("Student ID: ");
    if (r.id <= 0) { printf("ID must be positive.\n"); return; }
    if (findStudent(r.id)) { printf("Error: ID %d already exists.\n", r.id); return; }
    readLine("Name: ", r.name, NAME_LEN);
    if (strlen(r.name) == 0) { printf("Name cannot be empty.\n"); return; }
    r.fees = readFloat("Fees paid: ");
    appendNode(r);
    saveStudents();
    printf("Student registered successfully.\n");
}

static void viewAll(void) {
    if (!head) { printf("No student records found.\n"); return; }
    printHeader();
    for (Node *p = head; p; p = p->next) printRow(&p->data);
}

static void searchStudent(void) {
    Node *n = findStudent(readInt("Enter ID to search: "));
    if (!n) { printf("Student not found.\n"); return; }
    printHeader(); printRow(&n->data);
}

static void payFees(void) {
    Node *n = findStudent(readInt("Enter ID: "));
    if (!n) { printf("Student not found.\n"); return; }
    float amt = readFloat("Amount to pay: ");
    n->data.fees += amt;
    saveStudents();
    printf("Fees updated. Total paid: %.2f\n", n->data.fees);
}

static void allotCourse(void) {
    Node *n = findStudent(readInt("Enter ID: "));
    if (!n) { printf("Student not found.\n"); return; }
    if (n->data.fees < MIN_FEES) {
        printf("Cannot allot course: fees paid %.2f is below minimum %.2f.\n",
               n->data.fees, MIN_FEES);
        return;
    }
    if (n->data.course != C_NONE) {
        printf("Student already allotted to %s.\n", COURSE_NAME[n->data.course]);
        return;
    }
    printf("1. DS  2. AI  3. ML  4. Web\n");
    int c = readInt("Choose course: ");
    if (c < 1 || c > 4) { printf("Invalid course.\n"); return; }
    n->data.course = c;
    saveStudents();
    printf("Course %s allotted to %s.\n", COURSE_NAME[c], n->data.name);
}

static void declareResult(void) {
    Node *n = findStudent(readInt("Enter ID: "));
    if (!n) { printf("Student not found.\n"); return; }
    if (n->data.course == C_NONE) { printf("No course allotted yet; cannot declare result.\n"); return; }
    printf("1. Excellent  2. Good  3. Fail\n");
    int r = readInt("Choose result: ");
    if (r < 1 || r > 3) { printf("Invalid result.\n"); return; }
    n->data.result = r;
    saveStudents();
    printf("Result '%s' declared for %s.\n", RESULT_NAME[r], n->data.name);
}

static void deleteStudent(void) {
    int id = readInt("Enter ID to delete: ");
    Node *prev = NULL, *p = head;
    while (p && p->data.id != id) { prev = p; p = p->next; }
    if (!p) { printf("Student not found.\n"); return; }
    if (prev) prev->next = p->next; else head = p->next;
    free(p);
    saveStudents();
    printf("Student %d deleted.\n", id);
}

static void courseReport(void) {
    int cnt[5] = {0}, res[5][4] = {{0}};
    for (Node *p = head; p; p = p->next) {
        cnt[p->data.course]++;
        res[p->data.course][p->data.result]++;
    }
    printf("%-14s %-9s %-10s %-6s %-6s %-8s\n", "COURSE", "STUDENTS", "EXCELLENT", "GOOD", "FAIL", "PENDING");
    printf("------------------------------------------------------------\n");
    for (int c = 0; c < 5; c++)
        printf("%-14s %-9d %-10d %-6d %-6d %-8d\n", COURSE_NAME[c], cnt[c],
               res[c][R_EXCELLENT], res[c][R_GOOD], res[c][R_FAIL], res[c][R_NONE]);
}

/* ---------- Main menu ---------- */
int main(void) {
    printf("=== Student Course & Result Management System ===\n");
    ensureDefaultUser();
    if (!login()) return 1;
    loadStudents();

    int ch;
    do {
        printf("\n------------- MENU -------------\n"
               "1. Register student\n"
               "2. View all students\n"
               "3. Search student by ID\n"
               "4. Pay / update fees\n"
               "5. Allot course\n"
               "6. Declare result\n"
               "7. Delete student\n"
               "8. Course-wise report\n"
               "9. Add new user (login)\n"
               "0. Exit\n");
        ch = readInt("Enter choice: ");
        switch (ch) {
            case 1: registerStudent(); break;
            case 2: viewAll();         break;
            case 3: searchStudent();   break;
            case 4: payFees();         break;
            case 5: allotCourse();     break;
            case 6: declareResult();   break;
            case 7: deleteStudent();   break;
            case 8: courseReport();    break;
            case 9: addUser();         break;
            case 0: saveStudents(); printf("Data saved. Goodbye!\n"); break;
            default: printf("Invalid choice.\n");
        }
    } while (ch != 0);

    freeList();
    return 0;
}
