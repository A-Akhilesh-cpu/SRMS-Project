/* student.c
 * Simple role-based Student Management System (text-file backend).
 *
 * Compile:
 *   gcc -std=c11 -Wall -Wextra -o student student.c
 *
 * Notes:
 * - credentials.txt format: username password ROLE   (one record per line)
 * - student.txt format: roll|name|marks   (one record per line; name may contain spaces)
 * - This program is educational; do NOT store plaintext passwords in production.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define STUDENT_FILE      "student.txt"
#define CREDENTIALS_FILE  "credentials.txt"
#define LINE_BUF_SIZE     512
#define NAME_SIZE         128

typedef struct {
    int roll;
    char name[NAME_SIZE];
    float marks;
} Student;

static char currentRole[32] = "";
static char currentUser[64] = "";

/* prototypes */
int  loginSystem(void);
void mainMenu(void);
void adminMenu(void);
void staffMenu(void);
void guestMenu(void);

void addStudent(void);
void displayStudents(void);
void searchStudent(void);
void updateStudent(void);
void deleteStudent(void);

static void trim_newline(char *s);
static int roll_exists(int roll);
static void clear_stdin(void);

/* -------------------- Helpers -------------------- */
static void trim_newline(char *s) {
    if (!s) return;
    size_t n = strlen(s);
    if (n > 0 && s[n-1] == '\n') s[n-1] = '\0';
}

/* Consume remaining characters on stdin until newline or EOF */
static void clear_stdin(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { /* discard */ }
}

/* Check whether a student with given roll exists */
static int roll_exists(int roll) {
    FILE *fp = fopen(STUDENT_FILE, "r");
    if (!fp) return 0; /* no file -> no roll */
    char line[LINE_BUF_SIZE];
    Student st;
    while (fgets(line, sizeof(line), fp)) {
        trim_newline(line);
        /* parse "roll|name|marks" */
        if (sscanf(line, "%d|%127[^|]|%f", &st.roll, st.name, &st.marks) == 3) {
            if (st.roll == roll) {
                fclose(fp);
                return 1;
            }
        }
    }
    fclose(fp);
    return 0;
}

/* -------------------- Login -------------------- */
int loginSystem(void) {
    char username[64], password[64];
    char line[LINE_BUF_SIZE];
    char fileUser[64], filePass[64], fileRole[32];

    printf("========= LOGIN SCREEN =========\n");

    printf("Username: ");
    if (!fgets(username, sizeof(username), stdin)) return 0;
    trim_newline(username);

    printf("Password: ");
    if (!fgets(password, sizeof(password), stdin)) return 0;
    trim_newline(password);

    FILE *fp = fopen(CREDENTIALS_FILE, "r");
    if (!fp) {
        printf("Error: credentials file '%s' not found.\n", CREDENTIALS_FILE);
        printf("Create '%s' with lines like: admin adminpass ADMIN\n", CREDENTIALS_FILE);
        return 0;
    }

    while (fgets(line, sizeof(line), fp)) {
        trim_newline(line);
        if (sscanf(line, "%63s %63s %31s", fileUser, filePass, fileRole) == 3) {
            if (strcmp(username, fileUser) == 0 && strcmp(password, filePass) == 0) {
                strncpy(currentRole, fileRole, sizeof(currentRole)-1);
                currentRole[sizeof(currentRole)-1] = '\0';
                strncpy(currentUser, fileUser, sizeof(currentUser)-1);
                currentUser[sizeof(currentUser)-1] = '\0';
                fclose(fp);
                return 1;
            }
        }
    }

    fclose(fp);
    return 0;
}

/* -------------------- Main Menu Dispatcher -------------------- */
void mainMenu(void) {
    if (strcasecmp(currentRole, "ADMIN") == 0) {
        adminMenu();
    } else if (strcasecmp(currentRole, "STAFF") == 0) {
        staffMenu();
    } else {
        guestMenu();
    }
}

/* -------------------- Menus -------------------- */
void adminMenu(void) {
    int choice = 0;
    do {
        printf("\n====== ADMIN MENU (user: %s role: %s) ======\n", currentUser, currentRole);
        printf("1. Add Student\n");
        printf("2. Display Students\n");
        printf("3. Search Student\n");
        printf("4. Update Student\n");
        printf("5. Delete Student\n");
        printf("6. Logout\n");
        printf("Enter choice: ");
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Try again.\n");
            clear_stdin();
            continue;
        }
        clear_stdin();
        switch (choice) {
            case 1: addStudent();      break;
            case 2: displayStudents(); break;
            case 3: searchStudent();   break;
            case 4: updateStudent();   break;
            case 5: deleteStudent();   break;
            case 6: printf("Logging out...\n"); break;
            default: printf("Invalid choice. Try again.\n");
        }
    } while (choice != 6);
}

