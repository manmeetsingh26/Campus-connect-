# CampusConnect: Student Helpdesk & Student Management System

A full-stack, enterprise-grade university portal combining an **Interactive Student Helpdesk** with an **Academic Student Management System (SMS)**. 

---

## 🌟 Key Features

### 1. 🎓 Student Helpdesk & Inquiry Center
- **Query / Grievance Submission**: Submit tickets under categories like *Academic*, *Technical Support*, *Campus Complaints*, *Feedback*, *Hostel & Facilities*, and *Financial/Fee Receipts*.
- **Priority Escalation**: Assign priority levels (*Low*, *Medium*, *High*, *Urgent*).
- **Unique Ticket Code Generation**: Automatically assigns reference tracking numbers (e.g., `CC-2026-1001`).
- **Live Ticket Tracker**: Search by Ticket Reference Code or Student ID to inspect ticket status, assigned timeline, and official staff responses.
- **Self-Service FAQ Accordion**: Instant answers to common student queries.

### 2. 👥 Student Management System (SMS)
- **Student Academic Directory**: Browse enrolled university students with ID, Full Name, Email, Phone, Department, Semester, and CGPA.
- **Search & Department Filters**: Instant live search by name/ID and dropdown filtering by engineering and business departments.
- **Student Registration**: Modal form with validation to enroll new students.
- **Student Profile View**: Modal profile card displaying full student records and linking directly to their helpdesk ticket history.
- **Delete Records**: Safe removal with user confirmation prompts.

### 3. 🛡️ Helpdesk Staff & Administrator Console
- **Centralized Ticket Management**: View and filter all campus tickets by status (*Open*, *In Progress*, *Resolved*, *Closed*) and category.
- **KPI Metrics Dashboard**: Real-time counter of total inquiries, pending issues, active reviews, and resolution count.
- **Staff Resolution Desk**: Dedicated modal allowing university officers to update status and submit official resolution notes.
- **CSV Data Export**: One-click download of helpdesk reports for administrative reporting.

---

## 🏗️ Project Architecture & File Structure

```
campus connect/
│
├── index.html              # Main Landing Page, Service Portals & Quick Tracker
├── contact.html            # Helpdesk Inquiry Form, Status Tracker & FAQ
├── students.html           # Student Management Directory & Admission Modal
├── admin.html              # Administrator & Staff Ticket Management Console
├── style.css               # Modern Design System (Vars, Cards, Badges, Modals, Tables)
├── style2.css              # Secondary Form Styling & Backward Compatibility
├── script.js               # Frontend JavaScript Engine (Dual REST API + LocalStorage)
│
├── backend/                # Native C++ Backend Source Code
│   ├── main.cpp            # Server Entry Point & Interactive Console Dispatcher
│   ├── server.h            # HTTP REST Server Header (Multi-threaded Winsock2)
│   ├── server.cpp          # HTTP Request Parser, CORS Headers & Static File Server
│   ├── database.h          # Database Manager Header (Thread-Safe CRUD)
│   ├── database.cpp        # In-Memory Storage & File Persistence Engine
│   ├── cli.h               # Terminal Console Interface Header
│   ├── cli.cpp             # Interactive ASCII Terminal Menu for Students/Admins
│   └── models.h            # C++ Data Structures (Student, Ticket, SystemStats)
│
├── database/               # Database Files & Schemas
│   ├── schema.sql          # Standard SQL Schema for SQLite / MySQL / PostgreSQL
│   ├── students.json       # Persistent Student JSON Database
│   └── tickets.json        # Persistent Ticket JSON Database
│
├── server.js               # Zero-Dependency Node.js Companion Runner
├── run_server.bat          # 1-Click Server Launcher
├── build.bat               # MSVC (cl.exe) C++ Build Script
├── build_gcc.bat           # MinGW (g++) C++ Build Script
├── CMakeLists.txt          # CMake Configuration File
└── README.md               # Documentation
```

---

## 🗄️ Database Design

