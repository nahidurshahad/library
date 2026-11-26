#include "library.h"

struct User users[100];
struct Book books[100];
int userCount = 0;
int bookCount = 0;

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
             strlen(fields[6]) >= sizeof(users[0].securityAnswerHash))) {
            continue;
        }
        id = strtol(fields[0], &end, 10);
        if (*fields[0] == '\0' || *end != '\0' || id < 1 || id > INT_MAX) {
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
            strcpy(users[userCount].securityAnswerHash, fields[6]);
        } else {
            users[userCount].securityAnswerHash[0] = '\0';
        }
        userCount++;
    }
    fclose(file);
}

void saveUsers(void) {
    FILE *file = fopen("users.csv.tmp", "w");
    int i;
    if (file == NULL) {
        printf("Cannot create temporary users file.\n");
        return;
    }
    if (fprintf(file, "id,name,email,passwordHash,role,securityQuestionId,securityAnswerHash\n") < 0) {
        fclose(file);
        remove("users.csv.tmp");
        printf("Cannot write users file.\n");
        return;
    }
    for (i = 0; i < userCount; i++) {
        if (fprintf(file, "%d,", users[i].id) < 0 ||
            !writeCsvField(file, users[i].name) || fputc(',', file) == EOF ||
            !writeCsvField(file, users[i].email) || fputc(',', file) == EOF ||
            !writeCsvField(file, users[i].passwordHash) || fputc(',', file) == EOF ||
            !writeCsvField(file, users[i].role) ||
            fprintf(file, ",%d,", users[i].securityQuestionId) < 0 ||
            !writeCsvField(file, users[i].securityAnswerHash[0] != '\0' ?
                           users[i].securityAnswerHash : "-") ||
            fputc('\n', file) == EOF) {
            fclose(file);
            remove("users.csv.tmp");
            printf("Cannot write users file.\n");
            return;
        }
    }
    if (fflush(file) != 0) {
        fclose(file);
        remove("users.csv.tmp");
        printf("Cannot finish writing users file.\n");
        return;
    }
    if (fclose(file) != 0) {
        remove("users.csv.tmp");
        printf("Cannot close temporary users file.\n");
        return;
    }
#ifdef _WIN32
    if (!MoveFileExA("users.csv.tmp", "users.csv",
                     MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
#else
    if (rename("users.csv.tmp", "users.csv") != 0) {
#endif
        remove("users.csv.tmp");
        printf("Cannot safely replace users.csv.\n");
    }
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

    while (bookCount < 100 &&
           fscanf(file, "%d %49s %49s %29s %d",
                  &id, title, author, category, &copies) == 5) {
        if (id < 1 || copies < 0) continue;
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
