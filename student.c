#include <stdio.h>

void viewBooks(void);
void searchBook(void);

void studentDashboard(int userIndex) {
    int choice;

    do {
        printf("\n===== STUDENT DASHBOARD =====\n");
        printf("1. View books\n");
        printf("2. Search books\n");
        printf("0. Logout\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        if (choice == 1) viewBooks();
        if (choice == 2) searchBook();
        if (choice == 0) printf("Student logged out.\n");
    } while (choice != 0);
}
