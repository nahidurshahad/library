#include "library.h"

void viewUsers(void) {
    int i;
    loadUsers();
    printf("\nID\tName\tEmail\tRole\n");
    for (i = 0; i < userCount; i++) {
        printf("%d\t%s\t%s\t%s\n",
               users[i].id, users[i].name,
               users[i].email, users[i].role);
    }
}
