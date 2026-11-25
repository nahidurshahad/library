#include <stdio.h>
#include <stdlib.h>
#include <string.h>
struct Book {
    int id;
    char title[50];
    char author[50];
    char category[30];
    int copies;
};

struct Book *getBooks(void);
int getBookCount(void);
void setBookCount(int count);
void loadBooks(void);
void saveBooks(void);

void addBook(void) {
    struct Book *books = getBooks();
    int count = getBookCount();

    loadBooks();
    books = getBooks();
    count = getBookCount();

    if (count >= 100) {
        printf("Book limit reached.\n");
        return;
    }

    books[count].id = count + 1;
    printf("Title (no spaces): ");
    scanf("%49s", books[count].title);
    printf("Author (no spaces): ");
    scanf("%49s", books[count].author);
    printf("Category (no spaces): ");
    scanf("%29s", books[count].category);
    printf("Number of copies: ");
    scanf("%d", &books[count].copies);

    if (books[count].copies < 0) {
        printf("Invalid number of copies.\n");
        return;
    }

    count++;
    setBookCount(count);
    saveBooks();
    printf("Book added successfully.\n");
}

void updateBook(void) {
    struct Book *books = getBooks();
    int count = getBookCount();
    int id;
    int i;

    loadBooks();
    books = getBooks();
    count = getBookCount();
    printf("Enter book ID: ");
    scanf("%d", &id);

    for (i = 0; i < count; i++) {
        if (books[i].id == id) {
            printf("New title: ");
            scanf("%49s", books[i].title);
            printf("New author: ");
            scanf("%49s", books[i].author);
            printf("New category: ");
            scanf("%29s", books[i].category);
            printf("New copies: ");
            scanf("%d", &books[i].copies);
            if (books[i].copies < 0) {
                printf("Invalid number of copies.\n");
                return;
            }
            saveBooks();
            printf("Book updated.\n");
            return;
        }
    }
    printf("Book not found.\n");
}

void deleteBook(void) {
    struct Book *books = getBooks();
    int count = getBookCount();
    int id;
    int i;
    int j;

    loadBooks();
    books = getBooks();
    count = getBookCount();
    printf("Enter book ID: ");
    scanf("%d", &id);

    for (i = 0; i < count; i++) {
        if (books[i].id == id) {
            for (j = i; j < count - 1; j++) {
                books[j] = books[j + 1];
            }
            count--;
            setBookCount(count);
            saveBooks();
            printf("Book deleted.\n");
            return;
        }
    }
    printf("Book not found.\n");
}

void viewBooks(void) {
    struct Book *books = getBooks();
    int count = getBookCount();
    int i;

    loadBooks();
    books = getBooks();
    count = getBookCount();
    printf("\n===== BOOK LIST =====\n");
    printf("ID\tTitle\tAuthor\tCategory\tCopies\n");
    for (i = 0; i < count; i++) {
        printf("%d\t%s\t%s\t%s\t%d\n",
               books[i].id, books[i].title, books[i].author,
               books[i].category, books[i].copies);
    }
}

void searchBook(void) {
    struct Book *books = getBooks();
    int count = getBookCount();
    char text[50];
    int searchType;
    int i;
    int found = 0;

    loadBooks();
    books = getBooks();
    count = getBookCount();
    printf("Search by 1-ID 2-Title 3-Author 4-Category: ");
    scanf("%d", &searchType);
    printf("Enter search text: ");
    scanf("%49s", text);

    for (i = 0; i < count; i++) {
        if ((searchType == 1 && books[i].id == atoi(text)) ||
            (searchType == 2 && strstr(books[i].title, text) != NULL) ||
            (searchType == 3 && strstr(books[i].author, text) != NULL) ||
            (searchType == 4 && strstr(books[i].category, text) != NULL)) {
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
