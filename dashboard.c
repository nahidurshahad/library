#include "library.h"

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