void staffMenu(void) {
    int choice = 0;
    do {
        printf("\n====== STAFF MENU (user: %s role: %s) ======\n", currentUser, currentRole);
        printf("1. Display Students\n");
        printf("2. Search Student\n");
        printf("3. Logout\n");
        printf("Enter choice: ");
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Try again.\n");
            clear_stdin();
            continue;
        }
        clear_stdin();
        switch (choice) {
            case 1: displayStudents(); break;
            case 2: searchStudent();   break;
            case 3: printf("Logging out...\n"); break;
            default: printf("Invalid choice. Try again.\n");
        }
    } while (choice != 3);
}

void guestMenu(void) {
    int choice = 0;
    do {
        printf("\n====== GUEST MENU (user: %s role: %s) ======\n", currentUser, currentRole);
        printf("1. Display Students\n");
        printf("2. Logout\n");
        printf("Enter choice: ");
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Try again.\n");
            clear_stdin();
            continue;
        }
        clear_stdin();
        switch (choice) {
            case 1: displayStudents(); break;
            case 2: printf("Logging out...\n"); break;
            default: printf("Invalid choice. Try again.\n");
        }
    } while (choice != 2);
}

/* -------------------- CRUD Operations -------------------- */
void addStudent(void) {
    Student st;
    char line[LINE_BUF_SIZE];

    printf("\nEnter Roll: ");
    if (!fgets(line, sizeof(line), stdin)) return;
    if (sscanf(line, "%d", &st.roll) != 1) {
        printf("Invalid roll number.\n");
        return;
    }

    if (roll_exists(st.roll)) {
        printf("Error: a student with roll %d already exists.\n", st.roll);
        return;
    }

    printf("Enter Name (spaces allowed): ");
    if (!fgets(st.name, sizeof(st.name), stdin)) return;
    trim_newline(st.name);
    /* sanitize delimiter char '|' */
    for (size_t i = 0; i < strlen(st.name); ++i) if (st.name[i] == '|') st.name[i] = ' ';

    printf("Enter Marks: ");
    if (!fgets(line, sizeof(line), stdin)) return;
    if (sscanf(line, "%f", &st.marks) != 1) {
        printf("Invalid marks.\n");
        return;
    }

    FILE *fp = fopen(STUDENT_FILE, "a");
    if (!fp) {
        perror("Failed to open student file for writing");
        return;
    }
    fprintf(fp, "%d|%s|%.2f\n", st.roll, st.name, st.marks);
    fclose(fp);
    printf("Student added successfully.\n");
}

void displayStudents(void) {
    FILE *fp = fopen(STUDENT_FILE, "r");
    if (!fp) {
        printf("No student records found (file '%s' missing or empty).\n", STUDENT_FILE);
        return;
    }

    char line[LINE_BUF_SIZE];
    Student st;
    printf("\n%-6s  %-30s  %6s\n", "Roll", "Name", "Marks");
    printf("--------------------------------------------------------------\n");
    while (fgets(line, sizeof(line), fp)) {
        trim_newline(line);
        if (sscanf(line, "%d|%127[^|]|%f", &st.roll, st.name, &st.marks) == 3) {
            printf("%-6d  %-30s  %6.2f\n", st.roll, st.name, st.marks);
        }
    }
    fclose(fp);
}

void searchStudent(void) {
    char line[LINE_BUF_SIZE];
    Student st;
    int roll;
    printf("Enter Roll to search: ");
    if (!fgets(line, sizeof(line), stdin)) return;
    if (sscanf(line, "%d", &roll) != 1) {
        printf("Invalid roll.\n");
        return;
    }

    FILE *fp = fopen(STUDENT_FILE, "r");
    if (!fp) {
        printf("No student records found.\n");
        return;
    }

    int found = 0;
    while (fgets(line, sizeof(line), fp)) {
        trim_newline(line);
        if (sscanf(line, "%d|%127[^|]|%f", &st.roll, st.name, &st.marks) == 3) {
            if (st.roll == roll) {
                printf("\nRecord Found:\n");
                printf("Roll : %d\nName : %s\nMarks: %.2f\n", st.roll, st.name, st.marks);
                found = 1;
                break;
            }
        }
    }

    if (!found) printf("Record not found for roll %d.\n", roll);
    fclose(fp);
}

