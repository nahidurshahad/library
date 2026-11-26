#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sodium.h>
#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

struct User {
    int id;
    char role[20];
    char name[50];
    char email[60];
    char passwordHash[crypto_pwhash_argon2id_STRBYTES];
    int securityQuestionId;
    char securityAnswer[64];
};

struct Book {
    int id;
    char title[50];
    char author[50];
    char category[30];
    int copies;
};

struct User users[100];
struct Book books[100];
int userCount = 0;
int bookCount = 0;

int readPassword(const char prompt[], char password[], size_t size) {
    char showOption;
    int character;
    int overflow = 0;
    size_t length = 0;
#ifndef _WIN32
    struct termios originalMode;
    struct termios inputMode;
    int terminalInput = isatty(STDIN_FILENO);
#endif

    printf("Show password while typing? (y/N): ");
    if (scanf(" %c", &showOption) != 1) {
        return 0;
    }
    while ((character = getchar()) != '\n' && character != EOF) {
    }
    showOption = tolower((unsigned char)showOption) == 'y' ? 'y' : 'n';

#ifndef _WIN32
    if (terminalInput) {
        if (tcgetattr(STDIN_FILENO, &originalMode) != 0) {
            printf("Could not configure password input.\n");
            return 0;
        }
        inputMode = originalMode;
        inputMode.c_lflag &= (tcflag_t)~(ICANON | ECHO);
        inputMode.c_cc[VMIN] = 1;
        inputMode.c_cc[VTIME] = 0;
        if (tcsetattr(STDIN_FILENO, TCSANOW, &inputMode) != 0) {
            printf("Could not configure password input.\n");
            return 0;
        }
    }
#endif

    printf("%s", prompt);
    fflush(stdout);
    for (;;) {
#ifdef _WIN32
        character = _getch();
        if (character == 0 || character == 224) {
            (void)_getch();
            continue;
        }
#else
        character = getchar();
#endif
        if (character == '\r' || character == '\n' || character == EOF) {
            break;
        }
        if (character == '\b' || character == 127) {
            if (length > 0) {
                length--;
                printf("\b \b");
                fflush(stdout);
            }
            continue;
        }
        if (length + 1 < size) {
            password[length++] = (char)character;
            putchar(showOption == 'y' ? character : '*');
            fflush(stdout);
        } else {
            overflow = 1;
        }
    }

#ifndef _WIN32
    if (terminalInput &&
        tcsetattr(STDIN_FILENO, TCSANOW, &originalMode) != 0) {
        password[0] = '\0';
        printf("\nCould not restore terminal settings.\n");
        return 0;
    }
#endif
    printf("\n");
    if (character == EOF || overflow) {
        password[0] = '\0';
        if (overflow) {
            printf("Password exceeds the maximum length.\n");
        }
        return 0;
    }
    password[length] = '\0';
    return 1;
}

int isStrongPassword(const char password[]) {
    int hasUpper = 0;
    int hasLower = 0;
    int hasDigit = 0;
    int hasSpecial = 0;
    int i;

    if (strlen(password) < 8) {
        return 0;
    }

    for (i = 0; password[i] != '\0'; i++) {
        unsigned char character = (unsigned char)password[i];
        if (isupper(character)) hasUpper = 1;
        if (islower(character)) hasLower = 1;
        if (isdigit(character)) hasDigit = 1;
        if (ispunct(character)) hasSpecial = 1;
    }

    return hasUpper && hasLower && hasDigit && hasSpecial;
}

/* Returns 1 if the stored text is an Argon2id hash. */
int isPasswordHash(const char stored[]) {
    return strncmp(stored, crypto_pwhash_argon2id_STRPREFIX,
                   strlen(crypto_pwhash_argon2id_STRPREFIX)) == 0;
}

/* Hashes a password with Argon2id. The salt is random and kept inside the hash. */
int hashPassword(const char password[], char hash[]) {
    if (crypto_pwhash_argon2id_str(hash, password, strlen(password),
                                   crypto_pwhash_argon2id_OPSLIMIT_INTERACTIVE,
                                   crypto_pwhash_argon2id_MEMLIMIT_INTERACTIVE) != 0) {
        printf("Could not hash password (out of memory).\n");
        return 0;
    }
    return 1;
}

