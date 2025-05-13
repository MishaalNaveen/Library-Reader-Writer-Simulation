# 🏛️ Thread-Safe Library Booking System (Reader-Writer Problem Simulation)

This project simulates the **Reader-Writer problem** using POSIX threads, semaphores, and mutexes in C. It models a **library management system** where multiple readers can concurrently view book data, while only one librarian (writer) can update records at a time. Designed for Operating Systems coursework, this system demonstrates thread synchronization and real-time access control in a shared environment.

---

## 📁 Project Structure

| File       | Description                                  |
|------------|----------------------------------------------|
| `main.c`   | Main C program implementing the simulation   |
| `books.txt`| Sample book database (editable)              |

---

## 🚀 Features

- 🧵 **Multithreaded Simulation** using POSIX `pthreads`
- 🔐 **Synchronization** using mutexes and semaphores
- 📚 **Reader Threads**:
  - Select and view book records concurrently
- ✍️ **Writer Thread (Librarian)**:
  - Add new books (one at a time, no readers allowed during write)
- 🔍 **Case-insensitive search** by book title
- 🕓 **Timestamps** for each reader/writer action
- 📜 Interactive CLI Menu for user navigation

---

## 🧪 How to Run

### 💻 Compile

```bash
gcc main.c -o library -lpthread
```

### ▶️ Run

```bash
./library
```

---

## 📘 Sample Menu

```
=====================================
     Welcome to the Library System
=====================================

Menu:
1. Librarian (Add Book)
2. Reader (View Book)
3. Search Book by Title
4. Exit
-------------------------------------
Enter choice:
```

---

## 🔧 Synchronization Strategy

This project implements a **first readers-writers problem** solution:

- Multiple readers can access the data **simultaneously**
- A writer **must wait** until **all readers finish**
- If a writer is active or waiting, **no new readers can start**

Used synchronization primitives:

- `pthread_mutex_t` for critical sections
- `sem_t` for reader-writer mutual exclusion
- Extra control to prevent writer starvation using `readerTry` semaphore

---

## 📓 Use Cases

- 📖 Simulate **concurrent read/write access**
- 🎓 Learn **OS synchronization concepts** in practice
- 💡 Ideal for demos, labs, and viva practice

---

## ✅ Requirements

- GCC compiler
- POSIX-compliant OS (Linux, macOS, or WSL on Windows)

---

## 📌 Notes

- Make sure `books.txt` exists in the same directory as `main.c`
- You can manually edit `books.txt` to add test data
- Use `Ctrl+C` to force-exit if needed

---

## 👨‍💻 Author

This project was created as part of the **Operating Systems course** to demonstrate synchronization techniques using real-world analogies.

---

## 🏷️ Tags

`#ReaderWriter` `#OperatingSystems` `#Threading` `#LibrarySystem` `#CProgramming` `#POSIXThreads`

---