void updateStudent(void) {
    char line[LINE_BUF_SIZE];
    Student st;
    int roll;
    printf("Enter Roll to update: ");
    if (!fgets(line, sizeof(line), stdin)) return;
    if (sscanf(line, "%d", &roll) != 1) {
        printf("Invalid roll.\n");
        return;
    }

    FILE *fp = fopen(STUDENT_FILE, "r");
    FILE *temp = fopen("temp.txt", "w");
    if (!fp || !temp) {
        perror("Error opening files for update");
        if (fp) fclose(fp);
        if (temp) fclose(temp);
        return;
    }

    int found = 0;
    while (fgets(line, sizeof(line), fp)) {
        trim_newline(line);
        if (sscanf(line, "%d|%127[^|]|%f", &st.roll, st.name, &st.marks) == 3) {
            if (st.roll == roll) {
                printf("Current Name : %s\n", st.name);
                printf("Current Marks: %.2f\n", st.marks);

                printf("Enter new Name (blank to keep current): ");
                char newName[NAME_SIZE];
                if (!fgets(newName, sizeof(newName), stdin)) newName[0] = '\0';
                trim_newline(newName);
                if (strlen(newName) > 0) {
                    for (size_t i = 0; i < strlen(newName); ++i) if (newName[i] == '|') newName[i] = ' ';
                    strncpy(st.name, newName, NAME_SIZE-1);
                    st.name[NAME_SIZE-1] = '\0';
                }

                printf("Enter new Marks (blank to keep current): ");
                char mline[64];
                if (!fgets(mline, sizeof(mline), stdin)) mline[0] = '\0';
                trim_newline(mline);
                if (strlen(mline) > 0) {
                    float m;
                    if (sscanf(mline, "%f", &m) == 1) st.marks = m;
                    else printf("Invalid marks input; keeping current marks.\n");
                }

                found = 1;
            }
            fprintf(temp, "%d|%s|%.2f\n", st.roll, st.name, st.marks);
        }
    }

    fclose(fp);
    fclose(temp);

    if (remove(STUDENT_FILE) != 0) {
        perror("Warning: failed to remove original student file");
    }
    if (rename("temp.txt", STUDENT_FILE) != 0) {
        perror("Warning: failed to rename temp file to student file");
    }

    if (found) printf("Record updated for roll %d.\n", roll);
    else printf("Record not found for roll %d.\n", roll);
}

void deleteStudent(void) {
    char line[LINE_BUF_SIZE];
    Student st;
    int roll;
    printf("Enter Roll to delete: ");
    if (!fgets(line, sizeof(line), stdin)) return;
    if (sscanf(line, "%d", &roll) != 1) {
        printf("Invalid roll.\n");
        return;
    }

    FILE *fp = fopen(STUDENT_FILE, "r");
    FILE *temp = fopen("temp.txt", "w");
    if (!fp || !temp) {
        perror("Error opening files for delete");
        if (fp) fclose(fp);
        if (temp) fclose(temp);
        return;
    }

    int found = 0;
    while (fgets(line, sizeof(line), fp)) {
        trim_newline(line);
        if (sscanf(line, "%d|%127[^|]|%f", &st.roll, st.name, &st.marks) == 3) {
            if (st.roll == roll) {
                found = 1;
                /* skip writing this record (delete) */
                continue;
            }
            fprintf(temp, "%d|%s|%.2f\n", st.roll, st.name, st.marks);
        }
    }

    fclose(fp);
    fclose(temp);

    if (remove(STUDENT_FILE) != 0) {
        perror("Warning: failed to remove original student file");
    }
    if (rename("temp.txt", STUDENT_FILE) != 0) {
        perror("Warning: failed to rename temp file to student file");
    }

    if (found) printf("Record deleted for roll %d.\n", roll);
    else printf("Record not found for roll %d.\n", roll);
}

/* -------------------- Main -------------------- */
int main(void) {
    if (loginSystem()) {
        mainMenu();
    } else {
        printf("\nLogin failed. Exiting...\n");
    }
    return 0;
}
