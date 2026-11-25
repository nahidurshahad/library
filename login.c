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
const char *getSecurityQuestion(int questionId);
int setSecurityQuestion(int *questionId, char answer[]);
void adminDashboard(int userIndex);
void librarianDashboard(int userIndex);
void studentDashboard(int userIndex);
void teacherDashboard(int userIndex);

void login(void) {
    struct User *users = getUsers();
    char roleName[20];
    int roleChoice;
    char email[60];
    char password[64];
    int index;

    loadUsers();
    users = getUsers();

    printf("\n===== LOGIN =====\n");
    printf("Select Role:\n");
    printf("Enter role: ");
    scanf("%d", &roleChoice);
    printf("1. Admin\n");
    printf("2. Student\n");
    printf("3. Librarian\n");
    printf("4. Teacher\n");
    

    if (roleChoice == 1) strcpy(roleName, "Admin");
    if (roleChoice == 2) strcpy(roleName, "Student");
    if (roleChoice == 3) strcpy(roleName, "Librarian");
    if (roleChoice == 4) strcpy(roleName, "Teacher");
    if (roleChoice < 1 || roleChoice > 4) {
        printf("Invalid role.\n");
        return;
    }

    printf("Enter email: ");
    scanf("%59s", email);
    printf("Enter password: ");
    scanf("%63s", password);

    index = findUser(email);
    if (index == -1) {
        printf("Account not found.\n");
        return;
    }
    if (strcmp(users[index].role, roleName) != 0) {
        printf("Role does not match the account.\n");
        return;
    }
    if (strcmp(users[index].password, password) != 0) {
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
    if (roleChoice == 1) adminDashboard(index);
    if (roleChoice == 2) studentDashboard(index);
    if (roleChoice == 3) librarianDashboard(index);
    if (roleChoice == 4) teacherDashboard(index);
}
