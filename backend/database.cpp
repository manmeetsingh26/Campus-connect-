#include "database.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <chrono>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <random>

namespace CampusConnect {

static std::string trimString(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

static std::string extractJsonField(const std::string& obj, const std::string& key) {
    std::string needle = "\"" + key + "\"";
    size_t pos = obj.find(needle);
    if (pos == std::string::npos) return "";

    pos = obj.find(':', pos + needle.length());
    if (pos == std::string::npos) return "";

    pos++; // move past ':'
    while (pos < obj.length() && (obj[pos] == ' ' || obj[pos] == '\t' || obj[pos] == '\r' || obj[pos] == '\n')) {
        pos++;
    }

    if (pos >= obj.length()) return "";

    if (obj[pos] == '"') {
        // String value
        pos++;
        std::string val;
        while (pos < obj.length()) {
            if (obj[pos] == '\\' && pos + 1 < obj.length()) {
                val += obj[pos + 1];
                pos += 2;
            } else if (obj[pos] == '"') {
                break;
            } else {
                val += obj[pos];
                pos++;
            }
        }
        return val;
    } else {
        // Number or boolean
        size_t endPos = obj.find_first_of(",}\n\r", pos);
        if (endPos == std::string::npos) endPos = obj.length();
        return trimString(obj.substr(pos, endPos - pos));
    }
}

DatabaseManager& DatabaseManager::getInstance() {
    static DatabaseManager instance;
    return instance;
}

DatabaseManager::DatabaseManager() : m_dataDir("database") {
    m_studentsFilePath = m_dataDir + "/students.json";
    m_ticketsFilePath = m_dataDir + "/tickets.json";
}

std::string DatabaseManager::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm tm;
#if defined(_WIN32) || defined(_WIN64)
    localtime_s(&tm, &now_c);
#else
    localtime_r(&now_c, &tm);
#endif
    char buf[64];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
    return std::string(buf);
}

std::string DatabaseManager::generateTicketCode() {
    static std::mt19937 rng(1337);
    std::uniform_int_distribution<int> dist(1000, 9999);
    int num = dist(rng);
    return "CC-2026-" + std::to_string(num);
}

bool DatabaseManager::initialize(const std::string& dataDir) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_dataDir = dataDir;
    m_studentsFilePath = m_dataDir + "/students.json";
    m_ticketsFilePath = m_dataDir + "/tickets.json";

    bool studentsLoaded = loadStudentsFromFile();
    bool ticketsLoaded = loadTicketsFromFile();

    if (!studentsLoaded || m_students.empty()) {
        seedDefaultsIfEmpty();
        saveStudents();
    }

    if (!ticketsLoaded || m_tickets.empty()) {
        seedDefaultsIfEmpty();
        saveTickets();
    }

    // Determine next IDs
    for (const auto& s : m_students) {
        if (s.id >= m_nextStudentId) m_nextStudentId = s.id + 1;
    }
    for (const auto& t : m_tickets) {
        if (t.id >= m_nextTicketId) m_nextTicketId = t.id + 1;
    }

    std::cout << "[Database] Initialized with " << m_students.size() 
              << " students and " << m_tickets.size() << " tickets." << std::endl;
    return true;
}

void DatabaseManager::seedDefaultsIfEmpty() {
    if (m_students.empty()) {
        m_students = {
            {1, "STU2026001", "Aarav Sharma", "aarav.sharma@campus.edu", "+91 9876543210", "Computer Science & Engineering", 6, 8.85, "Active", "2026-10-15 09:30:00"},
            {2, "STU2026002", "Priya Patel", "priya.patel@campus.edu", "+91 9876543211", "Electronics & Communication", 4, 9.12, "Active", "2026-10-16 11:20:00"},
            {3, "STU2026003", "Rohan Mehta", "rohan.mehta@campus.edu", "+91 9876543212", "Mechanical Engineering", 8, 7.94, "Active", "2026-10-18 14:15:00"},
            {4, "STU2026004", "Ananya Iyer", "ananya.iyer@campus.edu", "+91 9876543213", "Business Administration", 2, 8.40, "Active", "2026-10-20 10:00:00"},
            {5, "STU2026005", "Vikram Singh", "vikram.singh@campus.edu", "+91 9876543214", "Computer Science & Engineering", 5, 8.20, "Active", "2026-10-22 16:45:00"},
            {6, "STU2026006", "Neha Gupta", "neha.gupta@campus.edu", "+91 9876543215", "Civil & Environmental Engineering", 3, 8.65, "Active", "2026-10-24 13:10:00"}
        };
    }

    if (m_tickets.empty()) {
        m_tickets = {
            {1, "CC-2026-1001", "STU2026001", "Aarav Sharma", "aarav.sharma@campus.edu", "Computer Science & Engineering", "Academic", "High", "Elective Subject Course Registration Issue", "I am unable to select Cloud Computing CS-402 in the student portal dropdown.", "In Progress", "Academic cell has received your request. We are reviewing system capacity.", "2026-10-25 10:14:00", "2026-10-26 14:30:00"},
            {2, "CC-2026-1002", "STU2026002", "Priya Patel", "priya.patel@campus.edu", "Electronics & Communication", "Technical Support", "Medium", "Campus Wi-Fi connectivity in Library 2nd Floor", "Wi-Fi signal drops frequently in the north corner study cubicles.", "Resolved", "Network access point AP-04 rebooted and firmware updated. Signal restored.", "2026-10-26 09:20:00", "2026-10-27 11:15:00"},
            {3, "CC-2026-1003", "STU2026003", "Rohan Mehta", "rohan.mehta@campus.edu", "Mechanical Engineering", "Complaint", "High", "Lab Machine Calibration Delay", "CNC machine in Workshop 3 requires recalibration for our final year project work.", "Open", "", "2026-10-28 15:45:00", "2026-10-28 15:45:00"},
            {4, "CC-2026-1004", "STU2026004", "Ananya Iyer", "ananya.iyer@campus.edu", "Business Administration", "Feedback", "Low", "Request for Extended Weekend Library Hours", "Students preparing for mid-term exams request the central library remain open until 10 PM on Saturdays and Sundays.", "Open", "", "2026-10-29 11:05:00", "2026-10-29 11:05:00"}
        };
    }
}

