# Structure-oriented Persistent Data Management

A menu-driven **Student, Course Allocation and Result Management System** written in **C**.
It stores data in files so records remain available after the program is closed.

## Problem Statement

A small education institute registers students with the fields **ID, NAME, Fees**.
After fees are paid, a student is allotted one course: **DS, AI, ML or Web**.
After the course is delivered, each student gets a result: **Excellent, Good or Fail**.
The system must use suitable data structures, persistent storage, user authentication and a menu-driven interface.

## Features

- Login system (username and password, 3 attempts)
- Passwords stored as hashes (djb2), not as plain text
- Register, view, search and delete students
- Pay or update fees
- Course allotment only after the minimum fees (Rs. 5000) are paid
- Result declaration only after a course is allotted
- Course-wise report (students per course and result counts)
- Add new login users
- Input validation and duplicate ID check
- Persistent storage using binary files

## Data Structures

| Purpose | Structure |
|---|---|
| Student record | `struct StudentRec` (id, name, fees, course, result) |
| In-memory storage | Singly linked list (`struct Node`) |
| Course and result codes | `enum CourseId`, `enum ResultId` with name lookup tables |
| Login user | `struct User` (username, password hash) |

## Persistent Storage

| File | Content |
|---|---|
| `students.dat` | Binary student records, rewritten after every change |
| `users.dat` | Binary user records, created on first run |

Records are loaded into the linked list at startup and saved after each operation and on exit.

## How to Compile and Run

```bash
gcc student_mgmt.c -o student_mgmt
./student_mgmt
```

On Windows (MinGW): `gcc student_mgmt.c -o student_mgmt.exe` then `student_mgmt.exe`

**Default login (created on first run):** `admin` / `admin123`

## Menu

```
1. Register student
2. View all students
3. Search student by ID
4. Pay / update fees
5. Allot course
6. Declare result
7. Delete student
8. Course-wise report
9. Add new user (login)
0. Exit
```

## Business Rules

1. A student ID must be positive and unique.
2. A course can be allotted only if fees paid are at least 5000.
3. A student can be allotted only one course.
4. A result can be declared only after a course is allotted.

## Test Cases (Actual Output)

| TC | Scenario | Output |
|---|---|---|
| 1 | Wrong password, then correct | `Invalid credentials (1/3 attempts used).` then `Login successful. Welcome, admin!` |
| 2 | Register 101 Amit Sen (3000) and 102 Riya Das (6000) | `Student registered successfully.` (twice) |
| 3 | Register duplicate ID 101 | `Error: ID 101 already exists.` |
| 4 | Allot course with fees 3000 | `Cannot allot course: fees paid 3000.00 is below minimum 5000.00.` |
| 5 | Pay 3000 more for ID 101 | `Fees updated. Total paid: 6000.00` |
| 6 | Allot AI to 101, DS to 102 | `Course AI allotted to Amit Sen.` / `Course DS allotted to Riya Das.` |
| 7 | Declare results | `Result 'Excellent' declared for Riya Das.` / `Result 'Fail' declared for Amit Sen.` |
| 8 | Search ID 101 | `101    Amit Sen             6000.00    AI             Fail` |
| 9 | Persistence (exit, run again, view all) | Both records still present with course and result |
| 10 | Delete ID 102 | `Student 102 deleted.` |
| 11 | Add user `teacher`, log in with it | `User 'teacher' created.` / `Login successful. Welcome, teacher!` |
| 12 | Three wrong logins | `Too many failed attempts. Exiting.` |

### Sample: View All Students

```
ID     NAME                 FEES       COURSE         RESULT
------------------------------------------------------------------
101    Amit Sen             6000.00    AI             Fail
102    Riya Das             6000.00    DS             Excellent
```

### Sample: Course-wise Report

```
COURSE         STUDENTS  EXCELLENT  GOOD   FAIL   PENDING
------------------------------------------------------------
Not Allotted   0         0          0      0      0
DS             1         1          0      0      0
AI             1         0          0      1      0
ML             0         0          0      0      0
Web            0         0          0      0      0
```

## Possible Improvements

- Per-course fees instead of a single minimum
- Encrypted or salted password hashing
- Sorting and filtering of the student list
- Export of reports to CSV

## Author

Name: _your name_
Roll No: _your roll number_
Class / Subject: _your class and subject_
