#ifndef MODELS_H
#define MODELS_H

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

namespace CampusConnect {

// Utility helper to escape JSON string values
inline std::string escapeJsonString(const std::string& input) {
    std::ostringstream ss;
    for (char c : input) {
        switch (c) {
            case '"':  ss << "\\\""; break;
            case '\\': ss << "\\\\"; break;
            case '\b': ss << "\\b";  break;
            case '\f': ss << "\\f";  break;
            case '\n': ss << "\\n";  break;
            case '\r': ss << "\\r";  break;
            case '\t': ss << "\\t";  break;
            default:
                if ('\x00' <= c && c <= '\x1f') {
                    ss << "\\u" << std::hex << std::setw(4) << std::setfill('0') << (int)c;
                } else {
                    ss << c;
                }
        }
    }
    return ss.str();
}

// Student Data Model
struct Student {
    int id = 0;
    std::string studentId;
    std::string fullName;
    std::string email;
    std::string phone;
    std::string department;
    int semester = 1;
    double cgpa = 0.0;
    std::string status = "Active";
    std::string createdAt;

    std::string toJson() const {
        std::ostringstream ss;
        ss << "{"
           << "\"id\":" << id << ","
           << "\"studentId\":\"" << escapeJsonString(studentId) << "\","
           << "\"fullName\":\"" << escapeJsonString(fullName) << "\","
           << "\"email\":\"" << escapeJsonString(email) << "\","
           << "\"phone\":\"" << escapeJsonString(phone) << "\","
           << "\"department\":\"" << escapeJsonString(department) << "\","
           << "\"semester\":" << semester << ","
           << "\"cgpa\":" << std::fixed << std::setprecision(2) << cgpa << ","
           << "\"status\":\"" << escapeJsonString(status) << "\","
           << "\"createdAt\":\"" << escapeJsonString(createdAt) << "\""
           << "}";
        return ss.str();
    }
};

// Helpdesk Ticket Data Model
struct Ticket {
    int id = 0;
    std::string ticketCode;
    std::string studentId;
    std::string studentName;
    std::string email;
    std::string department;
    std::string queryType;     // Academic, Complaint, Technical Support, Feedback
    std::string priority;      // Low, Medium, High, Urgent
    std::string subject;
    std::string message;
    std::string status;        // Open, In Progress, Resolved, Closed
    std::string staffResponse;
    std::string createdAt;
    std::string updatedAt;

    std::string toJson() const {
        std::ostringstream ss;
        ss << "{"
           << "\"id\":" << id << ","
           << "\"ticketCode\":\"" << escapeJsonString(ticketCode) << "\","
           << "\"studentId\":\"" << escapeJsonString(studentId) << "\","
           << "\"studentName\":\"" << escapeJsonString(studentName) << "\","
           << "\"email\":\"" << escapeJsonString(email) << "\","
           << "\"department\":\"" << escapeJsonString(department) << "\","
           << "\"queryType\":\"" << escapeJsonString(queryType) << "\","
           << "\"priority\":\"" << escapeJsonString(priority) << "\","
           << "\"subject\":\"" << escapeJsonString(subject) << "\","
           << "\"message\":\"" << escapeJsonString(message) << "\","
           << "\"status\":\"" << escapeJsonString(status) << "\","
           << "\"staffResponse\":\"" << escapeJsonString(staffResponse) << "\","
           << "\"createdAt\":\"" << escapeJsonString(createdAt) << "\","
           << "\"updatedAt\":\"" << escapeJsonString(updatedAt) << "\""
           << "}";
        return ss.str();
    }
};

// System Summary Statistics Model
struct SystemStats {
    int totalStudents = 0;
    int totalTickets = 0;
    int openTickets = 0;
    int inProgressTickets = 0;
    int resolvedTickets = 0;

    std::string toJson() const {
        std::ostringstream ss;
        ss << "{"
           << "\"totalStudents\":" << totalStudents << ","
           << "\"totalTickets\":" << totalTickets << ","
           << "\"openTickets\":" << openTickets << ","
           << "\"inProgressTickets\":" << inProgressTickets << ","
           << "\"resolvedTickets\":" << resolvedTickets
           << "}";
        return ss.str();
    }
};

} // namespace CampusConnect

#endif // MODELS_H