/* Returns 1 if the password matches the stored Argon2id hash. */
int verifyPassword(const char hash[], const char password[]) {
    return isPasswordHash(hash) &&
           crypto_pwhash_argon2id_str_verify(hash, password,
                                             strlen(password)) == 0;
}

const char *getSecurityQuestion(int questionId) {
    static const char *questions[] = {
        "What was the name of your first pet?",
        "What was the name of your first school?",
        "In what city were you born?",
        "What is your mother's maiden name?",
        "What was your childhood nickname?",
        "Who was your favorite teacher?"
    };

    if (questionId < 1 || questionId > 6) {
        return NULL;
    }
    return questions[questionId - 1];
}

void printSecurityQuestions(void) {
    int i;

    for (i = 1; i <= 6; i++) {
        printf("%d. %s\n", i, getSecurityQuestion(i));
    }
}

int setSecurityQuestion(int *questionId, char answer[]) {
    char input[64];
    int choice;
    int i;

    printf("\nChoose a security question:\n");
    printSecurityQuestions();
    printf("Enter choice: ");
    if (scanf("%d", &choice) != 1 || getSecurityQuestion(choice) == NULL) {
        printf("Invalid security question.\n");
        return 0;
    }

    printf("Answer (one word, no spaces): ");
    if (scanf("%63s", input) != 1) {
        printf("Could not read security answer.\n");
        return 0;
    }

    for (i = 0; input[i] != '\0'; i++) {
        answer[i] = (char)tolower((unsigned char)input[i]);
    }
    answer[i] = '\0';
    *questionId = choice;
    return 1;
}

int securityAnswersMatch(const char stored[], const char provided[]) {
    int i;

    for (i = 0; stored[i] != '\0' && provided[i] != '\0'; i++) {
        if (tolower((unsigned char)stored[i]) !=
            tolower((unsigned char)provided[i])) {
            return 0;
        }
    }
    return stored[i] == '\0' && provided[i] == '\0';
}

static int parseCsvFields(const char *line, char fields[7][128]) {
    const char *cursor = line;
    int fieldCount;

    for (fieldCount = 0; fieldCount < 7; fieldCount++) {
        size_t length = 0;
        int quoted = *cursor == '"';
        int closed = !quoted;

        if (quoted) cursor++;
        while (*cursor != '\0') {
            if (quoted && *cursor == '"') {
                if (cursor[1] == '"') {
                    if (length >= sizeof(fields[fieldCount]) - 1) return 0;
                    fields[fieldCount][length++] = '"';
                    cursor += 2;
                    continue;
                }
                cursor++;
                closed = 1;
                break;
            }
            if (!quoted &&
                (*cursor == ',' || *cursor == '\r' || *cursor == '\n')) {
                break;
            }
            if (!quoted && *cursor == '"') return 0;
            if (length >= sizeof(fields[fieldCount]) - 1) return 0;
            fields[fieldCount][length++] = *cursor++;
        }
        if (!closed) return 0;
        fields[fieldCount][length] = '\0';

        if (quoted && *cursor != ',' && *cursor != '\r' &&
            *cursor != '\n' && *cursor != '\0') {
            return 0;
        }
        if (*cursor != ',') {
            while (*cursor == '\r' || *cursor == '\n') cursor++;
            return *cursor == '\0' ? fieldCount + 1 : 0;
        }
        cursor++;
    }
    return 0;
}

static int writeCsvField(FILE *file, const char *value) {
    const char *character;
    int quote = strpbrk(value, ",\"\r\n") != NULL;

    if (quote && fputc('"', file) == EOF) return 0;
    for (character = value; *character != '\0'; character++) {
        if (*character == '"' && fputc('"', file) == EOF) return 0;
        if (fputc(*character, file) == EOF) return 0;
    }
    return !quote || fputc('"', file) != EOF;
}

