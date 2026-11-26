#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

struct User {
    int id;
    char role[20];
    char name[50];
    char email[60];
    char password[64];
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

struct User *getUsers(void) {
    return users;
}

struct Book *getBooks(void) {
    return books;
}

int getUserCount(void) {
    return userCount;
}

int getBookCount(void) {
    return bookCount;
}

void setUserCount(int count) {
    userCount = count;
}

void setBookCount(int count) {
    bookCount = count;
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
    FILE *file;
    char fields[7][128];
    char line[1024];

    userCount = 0;
    file = fopen("users.csv", "r");
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
            strlen(fields[3]) >= sizeof(users[0].password) ||
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
            if (*fields[5] == '\0' || *end != '\0' ||
                questionId < 0 || questionId > 6) {
                continue;
            }
        }

        users[userCount].id = (int)id;
        strcpy(users[userCount].name, fields[1]);
        strcpy(users[userCount].email, fields[2]);
        strcpy(users[userCount].password, fields[3]);
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
    FILE *file;
    int i;

    file = fopen("users.csv", "w");
    if (file == NULL) {
        printf("Cannot open users.csv\n");
        return;
    }
    if (fprintf(file, "id,name,email,password,role,securityQuestionId,securityAnswer\n") < 0) {
        printf("Cannot write users.csv\n");
        fclose(file);
        return;
    }

    for (i = 0; i < userCount; i++) {
        if (fprintf(file, "%d,", users[i].id) < 0 ||
            !writeCsvField(file, users[i].name) || fputc(',', file) == EOF ||
            !writeCsvField(file, users[i].email) || fputc(',', file) == EOF ||
            !writeCsvField(file, users[i].password) || fputc(',', file) == EOF ||
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
    FILE *file;
    int id;
    char title[50];
    char author[50];
    char category[30];
    int copies;

    bookCount = 0;
    file = fopen("books.txt", "r");
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
    FILE *file;
    int i;

    file = fopen("books.txt", "w");
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
    int i;
    int found = 0;

    loadUsers();
    for (i = 0; i < userCount; i++) {
        if (strcmp(users[i].email, "admin@diu.edu.bd") == 0) {
            found = 1;
        }
    }

    if (found == 0) {
        users[userCount].id = 1;
        strcpy(users[userCount].name, "Admin");
        strcpy(users[userCount].email, "admin@diu.edu.bd");
        strcpy(users[userCount].password, "Admin@123");
        strcpy(users[userCount].role, "Admin");
        users[userCount].securityQuestionId = 0;
        users[userCount].securityAnswer[0] = '\0';
        userCount++;
        saveUsers();
    }
}

int checkEmail(char email[]) {
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

int findUser(char email[]) {
    int i;

    for (i = 0; i < userCount; i++) {
        if (strcmp(users[i].email, email) == 0) {
            return i;
        }
    }
    return -1;
}