bool DatabaseManager::loadStudentsFromFile() {
    std::ifstream file(m_studentsFilePath);
    if (!file.is_open()) return false;

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    m_students.clear();

    size_t pos = 0;
    while ((pos = content.find('{', pos)) != std::string::npos) {
        size_t endPos = content.find('}', pos);
        if (endPos == std::string::npos) break;

        std::string obj = content.substr(pos, endPos - pos + 1);
        Student s;
        std::string idStr = extractJsonField(obj, "id");
        if (!idStr.empty()) s.id = std::stoi(idStr);
        s.studentId = extractJsonField(obj, "studentId");
        s.fullName = extractJsonField(obj, "fullName");
        s.email = extractJsonField(obj, "email");
        s.phone = extractJsonField(obj, "phone");
        s.department = extractJsonField(obj, "department");
        std::string semStr = extractJsonField(obj, "semester");
        if (!semStr.empty()) s.semester = std::stoi(semStr);
        std::string cgpaStr = extractJsonField(obj, "cgpa");
        if (!cgpaStr.empty()) s.cgpa = std::stod(cgpaStr);
        s.status = extractJsonField(obj, "status");
        if (s.status.empty()) s.status = "Active";
        s.createdAt = extractJsonField(obj, "createdAt");

        if (!s.studentId.empty() && !s.fullName.empty()) {
            m_students.push_back(s);
        }
        pos = endPos + 1;
    }

    return true;
}

bool DatabaseManager::loadTicketsFromFile() {
    std::ifstream file(m_ticketsFilePath);
    if (!file.is_open()) return false;

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    m_tickets.clear();

    size_t pos = 0;
    while ((pos = content.find('{', pos)) != std::string::npos) {
        size_t endPos = content.find('}', pos);
        if (endPos == std::string::npos) break;

        std::string obj = content.substr(pos, endPos - pos + 1);
        Ticket t;
        std::string idStr = extractJsonField(obj, "id");
        if (!idStr.empty()) t.id = std::stoi(idStr);
        t.ticketCode = extractJsonField(obj, "ticketCode");
        t.studentId = extractJsonField(obj, "studentId");
        t.studentName = extractJsonField(obj, "studentName");
        t.email = extractJsonField(obj, "email");
        t.department = extractJsonField(obj, "department");
        t.queryType = extractJsonField(obj, "queryType");
        t.priority = extractJsonField(obj, "priority");
        if (t.priority.empty()) t.priority = "Medium";
        t.subject = extractJsonField(obj, "subject");
        t.message = extractJsonField(obj, "message");
        t.status = extractJsonField(obj, "status");
        if (t.status.empty()) t.status = "Open";
        t.staffResponse = extractJsonField(obj, "staffResponse");
        t.createdAt = extractJsonField(obj, "createdAt");
        t.updatedAt = extractJsonField(obj, "updatedAt");

        if (!t.ticketCode.empty() && !t.studentId.empty()) {
            m_tickets.push_back(t);
        }
        pos = endPos + 1;
    }

    return true;
}

bool DatabaseManager::saveStudents() {
    std::ofstream file(m_studentsFilePath);
    if (!file.is_open()) return false;

    file << "[\n";
    for (size_t i = 0; i < m_students.size(); ++i) {
        file << "  " << m_students[i].toJson();
        if (i + 1 < m_students.size()) file << ",";
        file << "\n";
    }
    file << "]\n";
    return true;
}

bool DatabaseManager::saveTickets() {
    std::ofstream file(m_ticketsFilePath);
    if (!file.is_open()) return false;

    file << "[\n";
    for (size_t i = 0; i < m_tickets.size(); ++i) {
        file << "  " << m_tickets[i].toJson();
        if (i + 1 < m_tickets.size()) file << ",";
        file << "\n";
    }
    file << "]\n";
    return true;
}