void loadUsers(void) {
    FILE *file = fopen("users.csv", "r");
    char fields[7][128];
    char line[1024];

    userCount = 0;
    if (file == NULL) {
        return;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        int fieldCount = parseCsvFields(line, fields);
        char *end;
        long id;
        long questionId = 0;

        if (fieldCount == 7 && strcmp(fields[0], "id") == 0) {
            continue;
        }
        if ((fieldCount != 5 && fieldCount != 7) || userCount >= 100 ||
            strlen(fields[1]) >= sizeof(users[0].name) ||
            strlen(fields[2]) >= sizeof(users[0].email) ||
            strlen(fields[3]) >= sizeof(users[0].passwordHash) ||
            strlen(fields[4]) >= sizeof(users[0].role) ||
            (fieldCount == 7 &&
             strlen(fields[6]) >= sizeof(users[0].securityAnswer))) {
            continue;
        }
        id = strtol(fields[0], &end, 10);
        if (*fields[0] == '\0' || *end != '\0' || id < 0 || id > 100) {
            continue;
        }
        if (fieldCount == 7) {
            questionId = strtol(fields[5], &end, 10);
            if (*fields[5] == '\0' || *end != '\0' || questionId < 0 ||
                questionId > 6) {
                continue;
            }
        }

        users[userCount].id = (int)id;
        strcpy(users[userCount].name, fields[1]);
        strcpy(users[userCount].email, fields[2]);
        strcpy(users[userCount].passwordHash, fields[3]);
        strcpy(users[userCount].role, fields[4]);
        users[userCount].securityQuestionId = (int)questionId;
        if (fieldCount == 7 && strcmp(fields[6], "-") != 0) {
            strcpy(users[userCount].securityAnswer, fields[6]);
        } else {
            users[userCount].securityAnswer[0] = '\0';
        }
        userCount++;
    }
    fclose(file);
}

void saveUsers(void) {
    FILE *file = fopen("users.csv", "w");
    int i;

    if (file == NULL) {
        printf("Cannot open users.csv\n");
        return;
    }
    if (fprintf(file, "id,name,email,passwordHash,role,securityQuestionId,securityAnswer\n") < 0) {
        printf("Cannot write users.csv\n");
        fclose(file);
        return;
    }

    for (i = 0; i < userCount; i++) {
        if (fprintf(file, "%d,", users[i].id) < 0 ||
            !writeCsvField(file, users[i].name) || fputc(',', file) == EOF ||
            !writeCsvField(file, users[i].email) || fputc(',', file) == EOF ||
            !writeCsvField(file, users[i].passwordHash) || fputc(',', file) == EOF ||
            !writeCsvField(file, users[i].role) ||
            fprintf(file, ",%d,", users[i].securityQuestionId) < 0 ||
            !writeCsvField(file, users[i].securityAnswer[0] != '\0' ?
                           users[i].securityAnswer : "-") ||
            fputc('\n', file) == EOF) {
            printf("Cannot write users.csv\n");
            fclose(file);
            return;
        }
    }
    fclose(file);
}

void loadBooks(void) {
    FILE *file = fopen("books.txt", "r");
    int id;
    char title[50];
    char author[50];
    char category[30];
    int copies;

    bookCount = 0;
    if (file == NULL) {
        return;
    }

    while (fscanf(file, "%d %49s %49s %29s %d",
                  &id, title, author, category, &copies) == 5) {
        books[bookCount].id = id;
        strcpy(books[bookCount].title, title);
        strcpy(books[bookCount].author, author);
        strcpy(books[bookCount].category, category);
        books[bookCount].copies = copies;
        bookCount++;
        if (bookCount >= 100) {
            break;
        }
    }
    fclose(file);
}

void saveBooks(void) {
    FILE *file = fopen("books.txt", "w");
    int i;

    if (file == NULL) {
        printf("Cannot open books.txt\n");
        return;
    }

    for (i = 0; i < bookCount; i++) {
        fprintf(file, "%d %s %s %s %d\n",
                books[i].id, books[i].title, books[i].author,
                books[i].category, books[i].copies);
    }
    fclose(file);
}

