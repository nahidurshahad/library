DIU LIBRARY MANAGEMENT SYSTEM
=============================

PROJECT STRUCTURE
-----------------
main.c       - Program entry point and main menu
library.h    - Shared structures, global declarations and function prototypes
security.c   - Password input, password strength, Argon2id hashing and security questions
data.c       - User CSV and book file loading/saving
auth.c       - Registration, password recovery, Super Admin creation and user lookup
login.c      - Login and role verification
admin.c      - Admin user-list operation
book.c       - Add, update, delete, view and search books
dashboard.c  - Admin, librarian, student and teacher dashboards

BUILD
-----
All source files must be compiled together.

Linux:
  sudo apt install gcc libsodium-dev
  gcc main.c security.c data.c auth.c admin.c book.c dashboard.c login.c -std=c11 -Wall -Wextra -o library -lsodium
  ./library

Windows (MSYS2 UCRT64):
  pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-libsodium
  gcc main.c security.c data.c auth.c admin.c book.c dashboard.c login.c -std=c11 -Wall -Wextra -o library.exe -lsodium
  ./library.exe

DATA FILES
----------
users.csv and books.txt are created/used locally and are excluded from Git.
Passwords and security answers are stored using Argon2id hashes.

ROLES
-----
Super Admin: created automatically on first run with a random one-time password.
Librarian: manages books.
Student: views and searches books.
Teacher: views and searches books.

The initial Super Admin password is printed once in the terminal. Save it securely.
