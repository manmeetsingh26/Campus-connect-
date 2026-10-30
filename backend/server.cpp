#include "server.h"
#include "database.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <map>
#include <chrono>

#if defined(_WIN32) || defined(_WIN64)
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
typedef SOCKET socket_t;
#define CLOSE_SOCKET(s) closesocket(s)
#define IS_INVALID_SOCKET(s) ((s) == INVALID_SOCKET)
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
typedef int socket_t;
#define CLOSE_SOCKET(s) close(s)
#define IS_INVALID_SOCKET(s) ((s) < 0)
#endif

namespace CampusConnect {

static std::string urlDecode(const std::string& str) {
    std::string res;
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '+') {
            res += ' ';
        } else if (str[i] == '%' && i + 2 < str.length()) {
            int val = 0;
            std::istringstream is(str.substr(i + 1, 2));
            if (is >> std::hex >> val) {
                res += static_cast<char>(val);
                i += 2;
            } else {
                res += '%';
            }
        } else {
            res += str[i];
        }
    }
    return res;
}

static std::map<std::string, std::string> parseQueryParams(const std::string& qs) {
    std::map<std::string, std::string> params;
    std::stringstream ss(qs);
    std::string pair;
    while (std::getline(ss, pair, '&')) {
        size_t eq = pair.find('=');
        if (eq != std::string::npos) {
            std::string key = urlDecode(pair.substr(0, eq));
            std::string val = urlDecode(pair.substr(eq + 1));
            params[key] = val;
        } else if (!pair.empty()) {
            params[urlDecode(pair)] = "";
        }
    }
    return params;
}

static std::string getJsonField(const std::string& obj, const std::string& key) {
    std::string needle = "\"" + key + "\"";
    size_t pos = obj.find(needle);
    if (pos == std::string::npos) return "";

    pos = obj.find(':', pos + needle.length());
    if (pos == std::string::npos) return "";

    pos++;
    while (pos < obj.length() && (obj[pos] == ' ' || obj[pos] == '\t' || obj[pos] == '\r' || obj[pos] == '\n')) {
        pos++;
    }

    if (pos >= obj.length()) return "";

    if (obj[pos] == '"') {
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
        size_t endPos = obj.find_first_of(",}\n\r", pos);
        if (endPos == std::string::npos) endPos = obj.length();
        std::string raw = obj.substr(pos, endPos - pos);
        size_t first = raw.find_first_not_of(" \t\n\r");
        if (first == std::string::npos) return "";
        size_t last = raw.find_last_not_of(" \t\n\r");
        return raw.substr(first, (last - first + 1));
    }
}

HttpServer::HttpServer(int port, const std::string& staticDir)
    : m_port(port), m_staticDir(staticDir), m_running(false) {
#if defined(_WIN32) || defined(_WIN64)
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
}

HttpServer::~HttpServer() {
    stop();
#if defined(_WIN32) || defined(_WIN64)
    WSACleanup();
#endif
}

bool HttpServer::start() {
    if (m_running) return true;

    socket_t s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (IS_INVALID_SOCKET(s)) {
        std::cerr << "[HttpServer] Failed to create socket." << std::endl;
        return false;
    }

    int opt = 1;
#if defined(_WIN32) || defined(_WIN64)
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
#else
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(static_cast<u_short>(m_port));

    if (bind(s, (sockaddr*)&serverAddr, sizeof(serverAddr)) != 0) {
        std::cerr << "[HttpServer] Bind failed on port " << m_port << "." << std::endl;
        CLOSE_SOCKET(s);
        return false;
    }

    if (listen(s, SOMAXCONN) != 0) {
        std::cerr << "[HttpServer] Listen failed." << std::endl;
        CLOSE_SOCKET(s);
        return false;
    }

    m_serverSocket = (uintptr_t)s;
    m_running = true;

    std::cout << "[HttpServer] ==============================================" << std::endl;
    std::cout << "[HttpServer] CampusConnect Server running at http://localhost:" << m_port << "/" << std::endl;
    std::cout << "[HttpServer] REST API endpoints available at /api/*" << std::endl;
    std::cout << "[HttpServer] ==============================================" << std::endl;

    m_listenerThread = std::thread(&HttpServer::listenLoop, this);
    return true;
}

void HttpServer::stop() {
    if (!m_running) return;
    m_running = false;

    if (m_serverSocket != 0) {
        CLOSE_SOCKET((socket_t)m_serverSocket);
        m_serverSocket = 0;
    }

    if (m_listenerThread.joinable()) {
        m_listenerThread.join();
    }
}

