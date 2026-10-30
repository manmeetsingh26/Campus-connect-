#include "cli.h"
#include "database.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <limits>

namespace CampusConnect {

void CliInterface::showBanner() {
    std::cout << "\n============================================================\n";
    std::cout << "        CAMPUS CONNECT - HELP DESK & STUDENT MANAGEMENT     \n";
    std::cout << "                 C++ Backend Engine v2.0                    \n";
    std::cout << "============================================================\n";
}

void CliInterface::run() {
    showBanner();
    auto& db = DatabaseManager::getInstance();

    int choice = -1;
    while (choice != 0) {
        std::cout << "\n------------------ [ MAIN MENU ] ------------------\n";
        std::cout << " 1. View All Enrolled Students\n";
        std::cout << " 2. Register New Student\n";
        std::cout << " 3. Search Student (by ID or Name)\n";
        std::cout << " 4. Remove Student Record\n";
        std::cout << " 5. View All Helpdesk Tickets\n";
        std::cout << " 6. Track Helpdesk Ticket by Code\n";
        std::cout << " 7. Update Ticket Status & Staff Response\n";
        std::cout << " 8. View System Analytics & Reports\n";
        std::cout << " 0. Return to Server / Exit CLI\n";
        std::cout << "---------------------------------------------------\n";
        std::cout << "Enter your choice [0-8]: ";

        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "[!] Invalid input. Please enter a valid number.\n";
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        switch (choice) {
            case 1: displayAllStudents(); break;
            case 2: addNewStudent(); break;
            case 3: searchStudent(); break;
            case 4: deleteStudent(); break;
            case 5: displayAllTickets(); break;
            case 6: trackTicket(); break;
            case 7: updateTicketStatus(); break;
            case 8: viewSystemStats(); break;
            case 0:
                std::cout << "[*] Exiting CLI Interface...\n";
                break;
            default:
                std::cout << "[!] Choice out of range.\n";
                break;
        }
    }
}

void CliInterface::displayAllStudents() {
    auto& db = DatabaseManager::getInstance();
    auto students = db.getAllStudents();

    std::cout << "\n>>> ENROLLED STUDENTS (" << students.size() << ") <<<\n";
    std::cout << std::left 
              << std::setw(6)  << "ID" 
              << std::setw(14) << "STUDENT ID" 
              << std::setw(22) << "FULL NAME" 
              << std::setw(28) << "DEPARTMENT" 
              << std::setw(6)  << "SEM" 
              << std::setw(8)  << "CGPA" 
              << std::setw(10) << "STATUS" << "\n";
    std::cout << std::string(94, '-') << "\n";

    for (const auto& s : students) {
        std::cout << std::left 
                  << std::setw(6)  << s.id
                  << std::setw(14) << s.studentId
                  << std::setw(22) << s.fullName
                  << std::setw(28) << s.department
                  << std::setw(6)  << s.semester
                  << std::setw(8)  << std::fixed << std::setprecision(2) << s.cgpa
                  << std::setw(10) << s.status << "\n";
    }
}

void CliInterface::addNewStudent() {
    auto& db = DatabaseManager::getInstance();
    Student s;

    std::cout << "\n>>> REGISTER NEW STUDENT <<<\n";
    std::cout << "Enter Student ID (e.g. STU2026099): ";
    std::getline(std::cin, s.studentId);
    std::cout << "Enter Full Name: ";
    std::getline(std::cin, s.fullName);
    std::cout << "Enter University Email: ";
    std::getline(std::cin, s.email);
    std::cout << "Enter Phone: ";
    std::getline(std::cin, s.phone);
    std::cout << "Enter Department: ";
    std::getline(std::cin, s.department);
    std::cout << "Enter Current Semester (1-8): ";
    std::cin >> s.semester;
    std::cout << "Enter Current CGPA (0.0 - 10.0): ";
    std::cin >> s.cgpa;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    s.status = "Active";

    if (db.addStudent(s)) {
        std::cout << "[SUCCESS] Student " << s.fullName << " registered with ID: " << s.studentId << "\n";
    } else {
        std::cout << "[ERROR] Could not register. A student with ID " << s.studentId << " may already exist.\n";
    }
}

void CliInterface::searchStudent() {
    auto& db = DatabaseManager::getInstance();
    std::cout << "\nEnter Search Keyword (Student ID or Name): ";
    std::string query;
    std::getline(std::cin, query);

    auto results = db.getAllStudents(query);
    if (results.empty()) {
        std::cout << "[!] No student found matching \"" << query << "\".\n";
        return;
    }

    std::cout << "\nFound " << results.size() << " match(es):\n";
    for (const auto& s : results) {
        std::cout << "-------------------------------------------\n";
        std::cout << "DB ID      : " << s.id << "\n";
        std::cout << "Student ID : " << s.studentId << "\n";
        std::cout << "Full Name  : " << s.fullName << "\n";
        std::cout << "Email      : " << s.email << "\n";
        std::cout << "Phone      : " << s.phone << "\n";
        std::cout << "Department : " << s.department << "\n";
        std::cout << "Semester   : " << s.semester << " | CGPA: " << s.cgpa << "\n";
        std::cout << "Status     : " << s.status << "\n";
    }
    std::cout << "-------------------------------------------\n";
}

void CliInterface::deleteStudent() {
    auto& db = DatabaseManager::getInstance();
    std::cout << "\nEnter Database Record ID of student to delete: ";
    int id;
    if (!(std::cin >> id)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "[!] Invalid ID.\n";
        return;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    Student s;
    if (db.getStudentById(id, s)) {
        std::cout << "Are you sure you want to delete student: " << s.fullName << " (" << s.studentId << ")? (y/n): ";
        char confirm;
        std::cin >> confirm;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (confirm == 'y' || confirm == 'Y') {
            if (db.deleteStudent(id)) {
                std::cout << "[SUCCESS] Student record deleted successfully.\n";
            } else {
                std::cout << "[ERROR] Failed to delete.\n";
            }
        } else {
            std::cout << "[*] Deletion cancelled.\n";
        }
    } else {
        std::cout << "[!] Student with DB ID " << id << " not found.\n";
    }
}

void CliInterface::displayAllTickets() {
    auto& db = DatabaseManager::getInstance();
    auto tickets = db.getAllTickets();

    std::cout << "\n>>> HELPDESK TICKETS (" << tickets.size() << ") <<<\n";
    std::cout << std::left 
              << std::setw(15) << "CODE" 
              << std::setw(13) << "STUDENT ID" 
              << std::setw(18) << "TYPE" 
              << std::setw(10) << "PRIORITY" 
              << std::setw(14) << "STATUS" 
              << "SUBJECT\n";
    std::cout << std::string(90, '-') << "\n";

    for (const auto& t : tickets) {
        std::cout << std::left 
                  << std::setw(15) << t.ticketCode
                  << std::setw(13) << t.studentId
                  << std::setw(18) << t.queryType
                  << std::setw(10) << t.priority
                  << std::setw(14) << t.status
                  << (t.subject.length() > 30 ? t.subject.substr(0, 27) + "..." : t.subject) << "\n";
    }
}

void CliInterface::trackTicket() {
    auto& db = DatabaseManager::getInstance();
    std::cout << "\nEnter Ticket Code (e.g. CC-2026-1001): ";
    std::string code;
    std::getline(std::cin, code);

    Ticket t;
    if (db.getTicketByCode(code, t)) {
        std::cout << "\n============================================\n";
        std::cout << " Ticket Details: " << t.ticketCode << "\n";
        std::cout << "============================================\n";
        std::cout << "Student ID   : " << t.studentId << " (" << t.studentName << ")\n";
        std::cout << "Email        : " << t.email << "\n";
        std::cout << "Department   : " << t.department << "\n";
        std::cout << "Query Type   : " << t.queryType << "\n";
        std::cout << "Priority     : " << t.priority << "\n";
        std::cout << "Status       : " << t.status << "\n";
        std::cout << "Created At   : " << t.createdAt << "\n";
        std::cout << "Subject      : " << t.subject << "\n";
        std::cout << "Message      : " << t.message << "\n";
        std::cout << "Staff Reply  : " << (t.staffResponse.empty() ? "(Awaiting Staff Response)" : t.staffResponse) << "\n";
        std::cout << "============================================\n";
    } else {
        std::cout << "[!] Ticket not found with code: " << code << "\n";
    }
}

void CliInterface::updateTicketStatus() {
    auto& db = DatabaseManager::getInstance();
    std::cout << "\nEnter Ticket Code to update: ";
    std::string code;
    std::getline(std::cin, code);

    Ticket t;
    if (!db.getTicketByCode(code, t)) {
        std::cout << "[!] Ticket not found.\n";
        return;
    }

    std::cout << "Current Status: " << t.status << "\n";
    std::cout << "Select New Status:\n";
    std::cout << " 1. Open\n";
    std::cout << " 2. In Progress\n";
    std::cout << " 3. Resolved\n";
    std::cout << " 4. Closed\n";
    std::cout << "Choice: ";

    int sChoice;
    std::cin >> sChoice;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::string newStatus = t.status;
    if (sChoice == 1) newStatus = "Open";
    else if (sChoice == 2) newStatus = "In Progress";
    else if (sChoice == 3) newStatus = "Resolved";
    else if (sChoice == 4) newStatus = "Closed";

    std::cout << "Enter Staff Resolution / Response Note: ";
    std::string responseNote;
    std::getline(std::cin, responseNote);

    if (db.updateTicketStatus(code, newStatus, responseNote)) {
        std::cout << "[SUCCESS] Ticket " << code << " updated to " << newStatus << "!\n";
    } else {
        std::cout << "[ERROR] Failed to update ticket.\n";
    }
}

void CliInterface::viewSystemStats() {
    auto& db = DatabaseManager::getInstance();
    auto stats = db.getStatistics();

    std::cout << "\n============================================\n";
    std::cout << "    CAMPUS CONNECT - SYSTEM ANALYTICS       \n";
    std::cout << "============================================\n";
    std::cout << " Total Students Enrolled : " << stats.totalStudents << "\n";
    std::cout << " Total Helpdesk Tickets  : " << stats.totalTickets << "\n";
    std::cout << "  - Open Queries         : " << stats.openTickets << "\n";
    std::cout << "  - In Progress Queries  : " << stats.inProgressTickets << "\n";
    std::cout << "  - Resolved Queries     : " << stats.resolvedTickets << "\n";
    if (stats.totalTickets > 0) {
        double rate = (static_cast<double>(stats.resolvedTickets) / stats.totalTickets) * 100.0;
        std::cout << " Resolution Rate         : " << std::fixed << std::setprecision(1) << rate << "%\n";
    }
    std::cout << "============================================\n";
}

} // namespace CampusConnect