std::vector<Student> DatabaseManager::getAllStudents(const std::string& search, const std::string& department) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<Student> result;

    std::string sLow = search;
    std::transform(sLow.begin(), sLow.end(), sLow.begin(), ::tolower);

    for (const auto& s : m_students) {
        if (!department.empty() && department != "All" && s.department != department) {
            continue;
        }
        if (!sLow.empty()) {
            std::string nameLow = s.fullName;
            std::transform(nameLow.begin(), nameLow.end(), nameLow.begin(), ::tolower);
            std::string idLow = s.studentId;
            std::transform(idLow.begin(), idLow.end(), idLow.begin(), ::tolower);
            std::string emailLow = s.email;
            std::transform(emailLow.begin(), emailLow.end(), emailLow.begin(), ::tolower);

            if (nameLow.find(sLow) == std::string::npos &&
                idLow.find(sLow) == std::string::npos &&
                emailLow.find(sLow) == std::string::npos) {
                continue;
            }
        }
        result.push_back(s);
    }
    return result;
}

bool DatabaseManager::getStudentById(int id, Student& outStudent) {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (const auto& s : m_students) {
        if (s.id == id) {
            outStudent = s;
            return true;
        }
    }
    return false;
}

bool DatabaseManager::getStudentByStudentId(const std::string& studentId, Student& outStudent) {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (const auto& s : m_students) {
        if (s.studentId == studentId) {
            outStudent = s;
            return true;
        }
    }
    return false;
}

bool DatabaseManager::addStudent(Student& student) {
    std::lock_guard<std::mutex> lock(m_mutex);

    // Check duplicate studentId
    for (const auto& s : m_students) {
        if (s.studentId == student.studentId) {
            return false;
        }
    }

    student.id = m_nextStudentId++;
    if (student.createdAt.empty()) {
        student.createdAt = getCurrentTimestamp();
    }
    if (student.status.empty()) {
        student.status = "Active";
    }

    m_students.push_back(student);
    saveStudents();
    return true;
}

bool DatabaseManager::updateStudent(const Student& student) {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& s : m_students) {
        if (s.id == student.id || s.studentId == student.studentId) {
            s.fullName = student.fullName;
            s.email = student.email;
            s.phone = student.phone;
            s.department = student.department;
            s.semester = student.semester;
            s.cgpa = student.cgpa;
            s.status = student.status;
            saveStudents();
            return true;
        }
    }
    return false;
}

bool DatabaseManager::deleteStudent(int id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto it = m_students.begin(); it != m_students.end(); ++it) {
        if (it->id == id) {
            m_students.erase(it);
            saveStudents();
            return true;
        }
    }
    return false;
}

std::vector<Ticket> DatabaseManager::getAllTickets(const std::string& status, const std::string& queryType) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<Ticket> result;

    for (const auto& t : m_tickets) {
        if (!status.empty() && status != "All" && t.status != status) {
            continue;
        }
        if (!queryType.empty() && queryType != "All" && t.queryType != queryType) {
            continue;
        }
        result.push_back(t);
    }
    return result;
}

bool DatabaseManager::getTicketByCode(const std::string& ticketCode, Ticket& outTicket) {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (const auto& t : m_tickets) {
        if (t.ticketCode == ticketCode) {
            outTicket = t;
            return true;
        }
    }
    return false;
}

std::vector<Ticket> DatabaseManager::getTicketsByStudentId(const std::string& studentId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<Ticket> result;
    for (const auto& t : m_tickets) {
        if (t.studentId == studentId) {
            result.push_back(t);
        }
    }
    return result;
}

bool DatabaseManager::addTicket(Ticket& ticket) {
    std::lock_guard<std::mutex> lock(m_mutex);

    ticket.id = m_nextTicketId++;
    if (ticket.ticketCode.empty()) {
        ticket.ticketCode = generateTicketCode();
    }
    std::string now = getCurrentTimestamp();
    ticket.createdAt = now;
    ticket.updatedAt = now;
    if (ticket.status.empty()) {
        ticket.status = "Open";
    }

    m_tickets.push_back(ticket);
    saveTickets();
    return true;
}

bool DatabaseManager::updateTicketStatus(const std::string& ticketCode, const std::string& newStatus, const std::string& staffResponse) {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& t : m_tickets) {
        if (t.ticketCode == ticketCode) {
            if (!newStatus.empty()) t.status = newStatus;
            if (!staffResponse.empty()) t.staffResponse = staffResponse;
            t.updatedAt = getCurrentTimestamp();
            saveTickets();
            return true;
        }
    }
    return false;
}

SystemStats DatabaseManager::getStatistics() {
    std::lock_guard<std::mutex> lock(m_mutex);
    SystemStats stats;
    stats.totalStudents = static_cast<int>(m_students.size());
    stats.totalTickets = static_cast<int>(m_tickets.size());

    for (const auto& t : m_tickets) {
        if (t.status == "Open") stats.openTickets++;
        else if (t.status == "In Progress") stats.inProgressTickets++;
        else if (t.status == "Resolved" || t.status == "Closed") stats.resolvedTickets++;
    }
    return stats;
}

} // namespace CampusConnect