The database includes relational models for students, helpdesk tickets, departments, and audit logs. See [schema.sql](file:///c:/Users/Manme/Downloads/campus%20connect/database/schema.sql) for the complete SQL statements.

### Tables Overview:
1. `students`:
   - `id`: Auto-incrementing primary key.
   - `student_id`: Unique identifier (e.g., `STU2026001`).
   - `full_name`, `email`, `phone`, `department`, `semester`, `cgpa`, `status`, `created_at`.
2. `tickets`:
   - `id`: Auto-incrementing primary key.
   - `ticket_code`: Unique reference code (e.g., `CC-2026-1001`).
   - `student_id`: Foreign key referencing the enrolled student.
   - `student_name`, `email`, `department`, `query_type`, `priority`, `subject`, `message`.
   - `status`: `Open` | `In Progress` | `Resolved` | `Closed`.
   - `staff_response`: Official resolution text.
   - `created_at`, `updated_at`.
3. `departments`:
   - `code`, `name`, `head_of_dept`.
4. `faqs`:
   - `category`, `question`, `answer`.

---

## ⚡ How to Run the Application

You have **three flexible ways** to run and explore the project:

### Option 1: 1-Click Server Launcher (Recommended)
Double-click `run_server.bat` in the project folder.
- If `server.exe` (compiled C++) is detected, it launches the native C++ server.
- Otherwise, it seamlessly launches via the runtime runner `server.js` on `http://localhost:8080/`.

### Option 2: Compiling the Native C++ Backend

#### Using MinGW / GCC (`g++`):
Run `build_gcc.bat` or execute in terminal:
```bash
g++ -std=c++17 -O2 backend/main.cpp backend/database.cpp backend/server.cpp backend/cli.cpp -o server.exe -lws2_32 -lpthread
./server.exe
```

#### Using Microsoft Visual C++ (`cl.exe`):
Open **x64 Native Tools Command Prompt for VS** and run `build.bat` or:
```cmd
cl.exe /EHsc /std:c++17 /O2 backend\main.cpp backend\database.cpp backend\server.cpp backend\cli.cpp /Fe:server.exe ws2_32.lib
server.exe
```

#### Using CMake:
```bash
mkdir build && cd build
cmake ..
cmake --build .
./server
```

#### C++ Server Command-Line Options:
- `./server.exe`: Starts the HTTP server on port 8080.
  - While running, type `c` and press Enter to open the **Interactive Terminal Menu**.
  - Type `q` and press Enter to safely shut down.
- `./server.exe --cli`: Launches terminal console management mode directly.
- `./server.exe --port 9000`: Runs the HTTP server on custom port 9000.

### Option 3: Direct Browser Mode (Zero-Installation)
Double-click [index.html](file:///c:/Users/Manmeet/Downloads/campus%20connect/index.html) in your browser.
The application includes an automatic client-side storage engine that stores, edits, and tracks tickets and students inside browser `localStorage` when no backend server is running!

---

## 📡 REST API Reference

| Endpoint | Method | Description |
|---|---|---|
| `/api/health` | `GET` | Health check and server timestamp |
| `/api/stats` | `GET` | Summary metrics (students, tickets, open, resolved) |
| `/api/students` | `GET` | Retrieve student records (supports `?search=` and `?department=`) |
| `/api/students` | `POST` | Register a new student record |
| `/api/students` | `PUT` | Update an existing student's details |
| `/api/students?id={id}` | `DELETE` | Delete a student record by ID |
| `/api/tickets` | `GET` | List all tickets (supports `?status=` and `?queryType=`) |
| `/api/tickets` | `POST` | Submit a new helpdesk ticket |
| `/api/tickets` | `PUT` | Update ticket status and append staff resolution notes |
| `/api/tickets/track?code={code}` | `GET` | Track ticket by Ticket Code (or `?studentId={id}`) |

---

## 💻 Tech Stack
- **Frontend**: HTML5, CSS3, JavaScript ES6+
- **Backend**: C++ (C++17, Windows Sockets `winsock2`, multi-threaded) & Node.js runner
- **Database**: Relational SQL (`schema.sql`) + JSON storage
- **Compatible OS**: Windows 10/11, Linux, macOS
