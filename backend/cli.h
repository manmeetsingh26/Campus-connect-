#ifndef CLI_H
#define CLI_H

namespace CampusConnect {

class CliInterface {
public:
    static void run();

private:
    static void showBanner();
    static void displayAllStudents();
    static void addNewStudent();
    static void searchStudent();
    static void deleteStudent();
    static void displayAllTickets();
    static void trackTicket();
    static void updateTicketStatus();
    static void viewSystemStats();
};

} // namespace CampusConnect

#endif // CLI_H
