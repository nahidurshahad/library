#include "library.h"

void login(void) {
    char email[60];
    char password[64];
    char roleName[20];
    int role;
    int index;

    loadUsers();
    printf("\n===== LOGIN =====\n");
    printf("Select role:\n");
    printf("1. Super Admin\n");
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
                                 users[index].securityAnswerHash)) {
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
