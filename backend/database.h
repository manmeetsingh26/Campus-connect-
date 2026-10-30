#ifndef DATABASE_H
#define DATABASE_H

#include "models.h"
#include <vector>
#include <string>
#include <mutex>
#include <memory>

namespace CampusConnect {

class DatabaseManager {
public:
    static DatabaseManager& getInstance();

    // Initialization & Persistence
    bool initialize(const std::string& dataDir = "database");
    bool saveStudents();
    bool saveTickets();

    // Student Management Operations
    std::vector<Student> getAllStudents(const std::string& search = "", const std::string& department = "");
    bool getStudentById(int id, Student& outStudent);
    bool getStudentByStudentId(const std::string& studentId, Student& outStudent);
    bool addStudent(Student& student);
    bool updateStudent(const Student& student);
    bool deleteStudent(int id);

    // Helpdesk Ticket Operations
    std::vector<Ticket> getAllTickets(const std::string& status = "", const std::string& queryType = "");
    bool getTicketByCode(const std::string& ticketCode, Ticket& outTicket);
    std::vector<Ticket> getTicketsByStudentId(const std::string& studentId);
    bool addTicket(Ticket& ticket);
    bool updateTicketStatus(const std::string& ticketCode, const std::string& newStatus, const std::string& staffResponse);

    // Metrics & Statistics
    SystemStats getStatistics();

    // Helpers
    std::string generateTicketCode();
    std::string getCurrentTimestamp();

private:
    DatabaseManager();
    ~DatabaseManager() = default;
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    void seedDefaultsIfEmpty();
    bool loadStudentsFromFile();
    bool loadTicketsFromFile();

    std::string m_dataDir;
    std::string m_studentsFilePath;
    std::string m_ticketsFilePath;

    std::vector<Student> m_students;
    std::vector<Ticket> m_tickets;

    int m_nextStudentId = 1;
    int m_nextTicketId = 1;

    mutable std::mutex m_mutex;
};

} // namespace CampusConnect

#endif // DATABASE_H
