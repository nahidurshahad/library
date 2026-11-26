#ifndef LIBRARY_H
#define LIBRARY_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <sodium.h>
#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif
struct User { int id; char role[20]; char name[50]; char email[60]; char passwordHash[crypto_pwhash_argon2id_STRBYTES]; int securityQuestionId; char securityAnswerHash[crypto_pwhash_argon2id_STRBYTES]; };
struct Book { int id; char title[50]; char author[50]; char category[30]; int copies; };
extern struct User users[100]; extern struct Book books[100]; extern int userCount, bookCount;
int readPassword(const char[], char[], size_t); int isStrongPassword(const char[]); int isPasswordHash(const char[]); int hashPassword(const char[], char[]); int verifyPassword(const char[], const char[]);
const char *getSecurityQuestion(int); void printSecurityQuestions(void); int hashSecurityAnswer(const char[], char[]); int setSecurityQuestion(int *, char[]); int securityAnswersMatch(const char[], const char[]);
void loadUsers(void); void saveUsers(void); void loadBooks(void); void saveBooks(void);
void createAdmin(void); int nextUserId(void); int findUser(char[]); int validEmail(char[]); void registration(void); void forgotPassword(void); void login(void);
void addBook(void); void updateBook(void); void deleteBook(void); void viewBooks(void); void searchBook(void);
void viewUsers(void); void adminDashboard(void); void librarianDashboard(void); void studentDashboard(void); void teacherDashboard(void);
#endif
