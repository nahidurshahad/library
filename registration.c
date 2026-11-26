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
int getUserCount(void);
void setUserCount(int count);
void loadUsers(void);
void saveUsers(void);
int checkEmail(char email[]);
int findUser(char email[]);
int isStrongPassword(const char password[]);
int setSecurityQuestion(int *questionId, char answer[]);

void registration(void) {
    int role;
    struct User *users = getUsers();
    int count = getUserCount();
    char password[64];
    char confirm[64];
    int character;

    loadUsers();
    users = getUsers();
    count = getUserCount();
    if (count >= 100) {
        printf("User limit reached.\n");
        return;
    }

    printf("\n===== REGISTRATION =====\n");
    printf("Role 1-Student 2-Teacher 3-Librarian: ");
    scanf("%d", &role);
    if (role == 1) strcpy(users[count].role, "Student");
    if (role == 2) strcpy(users[count].role, "Teacher");
    if (role == 3) strcpy(users[count].role, "Librarian");
    if (role < 1 || role > 3) {
        printf("Invalid role.\n");
        return;
    }

    printf("Enter name: ");
    if (scanf(" %49[^\t\r\n]", users[count].name) != 1) {
        printf("Invalid name.\n");
        return;
    }
    while ((character = getchar()) != '\n' && character != EOF) {
    }
    printf("Enter DIU email: ");
    scanf("%59s", users[count].email);

    if (checkEmail(users[count].email) == 0) {
        printf("Invalid email.\n");
        return;
    }
    if (findUser(users[count].email) != -1) {
        printf("Email already registered.\n");
        return;
    }

    printf("Enter password: ");
    scanf("%63s", password);
    if (!isStrongPassword(password)) {
        printf("Weak password. Try a stronger password.\n");
        return;
    }
    printf("Confirm password: ");
    scanf("%63s", confirm);
    if (strcmp(password, confirm) != 0) {
        printf("Passwords do not match.\n");
        return;
    }

    strcpy(users[count].password, password);
    if (!setSecurityQuestion(&users[count].securityQuestionId,
                             users[count].securityAnswer)) {
        return;
    }
    users[count].id = count + 1;
    count++;
    setUserCount(count);
    saveUsers();
    printf("Registration successful.\n");
}