void createAdmin(void) {
    char hash[crypto_pwhash_argon2id_STRBYTES];
    int i;
    int found = 0;
    int changed = 0;

    loadUsers();
    for (i = 0; i < userCount; i++) {
        if (strcmp(users[i].email, "admin@diu.edu.bd") == 0) {
            found = 1;
        }
        /* Plaintext password from an older users.csv: replace it with a hash. */
        if (!isPasswordHash(users[i].passwordHash) &&
            hashPassword(users[i].passwordHash, hash)) {
            strcpy(users[i].passwordHash, hash);
            changed = 1;
        }
    }

    if (found == 0 && userCount < 100 && hashPassword("Admin@123", hash)) {
        users[userCount].id = 1;
        strcpy(users[userCount].name, "Admin");
        strcpy(users[userCount].email, "admin@diu.edu.bd");
        strcpy(users[userCount].passwordHash, hash);
        strcpy(users[userCount].role, "Admin");
        users[userCount].securityQuestionId = 0;
        users[userCount].securityAnswer[0] = '\0';
        userCount++;
        changed = 1;
    }

    if (changed) {
        saveUsers();
    }
}

int findUser(char email[]) {
    int i;

    for (i = 0; i < userCount; i++) {
        if (strcmp(users[i].email, email) == 0) {
            return i;
        }
    }
    return -1;
}

int validEmail(char email[]) {
    int length = strlen(email);
    int domainLength = strlen("@diu.edu.bd");

    if (length <= domainLength) {
        return 0;
    }
    if (strcmp(email + length - domainLength, "@diu.edu.bd") == 0) {
        return 1;
    }
    return 0;
}

void registration(void) {
    char password[64];
    char confirm[64];
    int role;
    int character;

    loadUsers();
    if (userCount >= 100) {
        printf("User limit reached.\n");
        return;
    }

    printf("\n===== REGISTRATION =====\n");
    printf("Select role:\n");
    printf("1. Student\n");
    printf("2. Teacher\n");
    printf("3. Librarian\n");
    printf("Enter choice: ");
    scanf("%d", &role);

    if (role == 1) {
        strcpy(users[userCount].role, "Student");
    } else if (role == 2) {
        strcpy(users[userCount].role, "Teacher");
    } else if (role == 3) {
        strcpy(users[userCount].role, "Librarian");
    } else {
        printf("Invalid role.\n");
        return;
    }

    printf("Enter name: ");
    if (scanf(" %49[^\t\r\n]", users[userCount].name) != 1) {
        printf("Invalid name.\n");
        return;
    }
    while ((character = getchar()) != '\n' && character != EOF) {
    }
    printf("Enter DIU email: ");
    scanf("%59s", users[userCount].email);

    if (validEmail(users[userCount].email) == 0) {
        printf("Invalid email.\n");
        return;
    }
    if (findUser(users[userCount].email) != -1) {
        printf("Email already registered.\n");
        return;
    }

    if (!readPassword("Enter password: ", password, sizeof(password))) {
        return;
    }
    if (!isStrongPassword(password)) {
        printf("Weak password. Try a stronger password.\n");
        return;
    }
    if (!readPassword("Confirm password: ", confirm, sizeof(confirm))) {
        return;
    }
    if (strcmp(password, confirm) != 0) {
        printf("Passwords do not match.\n");
        return;
    }

    if (!hashPassword(password, users[userCount].passwordHash)) {
        return;
    }
    if (!setSecurityQuestion(&users[userCount].securityQuestionId,
                             users[userCount].securityAnswer)) {
        return;
    }
    users[userCount].id = userCount + 1;
    userCount++;
    saveUsers();
    printf("Registration successful.\n");
}

void forgotPassword(void) {
    char email[60];
    char password[64];
    char confirm[64];
    int index;

    loadUsers();
    printf("Enter email: ");
    scanf("%59s", email);
    index = findUser(email);
    if (index == -1) {
        printf("Account not found.\n");
        return;
    }

    if (getSecurityQuestion(users[index].securityQuestionId) == NULL) {
        printf("No security question is set. Sign in to set one before resetting your password.\n");
        return;
    }
    printf("Security question: %s\n",
           getSecurityQuestion(users[index].securityQuestionId));
    printf("Answer (one word, no spaces): ");
    if (scanf("%63s", confirm) != 1 ||
        !securityAnswersMatch(users[index].securityAnswer, confirm)) {
        printf("Security answer does not match.\n");
        return;
    }

    if (!readPassword("Enter new password: ", password, sizeof(password))) {
        return;
    }
    if (!isStrongPassword(password)) {
        printf("Weak password. Try a stronger password.\n");
        return;
    }
    if (!readPassword("Confirm new password: ", confirm, sizeof(confirm))) {
        return;
    }
    if (strcmp(password, confirm) != 0) {
        printf("Passwords do not match.\n");
        return;
    }
    if (!hashPassword(password, users[index].passwordHash)) {
        return;
    }
    saveUsers();
    printf("Password changed successfully.\n");
}

