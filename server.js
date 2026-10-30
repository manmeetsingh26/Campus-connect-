/**
 * CampusConnect - Backend Server (Zero-Dependency Node.js Runner)
 * Mirrors the C++ REST API Server so the project runs immediately out-of-the-box!
 * Uses the exact same database files: database/students.json & database/tickets.json
 */

const http = require('http');
const fs = require('fs');
const path = require('path');
const url = require('url');

const PORT = process.env.PORT || 8080;
const DATA_DIR = path.join(__dirname, 'database');
const STUDENTS_FILE = path.join(DATA_DIR, 'students.json');
const TICKETS_FILE = path.join(DATA_DIR, 'tickets.json');

// Ensure database files exist
if (!fs.existsSync(DATA_DIR)) {
    fs.mkdirSync(DATA_DIR, { recursive: true });
}

function readJSON(file, fallback) {
    try {
        if (!fs.existsSync(file)) return fallback;
        const data = fs.readFileSync(file, 'utf8');
        return JSON.parse(data);
    } catch (err) {
        console.error(`Error reading ${file}:`, err.message);
        return fallback;
    }
}

function writeJSON(file, data) {
    try {
        fs.writeFileSync(file, JSON.stringify(data, null, 2), 'utf8');
        return true;
    } catch (err) {
        console.error(`Error writing ${file}:`, err.message);
        return false;
    }
}

function getNextId(items) {
    return items.reduce((max, item) => (item.id > max ? item.id : max), 0) + 1;
}

function generateTicketCode() {
    return 'CC-2026-' + Math.floor(1000 + Math.random() * 9000);
}

function getTimestamp() {
    return new Date().toISOString().replace('T', ' ').substring(0, 19);
}

const MIME_TYPES = {
    '.html': 'text/html; charset=utf-8',
    '.css': 'text/css; charset=utf-8',
    '.js': 'application/javascript; charset=utf-8',
    '.json': 'application/json; charset=utf-8',
    '.png': 'image/png',
    '.jpg': 'image/jpeg',
    '.svg': 'image/svg+xml',
    '.ico': 'image/x-icon'
};

