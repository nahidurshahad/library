#include "library.h"

int main(void) {
    int choice;

    if (sodium_init() < 0) {
        printf("Could not start libsodium.\n");
        return 1;
    }
    createAdmin();

    do {
        printf("\n=================================\n");
        printf(" DIU LIBRARY MANAGEMENT SYSTEM\n");
        printf("=================================\n");
        printf("1. Login\n");
        printf("2. Registration\n");
        printf("3. Forgot Password\n");
        printf("0. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        if (choice == 1) login();
        if (choice == 2) registration();
        if (choice == 3) forgotPassword();
        if (choice == 0) printf("Thank you.\n");
        if (choice < 0 || choice > 3) printf("Invalid choice.\n");
    } while (choice != 0);

    return 0;
}
