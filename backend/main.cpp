#include <iostream>
#include <string>
#include "database.h"
#include "server.h"
#include "cli.h"

int main(int argc, char* argv[]) {
    int port = 8080;
    bool cliModeOnly = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--cli") {
            cliModeOnly = true;
        } else if (arg == "--port" && i + 1 < argc) {
            port = std::stoi(argv[++i]);
        }
    }

    // Initialize Database
    CampusConnect::DatabaseManager& db = CampusConnect::DatabaseManager::getInstance();
    if (!db.initialize("database")) {
        std::cerr << "[!] Failed to initialize database.\n";
        return 1;
    }

    if (cliModeOnly) {
        CampusConnect::CliInterface::run();
        return 0;
    }

    // Start HTTP REST & Web Server
    CampusConnect::HttpServer server(port, ".");
    if (!server.start()) {
        std::cerr << "[!] Failed to start HTTP server on port " << port << ".\n";
        return 1;
    }

    std::cout << "\n============================================================\n";
    std::cout << "         CAMPUS CONNECT - FULL STACK SYSTEM ONLINE          \n";
    std::cout << "============================================================\n";
    std::cout << " Web Portal URL     : http://localhost:" << port << "/\n";
    std::cout << " Helpdesk & Query   : http://localhost:" << port << "/contact.html\n";
    std::cout << " Student Directory  : http://localhost:" << port << "/students.html\n";
    std::cout << " Admin Dashboard    : http://localhost:" << port << "/admin.html\n";
    std::cout << " REST API Health    : http://localhost:" << port << "/api/health\n";
    std::cout << "============================================================\n";
    std::cout << " Controls:\n";
    std::cout << "   - Type 'c' then Enter to open interactive CLI menu\n";
    std::cout << "   - Type 'q' then Enter to stop the server\n";
    std::cout << "============================================================\n\n";

    std::string command;
    while (server.isRunning() && std::cin >> command) {
        if (command == "q" || command == "quit" || command == "exit") {
            std::cout << "[*] Shutting down CampusConnect server...\n";
            server.stop();
            break;
        } else if (command == "c" || command == "cli" || command == "menu") {
            CampusConnect::CliInterface::run();
            std::cout << "\n[HttpServer] Server is still active at http://localhost:" << port << "/\n";
            std::cout << "Enter 'c' for CLI menu or 'q' to quit: ";
        } else {
            std::cout << "Unknown command '" << command << "'. Enter 'c' for CLI menu or 'q' to quit: ";
        }
    }

    return 0;
}