const server = http.createServer((req, res) => {
    // CORS headers
    res.setHeader('Access-Control-Allow-Origin', '*');
    res.setHeader('Access-Control-Allow-Methods', 'GET, POST, PUT, DELETE, OPTIONS');
    res.setHeader('Access-Control-Allow-Headers', 'Content-Type');

    if (req.method === 'OPTIONS') {
        res.writeHead(200);
        return res.end();
    }

    const parsedUrl = url.parse(req.url, true);
    const pathname = parsedUrl.pathname;
    const query = parsedUrl.query;

    const sendJSON = (statusCode, payload) => {
        res.writeHead(statusCode, { 'Content-Type': 'application/json; charset=utf-8' });
        res.end(JSON.stringify(payload));
    };

    let bodyStr = '';
    req.on('data', chunk => { bodyStr += chunk; });
    req.on('end', () => {
        let body = {};
        if (bodyStr) {
            try { body = JSON.parse(bodyStr); } catch (e) {
                // Ignore parse error
            }
        }

        // ==========================================
        // REST API ENDPOINTS
        // ==========================================

        // Health Check
        if (pathname === '/api/health') {
            return sendJSON(200, {
                status: 'ok',
                service: 'CampusConnect Backend (Node/C++ Compatible)',
                timestamp: getTimestamp()
            });
        }

        // System Statistics
        if (pathname === '/api/stats') {
            const students = readJSON(STUDENTS_FILE, []);
            const tickets = readJSON(TICKETS_FILE, []);

            let open = 0, inProgress = 0, resolved = 0;
            tickets.forEach(t => {
                if (t.status === 'Open') open++;
                else if (t.status === 'In Progress') inProgress++;
                else if (t.status === 'Resolved' || t.status === 'Closed') resolved++;
            });

            return sendJSON(200, {
                success: true,
                data: {
                    totalStudents: students.length,
                    totalTickets: tickets.length,
                    openTickets: open,
                    inProgressTickets: inProgress,
                    resolvedTickets: resolved
                }
            });
        }

        // Student Management: /api/students
        if (pathname === '/api/students') {
            const students = readJSON(STUDENTS_FILE, []);

            if (req.method === 'GET') {
                const search = (query.search || '').toLowerCase();
                const dept = query.department || '';

                const filtered = students.filter(s => {
                    if (dept && dept !== 'All' && s.department !== dept) return false;
                    if (search) {
                        const matchesName = (s.fullName || '').toLowerCase().includes(search);
                        const matchesId = (s.studentId || '').toLowerCase().includes(search);
                        const matchesEmail = (s.email || '').toLowerCase().includes(search);
                        return matchesName || matchesId || matchesEmail;
                    }
                    return true;
                });

                return sendJSON(200, { success: true, data: filtered });
            }

            if (req.method === 'POST') {
                if (!body.studentId || !body.fullName || !body.email) {
                    return sendJSON(400, { success: false, message: 'Student ID, Name, and Email are required.' });
                }

                if (students.some(s => s.studentId === body.studentId)) {
                    return sendJSON(409, { success: false, message: 'Student ID already exists.' });
                }

                const newStudent = {
                    id: getNextId(students),
                    studentId: body.studentId.trim(),
                    fullName: body.fullName.trim(),
                    email: body.email.trim(),
                    phone: (body.phone || '').trim(),
                    department: body.department || 'General',
                    semester: parseInt(body.semester, 10) || 1,
                    cgpa: parseFloat(body.cgpa) || 0.0,
                    status: body.status || 'Active',
                    createdAt: getTimestamp()
                };

                students.push(newStudent);
                writeJSON(STUDENTS_FILE, students);
                return sendJSON(201, { success: true, message: 'Student added successfully', data: newStudent });
            }

            if (req.method === 'PUT') {
                const index = students.findIndex(s => s.id === body.id || s.studentId === body.studentId);
                if (index === -1) {
                    return sendJSON(404, { success: false, message: 'Student not found.' });
                }

                students[index] = {
                    ...students[index],
                    fullName: body.fullName !== undefined ? body.fullName : students[index].fullName,
                    email: body.email !== undefined ? body.email : students[index].email,
                    phone: body.phone !== undefined ? body.phone : students[index].phone,
                    department: body.department !== undefined ? body.department : students[index].department,
                    semester: body.semester !== undefined ? parseInt(body.semester, 10) : students[index].semester,
                    cgpa: body.cgpa !== undefined ? parseFloat(body.cgpa) : students[index].cgpa,
                    status: body.status !== undefined ? body.status : students[index].status
                };

                writeJSON(STUDENTS_FILE, students);
                return sendJSON(200, { success: true, message: 'Student updated successfully', data: students[index] });
            }

            if (req.method === 'DELETE') {
                const targetId = parseInt(query.id || body.id, 10);
                const initialLen = students.length;
                const remaining = students.filter(s => s.id !== targetId);

                if (remaining.length === initialLen) {
                    return sendJSON(404, { success: false, message: 'Student record not found.' });
                }

                writeJSON(STUDENTS_FILE, remaining);
                return sendJSON(200, { success: true, message: 'Student record deleted successfully.' });
            }
        }

        // Helpdesk Tickets: /api/tickets
        if (pathname === '/api/tickets') {
            const tickets = readJSON(TICKETS_FILE, []);

            if (req.method === 'GET') {
                const status = query.status || '';
                const queryType = query.queryType || '';

                const filtered = tickets.filter(t => {
                    if (status && status !== 'All' && t.status !== status) return false;
                    if (queryType && queryType !== 'All' && t.queryType !== queryType) return false;
                    return true;
                });

                return sendJSON(200, { success: true, data: filtered });
            }

            if (req.method === 'POST') {
                const studentName = body.name || body.studentName;
                if (!body.studentId || !studentName || !body.email || !body.queryType || !body.subject || !body.message) {
                    return sendJSON(400, { success: false, message: 'All required query fields must be provided.' });
                }

                const newTicket = {
                    id: getNextId(tickets),
                    ticketCode: generateTicketCode(),
                    studentId: body.studentId.trim(),
                    studentName: studentName.trim(),
                    email: body.email.trim(),
                    department: body.department || 'General',
                    queryType: body.queryType,
                    priority: body.priority || 'Medium',
                    subject: body.subject.trim(),
                    message: body.message.trim(),
                    status: 'Open',
                    staffResponse: '',
                    createdAt: getTimestamp(),
                    updatedAt: getTimestamp()
                };

                tickets.push(newTicket);
                writeJSON(TICKETS_FILE, tickets);
                return sendJSON(201, {
                    success: true,
                    message: 'Query submitted successfully!',
                    ticketCode: newTicket.ticketCode,
                    data: newTicket
                });
            }

            if (req.method === 'PUT') {
                const ticketCode = body.ticketCode;
                const index = tickets.findIndex(t => t.ticketCode === ticketCode);
                if (index === -1) {
                    return sendJSON(404, { success: false, message: 'Ticket not found.' });
                }

                if (body.status) tickets[index].status = body.status;
                if (body.staffResponse !== undefined) tickets[index].staffResponse = body.staffResponse;
                tickets[index].updatedAt = getTimestamp();

                writeJSON(TICKETS_FILE, tickets);
                return sendJSON(200, { success: true, message: 'Ticket updated successfully', data: tickets[index] });
            }
        }

        // Track Ticket: /api/tickets/track
        if (pathname === '/api/tickets/track') {
            const tickets = readJSON(TICKETS_FILE, []);
            const code = query.code;
            const studentId = query.studentId;

            if (code) {
                const ticket = tickets.find(t => t.ticketCode.toUpperCase() === code.trim().toUpperCase());
                if (!ticket) {
                    return sendJSON(404, { success: false, message: `Ticket ${code} not found.` });
                }
                return sendJSON(200, { success: true, data: ticket });
            }

            if (studentId) {
                const list = tickets.filter(t => t.studentId.toUpperCase() === studentId.trim().toUpperCase());
                return sendJSON(200, { success: true, data: list });
            }

            return sendJSON(400, { success: false, message: 'Please provide ticket code or studentId.' });
        }

        // ==========================================
        // STATIC FILE SERVING
        // ==========================================
        let relativePath = pathname === '/' ? 'index.html' : pathname.replace(/^\//, '');
        let safePath = path.normalize(path.join(__dirname, relativePath));

        if (!safePath.startsWith(__dirname)) {
            res.writeHead(403);
            return res.end('Access Denied');
        }

        fs.readFile(safePath, (err, content) => {
            if (err) {
                res.writeHead(404, { 'Content-Type': 'text/html; charset=utf-8' });
                return res.end('<h1>404 - Not Found</h1><p>CampusConnect Server</p>');
            }
            const ext = path.extname(safePath).toLowerCase();
            const contentType = MIME_TYPES[ext] || 'application/octet-stream';
            res.writeHead(200, { 'Content-Type': contentType });
            res.end(content);
        });
    });
});

server.listen(PORT, () => {
    console.log('============================================================');
    console.log(` CampusConnect Server running at: http://localhost:${PORT}/`);
    console.log(` Web Portal      : http://localhost:${PORT}/index.html`);
    console.log(` Student Helpdesk: http://localhost:${PORT}/contact.html`);
    console.log(` Student Records : http://localhost:${PORT}/students.html`);
    console.log(` Admin Console   : http://localhost:${PORT}/admin.html`);
    console.log('============================================================');
});
