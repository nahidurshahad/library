# DIU Library Management System

A beginner-friendly C library management project. The maintained implementation is **main.c only**. Do not compile the removed legacy modules together with it.

## Requirements

- GCC or another C11-compatible compiler
- libsodium (Argon2id password hashing)

Install libsodium:

- Ubuntu/Debian: `sudo apt install libsodium-dev`
- macOS: `brew install libsodium`
- Windows (MSYS2): `pacman -S mingw-w64-x86_64-libsodium`

## Build

Linux/macOS:

```sh
gcc main.c -std=c11 -Wall -Wextra -o library -lsodium
```

Windows (MSYS2 MinGW):

```sh
gcc main.c -std=c11 -Wall -Wextra -o library.exe -lsodium
```

## First run

Run the program. On the first launch, it creates the Super Admin account:

- Email: `admin@diu.edu.bd`
- A random one-time password is generated and printed in the terminal.

**Save that password immediately.** It is not stored in the source code or README. If it is lost, remove the local `users.csv` only if you are willing to reset all accounts, then run the program again.

Login roles: Super Admin, Librarian, Student, Teacher.

Registration roles: Librarian, Student, Teacher. Super Admin cannot self-register.

## Password and account storage

- Passwords are stored as Argon2id hashes with unique random salts using libsodium.
- Password recovery answers are also Argon2id-hashed.
- Existing local CSV records containing plaintext passwords or recovery answers are upgraded to hashes when the program starts.
- User data is stored locally in `users.csv`; book data is stored in `books.txt`.
- These data files are excluded from Git. Back them up privately.

Security questions are suitable only for a student project, not as the sole recovery method for a production system. A production application should use verified email recovery, rate limits, and stronger account controls.

## Password rules

- At least 8 characters
- At least one uppercase and one lowercase letter
- At least one number
- At least one special character

Password input is masked by default. Choose `y` at the prompt to show it while typing.
