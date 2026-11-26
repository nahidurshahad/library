#include "library.h"

void addBook(void) {
    loadBooks();
    if (bookCount >= 100) {
        printf("Book limit reached.\n");
        return;
    }

    { int i, maximum = 0; for (i = 0; i < bookCount; i++) if (books[i].id > maximum) maximum = books[i].id; books[bookCount].id = maximum + 1; }
    printf("Enter title (no spaces): ");
    scanf("%49s", books[bookCount].title);
    printf("Enter author (no spaces): ");
    scanf("%49s", books[bookCount].author);
    printf("Enter category (no spaces): ");
    scanf("%29s", books[bookCount].category);
    printf("Enter copies: ");
    scanf("%d", &books[bookCount].copies);
    if (books[bookCount].copies < 0) {
        printf("Invalid copies.\n");
        return;
    }

    bookCount++;
    saveBooks();
    printf("Book added successfully.\n");
}

void updateBook(void) {
    int id;
    int i;

    loadBooks();
    printf("Enter book ID: ");
    scanf("%d", &id);
    for (i = 0; i < bookCount; i++) {
        if (books[i].id == id) {
            printf("Enter new title: ");
            scanf("%49s", books[i].title);
            printf("Enter new author: ");
            scanf("%49s", books[i].author);
            printf("Enter new category: ");
            scanf("%29s", books[i].category);
            printf("Enter new copies: ");
            scanf("%d", &books[i].copies);
            if (books[i].copies < 0) {
                printf("Invalid copies.\n");
                return;
            }
            saveBooks();
            printf("Book updated successfully.\n");
            return;
        }
    }
    printf("Book not found.\n");
}

void deleteBook(void) {
    int id;
    int i;
    int j;

    loadBooks();
    printf("Enter book ID: ");
    scanf("%d", &id);
    for (i = 0; i < bookCount; i++) {
        if (books[i].id == id) {
            for (j = i; j < bookCount - 1; j++) {
                books[j] = books[j + 1];
            }
            bookCount--;
            saveBooks();
            printf("Book deleted successfully.\n");
            return;
        }
    }
    printf("Book not found.\n");
}

void viewBooks(void) {
    int i;

    loadBooks();
    printf("\n===== BOOK LIST =====\n");
    printf("ID\tTitle\tAuthor\tCategory\tCopies\n");
    for (i = 0; i < bookCount; i++) {
        printf("%d\t%s\t%s\t%s\t%d\n",
               books[i].id, books[i].title, books[i].author,
               books[i].category, books[i].copies);
    }
}

void searchBook(void) {
    char text[50];
    int choice;
    int i;
    int found = 0;

    loadBooks();
    printf("Search by 1-ID 2-Title 3-Author 4-Category: ");
    scanf("%d", &choice);
    printf("Enter search text: ");
    scanf("%49s", text);

    for (i = 0; i < bookCount; i++) {
        if ((choice == 1 && books[i].id == atoi(text)) ||
            (choice == 2 && strstr(books[i].title, text) != NULL) ||
            (choice == 3 && strstr(books[i].author, text) != NULL) ||
            (choice == 4 && strstr(books[i].category, text) != NULL)) {
            printf("ID: %d, Title: %s, Author: %s, Category: %s, Copies: %d\n",
                   books[i].id, books[i].title, books[i].author,
                   books[i].category, books[i].copies);
            found = 1;
        }
    }
    if (found == 0) {
        printf("Book not found.\n");
    }
}
