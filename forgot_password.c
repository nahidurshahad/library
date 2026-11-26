#include <stdio.h>
#include <string.h>

struct User {
    int id;
    char role[20];
    char name[50];
    char email[60];
    char password[64];
    int securityQuestionId;
    char securityAnswer[64];
};

struct User *getUsers(void);
int findUser(char email[]);
void loadUsers(void);
void saveUsers(void);
int isStrongPassword(const char password[]);
const char *getSecurityQuestion(int questionId);
int securityAnswersMatch(const char stored[], const char provided[]);

void forgotPassword(void) {
    struct User *users = getUsers();
    char email[60];
    char password[64];
    char confirm[64];
    char answer[64];
    int index;

    loadUsers();
    users = getUsers();
    printf("Enter registered email: ");
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
    if (scanf("%63s", answer) != 1 ||
        !securityAnswersMatch(users[index].securityAnswer, answer)) {
        printf("Security answer does not match.\n");
        return;
    }

    printf("Enter new password: ");
    scanf("%63s", password);
    if (!isStrongPassword(password)) {
        printf("Weak password. Try a stronger password.\n");
        return;
    }
    printf("Confirm new password: ");
    scanf("%63s", confirm);
    if (strcmp(password, confirm) != 0) {
        printf("Passwords do not match.\n");
        return;
    }
    strcpy(users[index].password, password);
    saveUsers();
    printf("Password changed successfully.\n");
}