void HttpServer::listenLoop() {
    while (m_running) {
        sockaddr_in clientAddr{};
        int clientLen = sizeof(clientAddr);
        socket_t clientSock = accept((socket_t)m_serverSocket, (sockaddr*)&clientAddr, &clientLen);

        if (IS_INVALID_SOCKET(clientSock)) {
            if (!m_running) break;
            continue;
        }

        // Spawn detached worker thread for client request
        std::thread([this, clientSock]() {
            this->handleClient((uintptr_t)clientSock);
        }).detach();
    }
}

void HttpServer::handleClient(uintptr_t clientSockPtr) {
    socket_t clientSock = (socket_t)clientSockPtr;
    std::string rawRequest;
    char buffer[4096];
    int bytesRead = 0;

    // Read initial HTTP headers
    while ((bytesRead = recv(clientSock, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytesRead] = '\0';
        rawRequest.append(buffer, bytesRead);
        if (rawRequest.find("\r\n\r\n") != std::string::npos) {
            break;
        }
    }

    if (rawRequest.empty()) {
        CLOSE_SOCKET(clientSock);
        return;
    }

    size_t headerEnd = rawRequest.find("\r\n\r\n");
    std::string headerPart = rawRequest.substr(0, headerEnd);
    std::string bodyPart = (headerEnd != std::string::npos) ? rawRequest.substr(headerEnd + 4) : "";

    // Parse Content-Length to read remaining body if any
    size_t clPos = headerPart.find("Content-Length:");
    if (clPos == std::string::npos) clPos = headerPart.find("content-length:");
    if (clPos != std::string::npos) {
        size_t endLine = headerPart.find("\r\n", clPos);
        std::string clStr = headerPart.substr(clPos + 15, endLine - (clPos + 15));
        int contentLen = 0;
        try { contentLen = std::stoi(clStr); } catch (...) {}
        while (bodyPart.length() < (size_t)contentLen) {
            bytesRead = recv(clientSock, buffer, sizeof(buffer) - 1, 0);
            if (bytesRead <= 0) break;
            buffer[bytesRead] = '\0';
            bodyPart.append(buffer, bytesRead);
        }
    }

    // Parse Request Line: METHOD /path?query HTTP/1.1
    std::istringstream reqStream(headerPart);
    std::string method, fullPath, version;
    reqStream >> method >> fullPath >> version;

    std::string path = fullPath;
    std::string queryString;
    size_t qPos = fullPath.find('?');
    if (qPos != std::string::npos) {
        path = fullPath.substr(0, qPos);
        queryString = fullPath.substr(qPos + 1);
    }

    // Process HTTP method and build response
    std::string response = processRequest(method, path, queryString, bodyPart);

    // Send complete response
    send(clientSock, response.c_str(), (int)response.length(), 0);
    CLOSE_SOCKET(clientSock);
}

std::string HttpServer::getContentType(const std::string& path) {
    if (path.rfind(".html") != std::string::npos) return "text/html; charset=utf-8";
    if (path.rfind(".css") != std::string::npos) return "text/css; charset=utf-8";
    if (path.rfind(".js") != std::string::npos) return "application/javascript; charset=utf-8";
    if (path.rfind(".json") != std::string::npos) return "application/json; charset=utf-8";
    if (path.rfind(".png") != std::string::npos) return "image/png";
    if (path.rfind(".jpg") != std::string::npos || path.rfind(".jpeg") != std::string::npos) return "image/jpeg";
    if (path.rfind(".svg") != std::string::npos) return "image/svg+xml";
    if (path.rfind(".ico") != std::string::npos) return "image/x-icon";
    return "text/plain; charset=utf-8";
}

std::string HttpServer::buildResponse(int statusCode, const std::string& statusText, 
                                      const std::string& contentType, const std::string& body) {
    std::ostringstream ss;
    ss << "HTTP/1.1 " << statusCode << " " << statusText << "\r\n";
    ss << "Content-Type: " << contentType << "\r\n";
    ss << "Content-Length: " << body.length() << "\r\n";
    ss << "Access-Control-Allow-Origin: *\r\n";
    ss << "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n";
    ss << "Access-Control-Allow-Headers: Content-Type\r\n";
    ss << "Connection: close\r\n\r\n";
    ss << body;
    return ss.str();
}

std::string HttpServer::processRequest(const std::string& method, 
                                       const std::string& path, 
                                       const std::string& queryString, 
                                       const std::string& body) {
    // 1. CORS Pre-flight handler
    if (method == "OPTIONS") {
        return buildResponse(200, "OK", "text/plain", "");
    }

    auto& db = DatabaseManager::getInstance();
    auto params = parseQueryParams(queryString);

    // ========================================================
    // REST API ENDPOINTS
    // ========================================================

    // GET /api/health
    if (path == "/api/health") {
        std::string json = "{\"status\":\"ok\",\"service\":\"CampusConnect C++ Backend\",\"timestamp\":\"" +
                           db.getCurrentTimestamp() + "\"}";
        return buildResponse(200, "OK", "application/json", json);
    }

    // GET /api/stats
    if (path == "/api/stats") {
        SystemStats stats = db.getStatistics();
        std::string json = "{\"success\":true,\"data\":" + stats.toJson() + "}";
        return buildResponse(200, "OK", "application/json", json);
    }

    // /api/students
    if (path == "/api/students") {
        if (method == "GET") {
            std::string search = params["search"];
            std::string dept = params["department"];
            auto students = db.getAllStudents(search, dept);

            std::ostringstream ss;
            ss << "{\"success\":true,\"data\":[";
            for (size_t i = 0; i < students.size(); ++i) {
                ss << students[i].toJson();
                if (i + 1 < students.size()) ss << ",";
            }
            ss << "]}";
            return buildResponse(200, "OK", "application/json", ss.str());
        } 
        else if (method == "POST") {
            Student s;
            s.studentId = getJsonField(body, "studentId");
            s.fullName = getJsonField(body, "fullName");
            s.email = getJsonField(body, "email");
            s.phone = getJsonField(body, "phone");
            s.department = getJsonField(body, "department");
            std::string semStr = getJsonField(body, "semester");
            if (!semStr.empty()) s.semester = std::stoi(semStr);
            std::string cgpaStr = getJsonField(body, "cgpa");
            if (!cgpaStr.empty()) s.cgpa = std::stod(cgpaStr);
            s.status = getJsonField(body, "status");

            if (s.studentId.empty() || s.fullName.empty() || s.email.empty()) {
                return buildResponse(400, "Bad Request", "application/json", 
                    "{\"success\":false,\"message\":\"studentId, fullName, and email are required.\"}");
            }

            if (db.addStudent(s)) {
                return buildResponse(201, "Created", "application/json", 
                    "{\"success\":true,\"message\":\"Student added successfully\",\"data\":" + s.toJson() + "}");
            } else {
                return buildResponse(409, "Conflict", "application/json", 
                    "{\"success\":false,\"message\":\"Student with this ID already exists.\"}");
            }
        }
        else if (method == "PUT") {
            Student s;
            std::string idStr = getJsonField(body, "id");
            if (!idStr.empty()) s.id = std::stoi(idStr);
            s.studentId = getJsonField(body, "studentId");
            s.fullName = getJsonField(body, "fullName");
            s.email = getJsonField(body, "email");
            s.phone = getJsonField(body, "phone");
            s.department = getJsonField(body, "department");
            std::string semStr = getJsonField(body, "semester");
            if (!semStr.empty()) s.semester = std::stoi(semStr);
            std::string cgpaStr = getJsonField(body, "cgpa");
            if (!cgpaStr.empty()) s.cgpa = std::stod(cgpaStr);
            s.status = getJsonField(body, "status");

            if (db.updateStudent(s)) {
                return buildResponse(200, "OK", "application/json", 
                    "{\"success\":true,\"message\":\"Student updated successfully\"}");
            } else {
                return buildResponse(404, "Not Found", "application/json", 
                    "{\"success\":false,\"message\":\"Student not found.\"}");
            }
        }
        else if (method == "DELETE") {
            int id = 0;
            if (!params["id"].empty()) {
                id = std::stoi(params["id"]);
            } else {
                std::string idStr = getJsonField(body, "id");
                if (!idStr.empty()) id = std::stoi(idStr);
            }

            if (id > 0 && db.deleteStudent(id)) {
                return buildResponse(200, "OK", "application/json", 
                    "{\"success\":true,\"message\":\"Student deleted successfully\"}");
            } else {
                return buildResponse(404, "Not Found", "application/json", 
                    "{\"success\":false,\"message\":\"Student not found or invalid ID.\"}");
            }
        }
    }

    // /api/tickets
    if (path == "/api/tickets") {
        if (method == "GET") {
            std::string status = params["status"];
            std::string type = params["queryType"];
            auto tickets = db.getAllTickets(status, type);

            std::ostringstream ss;
            ss << "{\"success\":true,\"data\":[";
            for (size_t i = 0; i < tickets.size(); ++i) {
                ss << tickets[i].toJson();
                if (i + 1 < tickets.size()) ss << ",";
            }
            ss << "]}";
            return buildResponse(200, "OK", "application/json", ss.str());
        }
        else if (method == "POST") {
            Ticket t;
            t.studentId = getJsonField(body, "studentId");
            t.studentName = getJsonField(body, "name");
            if (t.studentName.empty()) t.studentName = getJsonField(body, "studentName");
            t.email = getJsonField(body, "email");
            t.department = getJsonField(body, "department");
            t.queryType = getJsonField(body, "queryType");
            t.priority = getJsonField(body, "priority");
            t.subject = getJsonField(body, "subject");
            t.message = getJsonField(body, "message");

            if (t.studentId.empty() || t.studentName.empty() || t.email.empty() || 
                t.queryType.empty() || t.subject.empty() || t.message.empty()) {
                return buildResponse(400, "Bad Request", "application/json", 
                    "{\"success\":false,\"message\":\"Please fill in all required query fields.\"}");
            }

            if (db.addTicket(t)) {
                return buildResponse(201, "Created", "application/json", 
                    "{\"success\":true,\"message\":\"Query submitted successfully!\",\"ticketCode\":\"" + 
                    t.ticketCode + "\",\"data\":" + t.toJson() + "}");
            } else {
                return buildResponse(500, "Internal Error", "application/json", 
                    "{\"success\":false,\"message\":\"Failed to save ticket.\"}");
            }
        }
        else if (method == "PUT") {
            std::string code = getJsonField(body, "ticketCode");
            std::string status = getJsonField(body, "status");
            std::string responseNote = getJsonField(body, "staffResponse");

            if (code.empty()) {
                return buildResponse(400, "Bad Request", "application/json", 
                    "{\"success\":false,\"message\":\"ticketCode is required.\"}");
            }

            if (db.updateTicketStatus(code, status, responseNote)) {
                return buildResponse(200, "OK", "application/json", 
                    "{\"success\":true,\"message\":\"Ticket updated successfully\"}");
            } else {
                return buildResponse(404, "Not Found", "application/json", 
                    "{\"success\":false,\"message\":\"Ticket not found.\"}");
            }
        }
    }

    // /api/tickets/track?code=... OR ?studentId=...
    if (path == "/api/tickets/track") {
        std::string code = params["code"];
        std::string studentId = params["studentId"];

        if (!code.empty()) {
            Ticket t;
            if (db.getTicketByCode(code, t)) {
                return buildResponse(200, "OK", "application/json", 
                    "{\"success\":true,\"data\":" + t.toJson() + "}");
            } else {
                return buildResponse(404, "Not Found", "application/json", 
                    "{\"success\":false,\"message\":\"Ticket not found.\"}");
            }
        } else if (!studentId.empty()) {
            auto tickets = db.getTicketsByStudentId(studentId);
            std::ostringstream ss;
            ss << "{\"success\":true,\"data\":[";
            for (size_t i = 0; i < tickets.size(); ++i) {
                ss << tickets[i].toJson();
                if (i + 1 < tickets.size()) ss << ",";
            }
            ss << "]}";
            return buildResponse(200, "OK", "application/json", ss.str());
        }
        return buildResponse(400, "Bad Request", "application/json", 
            "{\"success\":false,\"message\":\"Provide code or studentId parameter to track.\"}");
    }

    // ========================================================
    // STATIC FILE SERVING
    // ========================================================
    std::string filePath = m_staticDir + path;
    if (path == "/" || path.empty()) {
        filePath = m_staticDir + "/index.html";
    }

    std::ifstream file(filePath, std::ios::binary);
    if (file.is_open()) {
        std::stringstream ss;
        ss << file.rdbuf();
        std::string content = ss.str();
        return buildResponse(200, "OK", getContentType(filePath), content);
    }

    // Not found
    std::string notFoundBody = "<html><head><title>404 Not Found</title></head><body><h1>404 - Page Not Found</h1><p>CampusConnect Server</p></body></html>";
    return buildResponse(404, "Not Found", "text/html", notFoundBody);
}

} // namespace CampusConnect
