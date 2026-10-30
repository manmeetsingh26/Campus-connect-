#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <atomic>
#include <thread>
#include <vector>

namespace CampusConnect {

class HttpServer {
public:
    HttpServer(int port = 8080, const std::string& staticDir = ".");
    ~HttpServer();

    bool start();
    void stop();
    bool isRunning() const { return m_running; }
    int getPort() const { return m_port; }

private:
    void listenLoop();
    void handleClient(uintptr_t clientSocket);

    // HTTP Request Handler
    std::string processRequest(const std::string& requestMethod, 
                               const std::string& requestPath, 
                               const std::string& queryString, 
                               const std::string& body);

    // Helpers
    std::string getContentType(const std::string& path);
    std::string buildResponse(int statusCode, const std::string& statusText, 
                              const std::string& contentType, const std::string& body);

    int m_port;
    std::string m_staticDir;
    std::atomic<bool> m_running{false};
    uintptr_t m_serverSocket{0};
    std::thread m_listenerThread;
};

} // namespace CampusConnect

#endif // SERVER_H
