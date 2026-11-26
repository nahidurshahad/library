#include "library.h"

void createAdmin(void) {
    char hash[crypto_pwhash_argon2id_STRBYTES];
    char initialPassword[17];
    static const char alphabet[] =
        "ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz23456789!@#$%";
    int i, found = 0, changed = 0;

    loadUsers();
    for (i = 0; i < userCount; i++) {
        if (strcmp(users[i].email, "admin@diu.edu.bd") == 0) {
            found = 1;
            /* Invalidate the previously published default credential. */
            if ((!isPasswordHash(users[i].passwordHash) &&
                 strcmp(users[i].passwordHash, "Admin@123") == 0) ||
                verifyPassword(users[i].passwordHash, "Admin@123")) {
                for (int j = 0; j < 16; j++)
                    initialPassword[j] = alphabet[randombytes_uniform((uint32_t)(sizeof(alphabet) - 1))];
                initialPassword[16] = '\0';
                if (!hashPassword(initialPassword, hash)) return;
                strcpy(users[i].passwordHash, hash);
                printf("\nThe exposed default admin password was replaced. New one-time password: %s\n", initialPassword);
                sodium_memzero(initialPassword, sizeof(initialPassword));
                changed = 1;
            }
        }
        if (!isPasswordHash(users[i].passwordHash) &&
            hashPassword(users[i].passwordHash, hash)) {
            strcpy(users[i].passwordHash, hash);
            changed = 1;
        }
        if (users[i].securityAnswerHash[0] != '\0' &&
            strcmp(users[i].securityAnswerHash, "-") != 0 &&
            strncmp(users[i].securityAnswerHash, crypto_pwhash_argon2id_STRPREFIX,
                    strlen(crypto_pwhash_argon2id_STRPREFIX)) != 0) {
            char answerHash[crypto_pwhash_argon2id_STRBYTES];
            if (hashSecurityAnswer(users[i].securityAnswerHash, answerHash)) {
                strcpy(users[i].securityAnswerHash, answerHash);
                changed = 1;
            }
        }
    }

    if (!found && userCount < 100) {
        for (i = 0; i < 16; i++)
            initialPassword[i] = alphabet[randombytes_uniform((uint32_t)(sizeof(alphabet) - 1))];
        initialPassword[16] = '\0';
        if (!hashPassword(initialPassword, hash)) return;
        users[userCount].id = nextUserId();
        strcpy(users[userCount].name, "Super Admin");
        strcpy(users[userCount].email, "admin@diu.edu.bd");
        strcpy(users[userCount].passwordHash, hash);
        strcpy(users[userCount].role, "Admin");
        users[userCount].securityQuestionId = 0;
        users[userCount].securityAnswerHash[0] = '\0';
        userCount++;
        changed = 1;
        printf("\nSuper Admin created. Save this one-time password now: %s\n", initialPassword);
        sodium_memzero(initialPassword, sizeof(initialPassword));
    }

    if (changed) saveUsers();
}

int nextUserId(void) {
    int i, maximum = 0;
    for (i = 0; i < userCount; i++)
        if (users[i].id > maximum) maximum = users[i].id;
    return maximum + 1;
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
                             users[userCount].securityAnswerHash)) {
        return;
    }
    users[userCount].id = nextUserId();
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
        !securityAnswersMatch(users[index].securityAnswerHash, confirm)) {
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
