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
void addBook(void);
void updateBook(void);
void deleteBook(void);
void viewBooks(void);
void searchBook(void);
int isStrongPassword(const char password[]);
int setSecurityQuestion(int *questionId, char answer[]);

void viewUsers(void) {
    struct User *users = getUsers();
    int count = getUserCount();
    int i;

    loadUsers();
    users = getUsers();
    count = getUserCount();
    printf("\nID\tName\tEmail\tRole\n");
    for (i = 0; i < count; i++) {
        printf("%d\t%s\t%s\t%s\n",
               users[i].id, users[i].name,
               users[i].email, users[i].role);
    }
}

void createMember(void) {
    struct User *users = getUsers();
    int count = getUserCount();
    char password[64];
    char confirm[64];
    int roleChoice;
    int i;

    loadUsers();
    users = getUsers();
    count = getUserCount();
    if (count >= 100) {
        printf("User limit reached.\n");
        return;
    }

    printf("Name (no spaces): ");
    scanf("%49s", users[count].name);
    printf("DIU email: ");
    scanf("%59s", users[count].email);
    if (strstr(users[count].email, "@diu.edu.bd") == NULL) {
        printf("Invalid DIU email.\n");
        return;
    }

    for (i = 0; i < count; i++) {
        if (strcmp(users[i].email, users[count].email) == 0) {
            printf("Email already exists.\n");
            return;
        }
    }

    printf("Role 1-Librarian 2-Student 3-Teacher: ");
    scanf("%d", &roleChoice);
    if (roleChoice == 1) strcpy(users[count].role, "Librarian");
    if (roleChoice == 2) strcpy(users[count].role, "Student");
    if (roleChoice == 3) strcpy(users[count].role, "Teacher");
    if (roleChoice < 1 || roleChoice > 3) {
        printf("Invalid role.\n");
        return;
    }

    printf("Password: ");
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
    printf("Account created.\n");
}

void updateMember(void) {
    struct User *users = getUsers();
    int count = getUserCount();
    int id;
    int i;
    char password[64];
    char confirm[64];

    loadUsers();
    users = getUsers();
    count = getUserCount();
    printf("Account ID: ");
    scanf("%d", &id);
    for (i = 0; i < count; i++) {
        if (users[i].id == id) {
            if (strcmp(users[i].role, "Admin") == 0) {
                printf("The Admin account cannot be changed.\n");
                return;
            }
            printf("New password: ");
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
            if (!setSecurityQuestion(&users[i].securityQuestionId,
                                     users[i].securityAnswer)) {
                return;
            }
            printf("New name: ");
            scanf("%49s", users[i].name);
            strcpy(users[i].password, password);
            saveUsers();
            printf("Account updated.\n");
            return;
        }
    }
    printf("Account not found.\n");
}

void deleteMember(void) {
    struct User *users = getUsers();
    int count = getUserCount();
    int id;
    int i;
    int j;

    loadUsers();
    users = getUsers();
    count = getUserCount();
    printf("Account ID: ");
    scanf("%d", &id);
    for (i = 0; i < count; i++) {
        if (users[i].id == id) {
            if (strcmp(users[i].role, "Admin") == 0) {
                printf("The Admin account cannot be deleted.\n");
                return;
            }
            for (j = i; j < count - 1; j++) {
                users[j] = users[j + 1];
            }
            count--;
            setUserCount(count);
            saveUsers();
            printf("Account deleted.\n");
            return;
        }
    }
    printf("Account not found.\n");
}

void adminDashboard(int userIndex) {
    int choice;

    do {
        printf("\n===== ADMIN DASHBOARD =====\n");
        printf("1. View users\n");
        printf("2. Create account\n");
        printf("3. Update account\n");
        printf("4. Delete account\n");
        printf("5. Add book\n");
        printf("6. Update book\n");
        printf("7. Delete book\n");
        printf("8. View books\n");
        printf("9. Search books\n");
        printf("0. Logout\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        if (choice == 1) viewUsers();
        if (choice == 2) createMember();
        if (choice == 3) updateMember();
        if (choice == 4) deleteMember();
        if (choice == 5) addBook();
        if (choice == 6) updateBook();
        if (choice == 7) deleteBook();
        if (choice == 8) viewBooks();
        if (choice == 9) searchBook();
        if (choice == 0) printf("Admin logged out.\n");
    } while (choice != 0);
}
