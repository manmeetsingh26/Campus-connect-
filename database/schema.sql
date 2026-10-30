-- ============================================================
-- CampusConnect Database Schema
-- Student Helpdesk & Student Management System
-- Compatible with SQLite, MySQL, and PostgreSQL
-- ============================================================

-- 1. Departments Table
CREATE TABLE IF NOT EXISTS departments (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    code VARCHAR(10) UNIQUE NOT NULL,
    name VARCHAR(100) NOT NULL,
    head_of_dept VARCHAR(100)
);

-- 2. Students Table
CREATE TABLE IF NOT EXISTS students (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    student_id VARCHAR(20) UNIQUE NOT NULL,
    full_name VARCHAR(100) NOT NULL,
    email VARCHAR(120) UNIQUE NOT NULL,
    phone VARCHAR(20),
    department VARCHAR(50) NOT NULL,
    semester INTEGER DEFAULT 1,
    cgpa REAL DEFAULT 0.0,
    status VARCHAR(20) DEFAULT 'Active',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 3. Helpdesk Tickets Table
CREATE TABLE IF NOT EXISTS tickets (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    ticket_code VARCHAR(30) UNIQUE NOT NULL,
    student_id VARCHAR(20) NOT NULL,
    student_name VARCHAR(100) NOT NULL,
    email VARCHAR(120) NOT NULL,
    department VARCHAR(50),
    query_type VARCHAR(40) NOT NULL,
    priority VARCHAR(20) DEFAULT 'Medium',
    subject VARCHAR(200) NOT NULL,
    message TEXT NOT NULL,
    status VARCHAR(20) DEFAULT 'Open',
    staff_response TEXT DEFAULT '',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (student_id) REFERENCES students(student_id) ON DELETE CASCADE
);

-- 4. Ticket Activity Logs
CREATE TABLE IF NOT EXISTS ticket_logs (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    ticket_code VARCHAR(30) NOT NULL,
    action VARCHAR(50) NOT NULL,
    notes TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 5. FAQs / Knowledgebase Table
CREATE TABLE IF NOT EXISTS faqs (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    category VARCHAR(50) NOT NULL,
    question TEXT NOT NULL,
    answer TEXT NOT NULL
);

-- ============================================================
-- SEED DATA
-- ============================================================

-- Seed Departments
INSERT INTO departments (code, name, head_of_dept) VALUES
('CSE', 'Computer Science & Engineering', 'Dr. Sarah Mitchell'),
('ECE', 'Electronics & Communication', 'Prof. David Kumar'),
('MECH', 'Mechanical Engineering', 'Dr. Robert Jenkins'),
('CIVIL', 'Civil & Environmental Engineering', 'Dr. Elena Rostova'),
('BBA', 'Business Administration', 'Prof. Anita Sharma');

-- Seed Students
INSERT INTO students (student_id, full_name, email, phone, department, semester, cgpa, status) VALUES
('STU2026001', 'Aarav Sharma', 'aarav.sharma@campus.edu', '+91 9876543210', 'Computer Science & Engineering', 6, 8.85, 'Active'),
('STU2026002', 'Priya Patel', 'priya.patel@campus.edu', '+91 9876543211', 'Electronics & Communication', 4, 9.12, 'Active'),
('STU2026003', 'Rohan Mehta', 'rohan.mehta@campus.edu', '+91 9876543212', 'Mechanical Engineering', 8, 7.94, 'Active'),
('STU2026004', 'Ananya Iyer', 'ananya.iyer@campus.edu', '+91 9876543213', 'Business Administration', 2, 8.40, 'Active'),
('STU2026005', 'Vikram Singh', 'vikram.singh@campus.edu', '+91 9876543214', 'Computer Science & Engineering', 5, 8.20, 'Active'),
('STU2026006', 'Neha Gupta', 'neha.gupta@campus.edu', '+91 9876543215', 'Civil & Environmental Engineering', 3, 8.65, 'Active');

-- Seed Helpdesk Tickets
INSERT INTO tickets (ticket_code, student_id, student_name, email, department, query_type, priority, subject, message, status, staff_response, created_at, updated_at) VALUES
('CC-2026-1001', 'STU2026001', 'Aarav Sharma', 'aarav.sharma@campus.edu', 'Computer Science & Engineering', 'Academic', 'High', 'Elective Subject Course Registration Issue', 'I am unable to select Cloud Computing CS-402 in the student portal dropdown.', 'In Progress', 'Academic cell has received your request. We are reviewing system capacity.', '2026-10-25 10:14:00', '2026-10-26 14:30:00'),
('CC-2026-1002', 'STU2026002', 'Priya Patel', 'priya.patel@campus.edu', 'Electronics & Communication', 'Technical Support', 'Medium', 'Campus Wi-Fi connectivity in Library 2nd Floor', 'Wi-Fi signal drops frequently in the north corner study cubicles.', 'Resolved', 'Network access point AP-04 rebooted and firmware updated. Signal restored.', '2026-10-26 09:20:00', '2026-10-27 11:15:00'),
('CC-2026-1003', 'STU2026003', 'Rohan Mehta', 'rohan.mehta@campus.edu', 'Mechanical Engineering', 'Complaint', 'High', 'Lab Machine Calibration Delay', 'CNC machine in Workshop 3 requires recalibration for our final year project work.', 'Open', '', '2026-10-28 15:45:00', '2026-10-28 15:45:00'),
('CC-2026-1004', 'STU2026004', 'Ananya Iyer', 'ananya.iyer@campus.edu', 'Business Administration', 'Feedback', 'Low', 'Request for Extended Weekend Library Hours', 'Students preparing for mid-term exams request the central library remain open until 10 PM.', 'Open', '', '2026-10-29 11:05:00', '2026-10-29 11:05:00');

-- Seed FAQs
INSERT INTO faqs (category, question, answer) VALUES
('Academic', 'How do I apply for official transcripts or degree certificates?', 'You can request transcripts through the academic portal under Services > Transcripts or submit an Academic query ticket here.'),
('Technical Support', 'What is the default campus Wi-Fi credential format?', 'Use your Student ID as the username and your portal password to connect to CampusConnect-Student network.'),
('Hostel & Facilities', 'How do I lodge a hostel maintenance complaint?', 'Select "Complaint" under query type and include your Hostel block and room number in the message.'),
('Fees & Accounts', 'Where can I download semester fee payment receipts?', 'Receipts are generated under Student Dashboard > Accounts & Billing within 24 hours of successful payment.');
