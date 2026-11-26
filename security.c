#include "library.h"

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

int hashSecurityAnswer(const char answer[], char hash[]) {
    char normalized[64];
    size_t i, length = strlen(answer);
    if (length == 0 || length >= sizeof(normalized)) return 0;
    for (i = 0; i < length; i++)
        normalized[i] = (char)tolower((unsigned char)answer[i]);
    normalized[length] = '\0';
    return crypto_pwhash_argon2id_str(hash, normalized, length,
        crypto_pwhash_argon2id_OPSLIMIT_INTERACTIVE,
        crypto_pwhash_argon2id_MEMLIMIT_INTERACTIVE) == 0;
}

int setSecurityQuestion(int *questionId, char answerHash[]) {
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

    if (!hashSecurityAnswer(input, answerHash)) {
        printf("Could not protect security answer.\n");
        return 0;
    }
    *questionId = choice;
    return 1;
}

int securityAnswersMatch(const char storedHash[], const char provided[]) {
    char normalized[64];
    size_t i, length = strlen(provided);
    if (length == 0 || length >= sizeof(normalized) ||
        strncmp(storedHash, crypto_pwhash_argon2id_STRPREFIX,
                strlen(crypto_pwhash_argon2id_STRPREFIX)) != 0) return 0;
    for (i = 0; i < length; i++)
        normalized[i] = (char)tolower((unsigned char)provided[i]);
    normalized[length] = '\0';
    return crypto_pwhash_argon2id_str_verify(storedHash, normalized, length) == 0;
}
