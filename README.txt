DIU Library Management System - Beginner Version

Source
- main.c

Compile
gcc main.c -std=c11 -o library

Run
Windows: library.exe
Linux/macOS: ./library

Default admin
Email: admin@diu.edu.bd
Password: Admin@123

Password rules
- At least 8 characters
- At least one uppercase letter and one lowercase letter
- At least one number
- At least one special character (for example: !, @, or #)
- Password input is masked by default. Choose y at the prompt to show it while typing.

Password recovery
- Choose one of the six security questions when creating or changing a password.
- Existing accounts are asked to choose one after their next successful login.
- The answer is case-insensitive and must be entered as one word (no spaces).
- You must answer the selected question correctly before resetting a password.

Data files
- users.csv
- books.txt
- Account data is stored as CSV in users.csv.

Security note
- This beginner project stores passwords and security answers in plaintext in users.csv.
- Security questions are not suitable as the only recovery method for production systems.