void viewUsers(void) {
    int i;
    loadUsers();
    printf("\nID\tName\tEmail\tRole\n");
    for (i = 0; i < userCount; i++) {
        printf("%d\t%s\t%s\t%s\n",
               users[i].id, users[i].name,
               users[i].email, users[i].role);
    }
}

void addBook(void) {
    loadBooks();
    if (bookCount >= 100) {
        printf("Book limit reached.\n");
        return;
    }

    books[bookCount].id = bookCount + 1;
    printf("Enter title (no spaces): ");
    scanf("%49s", books[bookCount].title);
    printf("Enter author (no spaces): ");
    scanf("%49s", books[bookCount].author);
    printf("Enter category (no spaces): ");
    scanf("%29s", books[bookCount].category);
    printf("Enter copies: ");
    scanf("%d", &books[bookCount].copies);
    if (books[bookCount].copies < 0) {
        printf("Invalid copies.\n");
        return;
    }

    bookCount++;
    saveBooks();
    printf("Book added successfully.\n");
}

void updateBook(void) {
    int id;
    int i;

    loadBooks();
    printf("Enter book ID: ");
    scanf("%d", &id);
    for (i = 0; i < bookCount; i++) {
        if (books[i].id == id) {
            printf("Enter new title: ");
            scanf("%49s", books[i].title);
            printf("Enter new author: ");
            scanf("%49s", books[i].author);
            printf("Enter new category: ");
            scanf("%29s", books[i].category);
            printf("Enter new copies: ");
            scanf("%d", &books[i].copies);
            if (books[i].copies < 0) {
                printf("Invalid copies.\n");
                return;
            }
            saveBooks();
            printf("Book updated successfully.\n");
            return;
        }
    }
    printf("Book not found.\n");
}

void deleteBook(void) {
    int id;
    int i;
    int j;

    loadBooks();
    printf("Enter book ID: ");
    scanf("%d", &id);
    for (i = 0; i < bookCount; i++) {
        if (books[i].id == id) {
            for (j = i; j < bookCount - 1; j++) {
                books[j] = books[j + 1];
            }
            bookCount--;
            saveBooks();
            printf("Book deleted successfully.\n");
            return;
        }
    }
    printf("Book not found.\n");
}

void viewBooks(void) {
    int i;

    loadBooks();
    printf("\n===== BOOK LIST =====\n");
    printf("ID\tTitle\tAuthor\tCategory\tCopies\n");
    for (i = 0; i < bookCount; i++) {
        printf("%d\t%s\t%s\t%s\t%d\n",
               books[i].id, books[i].title, books[i].author,
               books[i].category, books[i].copies);
    }
}

void searchBook(void) {
    char text[50];
    int choice;
    int i;
    int found = 0;

    loadBooks();
    printf("Search by 1-ID 2-Title 3-Author 4-Category: ");
    scanf("%d", &choice);
    printf("Enter search text: ");
    scanf("%49s", text);

    for (i = 0; i < bookCount; i++) {
        if ((choice == 1 && books[i].id == atoi(text)) ||
            (choice == 2 && strstr(books[i].title, text) != NULL) ||
            (choice == 3 && strstr(books[i].author, text) != NULL) ||
            (choice == 4 && strstr(books[i].category, text) != NULL)) {
            printf("ID: %d, Title: %s, Author: %s, Category: %s, Copies: %d\n",
                   books[i].id, books[i].title, books[i].author,
                   books[i].category, books[i].copies);
            found = 1;
        }
    }
    if (found == 0) {
        printf("Book not found.\n");
    }
}

void adminDashboard(void) {
    int choice;
    do {
        printf("\n===== ADMIN DASHBOARD =====\n");
        printf("1. View users\n");
        printf("2. Add book\n");
        printf("3. Update book\n");
        printf("4. Delete book\n");
        printf("5. View books\n");
        printf("6. Search book\n");
        printf("0. Logout\n");
        printf("Enter choice: ");
        scanf("%d", &choice);
        if (choice == 1) viewUsers();
        if (choice == 2) addBook();
        if (choice == 3) updateBook();
        if (choice == 4) deleteBook();
        if (choice == 5) viewBooks();
        if (choice == 6) searchBook();
        if (choice == 0) printf("Admin logged out.\n");
    } while (choice != 0);
}

