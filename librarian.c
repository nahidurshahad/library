#include <stdio.h>

void addBook(void);
void updateBook(void);
void deleteBook(void);
void viewBooks(void);
void searchBook(void);

void librarianDashboard(int userIndex) {
    int choice;

    do {
        printf("\n===== LIBRARIAN DASHBOARD =====\n");
        printf("1. Add book\n");
        printf("2. Update book\n");
        printf("3. Delete book\n");
        printf("4. View books\n");
        printf("5. Search books\n");
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