void librarianDashboard(void) {
    int choice;
    do {
        printf("\n===== LIBRARIAN DASHBOARD =====\n");
        printf("1. Add book\n");
        printf("2. Update book\n");
        printf("3. Delete book\n");
        printf("4. View books\n");
        printf("5. Search book\n");
        printf("0. Logout\n");
        printf("Enter choice: ");
        scanf("%d", &choice);
        if (choice == 1) addBook();
        if (choice == 2) updateBook();
        if (choice == 3) deleteBook();
        if (choice == 4) viewBooks();
        if (choice == 5) searchBook();
        if (choice == 0) printf("Librarian logged out.\n");
    } while (choice != 0);
}

void studentDashboard(void) {
    int choice;
    do {
        printf("\n===== STUDENT DASHBOARD =====\n");
        printf("1. View books\n");
        printf("2. Search book\n");
        printf("0. Logout\n");
        printf("Enter choice: ");
        scanf("%d", &choice);
        if (choice == 1) viewBooks();
        if (choice == 2) searchBook();
        if (choice == 0) printf("Student logged out.\n");
    } while (choice != 0);
}

void teacherDashboard(void) {
    int choice;
    do {
        printf("\n===== TEACHER DASHBOARD =====\n");
        printf("1. View books\n");
        printf("2. Search book\n");
        printf("0. Logout\n");
        printf("Enter choice: ");
        scanf("%d", &choice);
        if (choice == 1) viewBooks();
        if (choice == 2) searchBook();
        if (choice == 0) printf("Teacher logged out.\n");
    } while (choice != 0);
}

void login(void) {
    char email[60];
    char password[64];
    char roleName[20];
    int role;
    int index;

    loadUsers();
    printf("\n===== LOGIN =====\n");
    printf("Select role:\n");
    printf("1. Admin\n");
    printf("2. Librarian\n");
    printf("3. Student\n");
    printf("4. Teacher\n");
    printf("Enter role: ");
    scanf("%d", &role);

    if (role == 1) strcpy(roleName, "Admin");
    if (role == 2) strcpy(roleName, "Librarian");
    if (role == 3) strcpy(roleName, "Student");
    if (role == 4) strcpy(roleName, "Teacher");
    if (role < 1 || role > 4) {
        printf("Invalid role.\n");
        return;
    }

    printf("Enter email: ");
    scanf("%59s", email);
    if (!readPassword("Enter password: ", password, sizeof(password))) {
        return;
    }

    index = findUser(email);
    if (index == -1) {
        printf("Account not found.\n");
        return;
    }
    if (strcmp(users[index].role, roleName) != 0) {
        printf("Role does not match.\n");
        return;
    }
    if (!verifyPassword(users[index].passwordHash, password)) {
        printf("Wrong password.\n");
        return;
    }

    if (getSecurityQuestion(users[index].securityQuestionId) == NULL) {
        printf("Set a security question for password recovery.\n");
        if (!setSecurityQuestion(&users[index].securityQuestionId,
                                 users[index].securityAnswer)) {
            return;
        }
        saveUsers();
    }

    printf("Login successful. Welcome %s!\n", users[index].name);
    if (role == 1) adminDashboard();
    if (role == 2) librarianDashboard();
    if (role == 3) studentDashboard();
    if (role == 4) teacherDashboard();
}

int main(void) {
    int choice;

    if (sodium_init() < 0) {
        printf("Could not start libsodium.\n");
        return 1;
    }
    createAdmin();

    do {
        printf("\n=================================\n");
        printf(" DIU LIBRARY MANAGEMENT SYSTEM\n");
        printf("=================================\n");
        printf("1. Login\n");
        printf("2. Registration\n");
        printf("3. Forgot Password\n");
        printf("0. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        if (choice == 1) login();
        if (choice == 2) registration();
        if (choice == 3) forgotPassword();
        if (choice == 0) printf("Thank you.\n");
        if (choice < 0 || choice > 3) printf("Invalid choice.\n");
    } while (choice != 0);

    return 0;
}
