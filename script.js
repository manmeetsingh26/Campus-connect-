/**
 * ============================================================
 * CampusConnect - Frontend Engine & REST API Client
 * Supports Live C++/Node Backend with Seamless LocalStorage Fallback!
 * ============================================================
 */

// Default Seed Datasets (Used when running in offline/localStorage mode)
const DEFAULT_STUDENTS = [
    { id: 1, studentId: "STU2026001", fullName: "Aarav Sharma", email: "aarav.sharma@campus.edu", phone: "+91 9876543210", department: "Computer Science & Engineering", semester: 6, cgpa: 8.85, status: "Active", createdAt: "2026-10-15 09:30:00" },
    { id: 2, studentId: "STU2026002", fullName: "Priya Patel", email: "priya.patel@campus.edu", phone: "+91 9876543211", department: "Electronics & Communication", semester: 4, cgpa: 9.12, status: "Active", createdAt: "2026-10-16 11:20:00" },
    { id: 3, studentId: "STU2026003", fullName: "Rohan Mehta", email: "rohan.mehta@campus.edu", phone: "+91 9876543212", department: "Mechanical Engineering", semester: 8, cgpa: 7.94, status: "Active", createdAt: "2026-10-18 14:15:00" },
    { id: 4, studentId: "STU2026004", fullName: "Ananya Iyer", email: "ananya.iyer@campus.edu", phone: "+91 9876543213", department: "Business Administration", semester: 2, cgpa: 8.40, status: "Active", createdAt: "2026-10-20 10:00:00" },
    { id: 5, studentId: "STU2026005", fullName: "Vikram Singh", email: "vikram.singh@campus.edu", phone: "+91 9876543214", department: "Computer Science & Engineering", semester: 5, cgpa: 8.20, status: "Active", createdAt: "2026-10-22 16:45:00" },
    { id: 6, studentId: "STU2026006", fullName: "Neha Gupta", email: "neha.gupta@campus.edu", phone: "+91 9876543215", department: "Civil & Environmental Engineering", semester: 3, cgpa: 8.65, status: "Active", createdAt: "2026-10-24 13:10:00" }
];

const DEFAULT_TICKETS = [
    { id: 1, ticketCode: "CC-2026-1001", studentId: "STU2026001", studentName: "Aarav Sharma", email: "aarav.sharma@campus.edu", department: "Computer Science & Engineering", queryType: "Academic", priority: "High", subject: "Elective Subject Course Registration Issue", message: "I am unable to select Cloud Computing CS-402 in the student portal dropdown.", status: "In Progress", staffResponse: "Academic cell has received your request. We are reviewing system capacity.", createdAt: "2026-10-25 10:14:00", updatedAt: "2026-10-26 14:30:00" },
    { id: 2, ticketCode: "CC-2026-1002", studentId: "STU2026002", studentName: "Priya Patel", email: "priya.patel@campus.edu", department: "Electronics & Communication", queryType: "Technical Support", priority: "Medium", subject: "Campus Wi-Fi connectivity in Library 2nd Floor", message: "Wi-Fi signal drops frequently in the north corner study cubicles.", status: "Resolved", staffResponse: "Network access point AP-04 rebooted and firmware updated. Signal restored.", createdAt: "2026-10-26 09:20:00", updatedAt: "2026-10-27 11:15:00" },
    { id: 3, ticketCode: "CC-2026-1003", studentId: "STU2026003", studentName: "Rohan Mehta", email: "rohan.mehta@campus.edu", department: "Mechanical Engineering", queryType: "Complaint", priority: "High", subject: "Lab Machine Calibration Delay", message: "CNC machine in Workshop 3 requires recalibration for our final year project work.", status: "Open", staffResponse: "", createdAt: "2026-10-28 15:45:00", updatedAt: "2026-10-28 15:45:00" },
    { id: 4, ticketCode: "CC-2026-1004", studentId: "STU2026004", studentName: "Ananya Iyer", email: "ananya.iyer@campus.edu", department: "Business Administration", queryType: "Feedback", priority: "Low", subject: "Request for Extended Weekend Library Hours", message: "Students preparing for mid-term exams request the central library remain open until 10 PM on Saturdays and Sundays.", status: "Open", staffResponse: "", createdAt: "2026-10-29 11:05:00", updatedAt: "2026-10-29 11:05:00" }
];

// Determine API Base URL
const API_BASE = window.location.protocol.startsWith('http') ? '' : 'http://localhost:8080';

class CampusConnectAPI {
    constructor() {
        this.backendOnline = false;
        this.checkBackendStatus();
    }

    async checkBackendStatus() {
        try {
            const res = await fetch(`${API_BASE}/api/health`, { method: 'GET' });
            if (res.ok) {
                this.backendOnline = true;
                this.updateStatusPill(true);
                return true;
            }
        } catch (e) {
            this.backendOnline = false;
        }
        this.updateStatusPill(false);
        this.initLocalStorage();
        return false;
    }

    updateStatusPill(online) {
        const pill = document.getElementById('system-status');
        if (!pill) return;
        if (online) {
            pill.innerHTML = `<span class="status-dot"></span> Backend Live`;
            pill.title = "Connected to CampusConnect C++ / Node HTTP Server";
        } else {
            pill.innerHTML = `<span class="status-dot offline"></span> Local Storage`;
            pill.title = "Operating in offline local storage mode. Start server for live sync.";
        }
    }

    initLocalStorage() {
        if (!localStorage.getItem('cc_students')) {
            localStorage.setItem('cc_students', JSON.stringify(DEFAULT_STUDENTS));
        }
        if (!localStorage.getItem('cc_tickets')) {
            localStorage.setItem('cc_tickets', JSON.stringify(DEFAULT_TICKETS));
        }
    }

    // --- Statistics ---
    async getStats() {
        if (this.backendOnline) {
            try {
                const res = await fetch(`${API_BASE}/api/stats`);
                const json = await res.json();
                if (json.success) return json.data;
            } catch (e) { console.warn("API fallback to LocalStorage", e); }
        }
        const students = JSON.parse(localStorage.getItem('cc_students') || '[]');
        const tickets = JSON.parse(localStorage.getItem('cc_tickets') || '[]');
        let open = 0, inProgress = 0, resolved = 0;
        tickets.forEach(t => {
            if (t.status === 'Open') open++;
            else if (t.status === 'In Progress') inProgress++;
            else if (t.status === 'Resolved' || t.status === 'Closed') resolved++;
        });
        return {
            totalStudents: students.length,
            totalTickets: tickets.length,
            openTickets: open,
            inProgressTickets: inProgress,
            resolvedTickets: resolved
        };
    }

    // --- Students Operations ---
    async getStudents(search = '', department = '') {
        if (this.backendOnline) {
            try {
                const url = new URL(`${API_BASE}/api/students`, window.location.href);
                if (search) url.searchParams.set('search', search);
                if (department) url.searchParams.set('department', department);
                const res = await fetch(url);
                const json = await res.json();
                if (json.success) return json.data;
            } catch (e) { console.warn("API fallback", e); }
        }
        let list = JSON.parse(localStorage.getItem('cc_students') || '[]');
        if (department && department !== 'All') {
            list = list.filter(s => s.department === department);
        }
        if (search) {
            const q = search.toLowerCase();
            list = list.filter(s => 
                (s.fullName || '').toLowerCase().includes(q) ||
                (s.studentId || '').toLowerCase().includes(q) ||
                (s.email || '').toLowerCase().includes(q)
            );
        }
        return list;
    }

    async addStudent(studentData) {
        if (this.backendOnline) {
            try {
                const res = await fetch(`${API_BASE}/api/students`, {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify(studentData)
                });
                const json = await res.json();
                if (!res.ok) throw new Error(json.message || "Failed to add student");
                return json;
            } catch (e) {
                if (e.message.includes("already exists")) throw e;
            }
        }
        const list = JSON.parse(localStorage.getItem('cc_students') || '[]');
        if (list.some(s => s.studentId === studentData.studentId)) {
            throw new Error(`Student with ID ${studentData.studentId} already exists.`);
        }
        const newStudent = {
            id: Date.now(),
            ...studentData,
            semester: parseInt(studentData.semester, 10) || 1,
            cgpa: parseFloat(studentData.cgpa) || 0.0,
            status: studentData.status || 'Active',
            createdAt: new Date().toISOString().replace('T', ' ').substring(0, 19)
        };
        list.push(newStudent);
        localStorage.setItem('cc_students', JSON.stringify(list));
        return { success: true, data: newStudent };
    }

    async updateStudent(studentData) {
        if (this.backendOnline) {
            try {
                const res = await fetch(`${API_BASE}/api/students`, {
                    method: 'PUT',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify(studentData)
                });
                const json = await res.json();
                if (res.ok) return json;
            } catch (e) {}
        }
        let list = JSON.parse(localStorage.getItem('cc_students') || '[]');
        const idx = list.findIndex(s => s.id === studentData.id || s.studentId === studentData.studentId);
        if (idx !== -1) {
            list[idx] = { ...list[idx], ...studentData };
            localStorage.setItem('cc_students', JSON.stringify(list));
            return { success: true, data: list[idx] };
        }
        throw new Error("Student record not found.");
    }

    async deleteStudent(id) {
        if (this.backendOnline) {
            try {
                const res = await fetch(`${API_BASE}/api/students?id=${id}`, { method: 'DELETE' });
                const json = await res.json();
                if (res.ok) return json;
            } catch (e) {}
        }
        let list = JSON.parse(localStorage.getItem('cc_students') || '[]');
        list = list.filter(s => s.id !== id);
        localStorage.setItem('cc_students', JSON.stringify(list));
        return { success: true };
    }

    // --- Tickets Operations ---
    async getTickets(status = '', queryType = '') {
        if (this.backendOnline) {
            try {
                const url = new URL(`${API_BASE}/api/tickets`, window.location.href);
                if (status) url.searchParams.set('status', status);
                if (queryType) url.searchParams.set('queryType', queryType);
                const res = await fetch(url);
                const json = await res.json();
                if (json.success) return json.data;
            } catch (e) {}
        }
        let list = JSON.parse(localStorage.getItem('cc_tickets') || '[]');
        if (status && status !== 'All') {
            list = list.filter(t => t.status === status);
        }
        if (queryType && queryType !== 'All') {
            list = list.filter(t => t.queryType === queryType);
        }
        return list;
    }

    async addTicket(ticketData) {
        if (this.backendOnline) {
            try {
                const res = await fetch(`${API_BASE}/api/tickets`, {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify(ticketData)
                });
                const json = await res.json();
                if (res.ok) return json;
            } catch (e) {}
        }
        const list = JSON.parse(localStorage.getItem('cc_tickets') || '[]');
        const randomCode = 'CC-2026-' + Math.floor(1000 + Math.random() * 9000);
        const now = new Date().toISOString().replace('T', ' ').substring(0, 19);
        const newTicket = {
            id: Date.now(),
            ticketCode: randomCode,
            studentId: ticketData.studentId,
            studentName: ticketData.name || ticketData.studentName,
            email: ticketData.email,
            department: ticketData.department || 'General',
            queryType: ticketData.queryType,
            priority: ticketData.priority || 'Medium',
            subject: ticketData.subject,
            message: ticketData.message,
            status: 'Open',
            staffResponse: '',
            createdAt: now,
            updatedAt: now
        };
        list.push(newTicket);
        localStorage.setItem('cc_tickets', JSON.stringify(list));
        return { success: true, ticketCode: randomCode, data: newTicket };
    }

    async updateTicket(ticketCode, status, staffResponse) {
        if (this.backendOnline) {
            try {
                const res = await fetch(`${API_BASE}/api/tickets`, {
                    method: 'PUT',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ ticketCode, status, staffResponse })
                });
                const json = await res.json();
                if (res.ok) return json;
            } catch (e) {}
        }
        let list = JSON.parse(localStorage.getItem('cc_tickets') || '[]');
        const idx = list.findIndex(t => t.ticketCode === ticketCode);
        if (idx !== -1) {
            if (status) list[idx].status = status;
            if (staffResponse !== undefined) list[idx].staffResponse = staffResponse;
            list[idx].updatedAt = new Date().toISOString().replace('T', ' ').substring(0, 19);
            localStorage.setItem('cc_tickets', JSON.stringify(list));
            return { success: true, data: list[idx] };
        }
        throw new Error("Ticket not found.");
    }

    async trackTicket(query) {
        const q = query.trim().toUpperCase();
        if (this.backendOnline) {
            try {
                const url = `${API_BASE}/api/tickets/track?code=${encodeURIComponent(q)}`;
                const res = await fetch(url);
                const json = await res.json();
                if (res.ok && json.success) return json.data;
            } catch (e) {}
        }
        const list = JSON.parse(localStorage.getItem('cc_tickets') || '[]');
        const found = list.find(t => t.ticketCode.toUpperCase() === q);
        if (found) return found;

        // Try matching by student ID
        const studentTickets = list.filter(t => t.studentId.toUpperCase() === q);
        if (studentTickets.length > 0) return studentTickets;

        return null;
    }
}

const api = new CampusConnectAPI();

// Toast Notifications Helper
function showToast(message, type = 'info') {
    let container = document.getElementById('toast-container');
    if (!container) {
        container = document.createElement('div');
        container.id = 'toast-container';
        document.body.appendChild(container);
    }

    const toast = document.createElement('div');
    toast.className = `toast ${type}`;
    toast.innerHTML = `
        <span>${message}</span>
        <button style="background:none;border:none;cursor:pointer;color:#94a3b8;font-size:16px;margin-left:10px;">&times;</button>
    `;

    toast.querySelector('button').onclick = () => toast.remove();
    container.appendChild(toast);

    setTimeout(() => {
        toast.style.opacity = '0';
        toast.style.transform = 'translateX(100%)';
        toast.style.transition = 'all 0.3s ease';
        setTimeout(() => toast.remove(), 300);
    }, 4000);
}

// Copy to Clipboard Helper
function copyToClipboard(text) {
    navigator.clipboard.writeText(text).then(() => {
        showToast(`Copied to clipboard: ${text}`, 'success');
    }).catch(() => {
        showToast(`Code: ${text}`, 'info');
    });
}

// ============================================================
// PAGE INITIALIZATIONS
// ============================================================

// Quick Track Handler on Home Page
function quickTrackTicket() {
    const input = document.getElementById("quickTrackInput");
    if (!input || !input.value.trim()) {
        showToast("Please enter a Ticket Reference Code.", "warning");
        return;
    }
    const val = input.value.trim();
    sessionStorage.setItem('prefill_student', val);
    window.location.href = "contact.html#track";
}

document.addEventListener("DOMContentLoaded", async () => {
    const quickInput = document.getElementById("quickTrackInput");
    if (quickInput) {
        quickInput.addEventListener("keypress", (e) => {
            if (e.key === "Enter") quickTrackTicket();
        });
    }
    // 1. Initialize System Stats if elements exist (e.g. index.html or admin.html)
    updateStatsElements();

    // 2. Contact / Helpdesk Page handlers
    initHelpdeskPage();

    // 3. Student Management Page handlers
    initStudentsPage();

    // 4. Admin Helpdesk Dashboard handlers
    initAdminPage();

    // 5. FAQ Accordion handler
    initFaqAccordion();
});

// Update Header / Card Stats
async function updateStatsElements() {
    try {
        const stats = await api.getStats();
        const totalStudentsEl = document.getElementById('stat-total-students');
        const totalTicketsEl = document.getElementById('stat-total-tickets');
        const openTicketsEl = document.getElementById('stat-open-tickets');
        const progressTicketsEl = document.getElementById('stat-progress-tickets');
        const resolvedTicketsEl = document.getElementById('stat-resolved-tickets');

        if (totalStudentsEl) totalStudentsEl.textContent = stats.totalStudents;
        if (totalTicketsEl) totalTicketsEl.textContent = stats.totalTickets;
        if (openTicketsEl) openTicketsEl.textContent = stats.openTickets;
        if (progressTicketsEl) progressTicketsEl.textContent = stats.inProgressTickets;
        if (resolvedTicketsEl) resolvedTicketsEl.textContent = stats.resolvedTickets;
    } catch (e) {
        console.error("Could not update stats", e);
    }
}

// ============================================================
// HELPDESK & CONTACT PAGE
// ============================================================

function initHelpdeskPage() {
    const contactForm = document.getElementById("contactForm");
    const trackBtn = document.getElementById("trackTicketBtn");
    const trackInput = document.getElementById("trackInput");

    // Tab Switching
    const tabBtns = document.querySelectorAll(".tab-btn");
    tabBtns.forEach(btn => {
        btn.addEventListener("click", () => {
            tabBtns.forEach(b => b.classList.remove("active"));
            document.querySelectorAll(".tab-content").forEach(c => c.classList.remove("active"));
            btn.classList.add("active");
            const target = document.getElementById(btn.dataset.target);
            if (target) target.classList.add("active");
        });
    });

    // Auto-select tab if URL has hash or prefill
    if (window.location.hash === "#track" || sessionStorage.getItem('prefill_student')) {
        const trackTabBtn = document.querySelector('.tab-btn[data-target="tab-track"]');
        if (trackTabBtn) trackTabBtn.click();
        const prefill = sessionStorage.getItem('prefill_student');
        if (prefill) {
            sessionStorage.removeItem('prefill_student');
            if (trackInput) {
                trackInput.value = prefill;
                handleTicketLookup(prefill);
            }
        }
    }

    // Form Submission
    if (contactForm) {
        contactForm.addEventListener("submit", async (e) => {
            e.preventDefault();

            const name = document.getElementById("name").value.trim();
            const email = document.getElementById("email").value.trim();
            const studentId = document.getElementById("studentId").value.trim();
            const department = document.getElementById("department") ? document.getElementById("department").value : "";
            const queryType = document.getElementById("queryType").value;
            const priority = document.getElementById("priority") ? document.getElementById("priority").value : "Medium";
            const subject = document.getElementById("subject").value.trim();
            const message = document.getElementById("message").value.trim();

            const responseEl = document.getElementById("response");

            if (!name || !email || !studentId || !queryType || !subject || !message) {
                if (responseEl) {
                    responseEl.textContent = "Please fill in all required fields.";
                    responseEl.style.color = "red";
                }
                showToast("Please fill in all required fields.", "error");
                return;
            }

            const emailPattern = /^[^\s@]+@[^\s@]+\.[^\s@]+$/;
            if (!emailPattern.test(email)) {
                if (responseEl) {
                    responseEl.textContent = "Please enter a valid email address.";
                    responseEl.style.color = "red";
                }
                showToast("Please enter a valid email address.", "error");
                return;
            }

            const submitBtn = contactForm.querySelector('button[type="submit"]');
            if (submitBtn) {
                submitBtn.disabled = true;
                submitBtn.textContent = "Submitting Query...";
            }

            try {
                const res = await api.addTicket({
                    name,
                    email,
                    studentId,
                    department,
                    queryType,
                    priority,
                    subject,
                    message
                });

                const ticketCode = res.ticketCode || (res.data ? res.data.ticketCode : "CC-2026-TICKET");

                if (responseEl) {
                    responseEl.innerHTML = `
                        <div style="background:#ecfdf5;border:1.5px solid #10b981;padding:16px;border-radius:8px;margin-top:16px;">
                            <h4 style="color:#065f46;margin-bottom:6px;">Query Submitted Successfully!</h4>
                            <p style="color:#047857;font-size:14px;margin-bottom:10px;">
                                Your Ticket Reference Code is: <strong>${ticketCode}</strong>
                            </p>
                            <button type="button" class="btn btn-sm btn-secondary" onclick="copyToClipboard('${ticketCode}')">
                                Copy Ticket Code
                            </button>
                        </div>
                    `;
                }

                showToast(`Ticket ${ticketCode} created successfully!`, "success");
                contactForm.reset();
                updateStatsElements();
            } catch (err) {
                showToast(err.message || "Failed to submit query.", "error");
                if (responseEl) {
                    responseEl.textContent = "Error: " + err.message;
                    responseEl.style.color = "red";
                }
            } finally {
                if (submitBtn) {
                    submitBtn.disabled = false;
                    submitBtn.textContent = "Submit Query";
                }
            }
        });
    }

    // Ticket Tracking
    if (trackBtn && trackInput) {
        trackBtn.addEventListener("click", () => handleTicketLookup(trackInput.value));
        trackInput.addEventListener("keypress", (e) => {
            if (e.key === "Enter") handleTicketLookup(trackInput.value);
        });
    }
}

async function handleTicketLookup(query) {
    const container = document.getElementById("trackResultContainer");
    if (!container) return;

    if (!query || !query.trim()) {
        showToast("Please enter a Ticket ID or Student ID.", "warning");
        return;
    }

    container.innerHTML = `<p style="text-align:center;color:#64748b;padding:20px;">Searching ticket records...</p>`;

    try {
        const result = await api.trackTicket(query);
        if (!result) {
            container.innerHTML = `
                <div class="ticket-tracker-card" style="text-align:center;padding:40px 20px;">
                    <div style="font-size:40px;margin-bottom:10px;">🔍</div>
                    <h4 style="color:#1e293b;">No Ticket Found</h4>
                    <p style="color:#64748b;font-size:14px;margin-top:6px;">
                        No inquiry matches "<strong>${query}</strong>". Please verify your Ticket Reference Code or Student ID.
                    </p>
                </div>
            `;
            return;
        }

        if (Array.isArray(result)) {
            // Multiple tickets for student ID
            container.innerHTML = `
                <div style="margin-top:20px;">
                    <h3 style="color:#173b67;margin-bottom:15px;">Inquiry Records for Student (${result.length})</h3>
                    ${result.map(renderTicketCardHTML).join('')}
                </div>
            `;
        } else {
            // Single ticket
            container.innerHTML = renderTicketCardHTML(result);
        }
    } catch (err) {
        container.innerHTML = `<p style="color:red;text-align:center;">Error: ${err.message}</p>`;
    }
}

function renderTicketCardHTML(t) {
    let statusClass = 'badge-open';
    if (t.status === 'In Progress') statusClass = 'badge-progress';
    if (t.status === 'Resolved') statusClass = 'badge-resolved';
    if (t.status === 'Closed') statusClass = 'badge-closed';

    const responseSection = t.staffResponse ? `
        <div class="ticket-response-box">
            <h5 style="color:#065f46;font-size:14px;font-weight:700;margin-bottom:6px;">Staff Resolution / Response:</h5>
            <p style="color:#1e293b;font-size:14.5px;">${t.staffResponse}</p>
        </div>
    ` : `
        <div class="ticket-response-box pending">
            <h5 style="color:#92400e;font-size:14px;font-weight:700;margin-bottom:6px;">Pending Review</h5>
            <p style="color:#78350f;font-size:14px;">An assigned helpdesk officer is currently investigating this query.</p>
        </div>
    `;

    return `
        <div class="ticket-tracker-card">
            <div class="ticket-header-meta">
                <div>
                    <span class="badge ${statusClass}">${t.status}</span>
                    <span class="badge badge-priority-${(t.priority || 'medium').toLowerCase()}" style="margin-left:8px;">${t.priority || 'Normal'} Priority</span>
                    <h3 class="ticket-title" style="margin-top:10px;">${t.subject}</h3>
                </div>
                <div style="text-align:right;">
                    <strong style="color:#173b67;font-size:17px;">${t.ticketCode}</strong>
                    <p style="font-size:12.5px;color:#64748b;margin-top:4px;">Submitted: ${t.createdAt || 'Recent'}</p>
                </div>
            </div>

            <div class="ticket-timeline">
                <div class="timeline-item">
                    <p class="label">Student Name</p>
                    <p class="value">${t.studentName || 'Student'}</p>
                </div>
                <div class="timeline-item">
                    <p class="label">Student ID</p>
                    <p class="value">${t.studentId}</p>
                </div>
                <div class="timeline-item">
                    <p class="label">Department</p>
                    <p class="value">${t.department || 'General'}</p>
                </div>
                <div class="timeline-item">
                    <p class="label">Query Category</p>
                    <p class="value">${t.queryType}</p>
                </div>
            </div>

            <div style="margin-top:16px;">
                <p class="label" style="font-size:12px;color:#64748b;text-transform:uppercase;font-weight:600;">Student's Message</p>
                <p style="color:#334155;background:#f8fafc;padding:14px;border-radius:6px;border:1px solid #e2e8f0;margin-top:6px;font-size:14.5px;">
                    ${t.message}
                </p>
            </div>

            ${responseSection}
        </div>
    `;
}

// ============================================================
// STUDENT MANAGEMENT PAGE
// ============================================================

let cachedStudents = [];

function initStudentsPage() {
    const tableBody = document.getElementById("studentsTableBody");
    if (!tableBody) return;

    const searchInput = document.getElementById("studentSearchInput");
    const deptFilter = document.getElementById("deptFilterSelect");
    const addStudentForm = document.getElementById("addStudentForm");

    // Load initial table data
    loadStudentsTable();

    // Search and filter listeners
    if (searchInput) {
        searchInput.addEventListener("input", debounce(() => {
            loadStudentsTable(searchInput.value, deptFilter ? deptFilter.value : '');
        }, 300));
    }

    if (deptFilter) {
        deptFilter.addEventListener("change", () => {
            loadStudentsTable(searchInput ? searchInput.value : '', deptFilter.value);
        });
    }

    // Add Student Form Handler
    if (addStudentForm) {
        addStudentForm.addEventListener("submit", async (e) => {
            e.preventDefault();
            const studentId = document.getElementById("addStudentId").value.trim();
            const fullName = document.getElementById("addFullName").value.trim();
            const email = document.getElementById("addEmail").value.trim();
            const phone = document.getElementById("addPhone").value.trim();
            const department = document.getElementById("addDepartment").value;
            const semester = document.getElementById("addSemester").value;
            const cgpa = document.getElementById("addCgpa").value;

            try {
                await api.addStudent({ studentId, fullName, email, phone, department, semester, cgpa });
                showToast(`Student ${fullName} registered successfully!`, "success");
                closeModal("addStudentModal");
                addStudentForm.reset();
                loadStudentsTable();
                updateStatsElements();
            } catch (err) {
                showToast(err.message || "Failed to add student.", "error");
            }
        });
    }
}

async function loadStudentsTable(search = '', dept = '') {
    const tableBody = document.getElementById("studentsTableBody");
    const countEl = document.getElementById("studentCountBadge");
    if (!tableBody) return;

    tableBody.innerHTML = `<tr><td colspan="7" style="text-align:center;padding:25px;color:#64748b;">Loading students...</td></tr>`;

    try {
        const students = await api.getStudents(search, dept);
        cachedStudents = students;

        if (countEl) countEl.textContent = `${students.length} Students`;

        if (students.length === 0) {
            tableBody.innerHTML = `<tr><td colspan="7" style="text-align:center;padding:30px;color:#64748b;">No student records found matching filter.</td></tr>`;
            return;
        }

        tableBody.innerHTML = students.map(s => `
            <tr>
                <td><strong>${s.studentId}</strong></td>
                <td>
                    <div style="font-weight:600;color:#173b67;">${s.fullName}</div>
                    <div style="font-size:12px;color:#64748b;">${s.email}</div>
                </td>
                <td>${s.department}</td>
                <td>Sem ${s.semester || 1}</td>
                <td><strong>${s.cgpa ? Number(s.cgpa).toFixed(2) : 'N/A'}</strong></td>
                <td>
                    <span class="badge ${s.status === 'Active' ? 'badge-resolved' : 'badge-closed'}">
                        ${s.status || 'Active'}
                    </span>
                </td>
                <td>
                    <div style="display:flex;gap:6px;">
                        <button class="btn btn-sm btn-secondary" onclick="viewStudentDetails('${s.studentId}')">View</button>
                        <button class="btn btn-sm btn-danger" onclick="confirmDeleteStudent(${s.id}, '${s.fullName}')">Delete</button>
                    </div>
                </td>
            </tr>
        `).join('');
    } catch (err) {
        tableBody.innerHTML = `<tr><td colspan="7" style="text-align:center;color:red;padding:20px;">Failed to load data: ${err.message}</td></tr>`;
    }
}

async function confirmDeleteStudent(id, name) {
    if (confirm(`Are you sure you want to delete student "${name}"?`)) {
        try {
            await api.deleteStudent(id);
            showToast(`Student "${name}" deleted.`, "success");
            loadStudentsTable();
            updateStatsElements();
        } catch (e) {
            showToast(e.message || "Failed to delete student.", "error");
        }
    }
}

function viewStudentDetails(studentId) {
    const student = cachedStudents.find(s => s.studentId === studentId);
    if (!student) return;

    const modalBody = document.getElementById("studentDetailsBody");
    if (!modalBody) return;

    modalBody.innerHTML = `
        <div style="text-align:center;margin-bottom:20px;">
            <div style="width:64px;height:64px;background:#dbeafe;color:#2563eb;border-radius:50%;display:flex;align-items:center;justify-content:center;font-size:24px;font-weight:700;margin:0 auto 10px;">
                ${student.fullName.charAt(0)}
            </div>
            <h3 style="color:#173b67;margin:0;">${student.fullName}</h3>
            <span class="badge badge-resolved" style="margin-top:6px;">${student.status || 'Active'}</span>
        </div>

        <div style="display:grid;grid-template-columns:1fr 1fr;gap:12px;font-size:14px;background:#f8fafc;padding:16px;border-radius:8px;border:1px solid #e2e8f0;">
            <div><strong style="color:#64748b;">Student ID:</strong> ${student.studentId}</div>
            <div><strong style="color:#64748b;">Department:</strong> ${student.department}</div>
            <div><strong style="color:#64748b;">Email:</strong> ${student.email}</div>
            <div><strong style="color:#64748b;">Phone:</strong> ${student.phone || 'N/A'}</div>
            <div><strong style="color:#64748b;">Semester:</strong> ${student.semester || 1}</div>
            <div><strong style="color:#64748b;">CGPA:</strong> ${student.cgpa || 'N/A'}</div>
            <div style="grid-column:1/-1;"><strong style="color:#64748b;">Enrolled:</strong> ${student.createdAt || 'N/A'}</div>
        </div>

        <div style="margin-top:20px;text-align:center;">
            <a href="contact.html" class="btn btn-sm btn-secondary" onclick="sessionStorage.setItem('prefill_student', '${student.studentId}')">
                View Helpdesk Inquiries for this Student
            </a>
        </div>
    `;

    openModal("studentDetailsModal");
}

// ============================================================
// ADMIN HELPDESK CONSOLE
// ============================================================

let cachedAdminTickets = [];

function initAdminPage() {
    const tableBody = document.getElementById("adminTicketsTableBody");
    if (!tableBody) return;

    const statusFilter = document.getElementById("adminStatusFilter");
    const typeFilter = document.getElementById("adminTypeFilter");
    const searchInput = document.getElementById("adminTicketSearch");

    loadAdminTickets();

    if (statusFilter) statusFilter.addEventListener("change", () => loadAdminTickets());
    if (typeFilter) typeFilter.addEventListener("change", () => loadAdminTickets());
    if (searchInput) {
        searchInput.addEventListener("input", debounce(() => {
            const query = searchInput.value.toLowerCase();
            const filtered = cachedAdminTickets.filter(t => 
                t.ticketCode.toLowerCase().includes(query) ||
                t.studentName.toLowerCase().includes(query) ||
                t.studentId.toLowerCase().includes(query) ||
                t.subject.toLowerCase().includes(query)
            );
            renderAdminTable(filtered);
        }, 250));
    }

    // Modal Form for Staff Ticket Response
    const respondForm = document.getElementById("adminRespondForm");
    if (respondForm) {
        respondForm.addEventListener("submit", async (e) => {
            e.preventDefault();
            const code = document.getElementById("modalTicketCode").value;
            const newStatus = document.getElementById("modalTicketStatus").value;
            const staffResponse = document.getElementById("modalStaffResponse").value.trim();

            try {
                await api.updateTicket(code, newStatus, staffResponse);
                showToast(`Ticket ${code} updated successfully!`, "success");
                closeModal("respondTicketModal");
                loadAdminTickets();
                updateStatsElements();
            } catch (err) {
                showToast(err.message || "Failed to update ticket", "error");
            }
        });
    }
}

async function loadAdminTickets() {
    const tableBody = document.getElementById("adminTicketsTableBody");
    if (!tableBody) return;

    const status = document.getElementById("adminStatusFilter") ? document.getElementById("adminStatusFilter").value : '';
    const type = document.getElementById("adminTypeFilter") ? document.getElementById("adminTypeFilter").value : '';

    try {
        const tickets = await api.getTickets(status, type);
        cachedAdminTickets = tickets;
        renderAdminTable(tickets);
    } catch (err) {
        tableBody.innerHTML = `<tr><td colspan="7" style="text-align:center;color:red;">Error: ${err.message}</td></tr>`;
    }
}

function renderAdminTable(tickets) {
    const tableBody = document.getElementById("adminTicketsTableBody");
    if (!tableBody) return;

    if (tickets.length === 0) {
        tableBody.innerHTML = `<tr><td colspan="7" style="text-align:center;padding:25px;color:#64748b;">No tickets match current filters.</td></tr>`;
        return;
    }

    tableBody.innerHTML = tickets.map(t => {
        let statusBadge = 'badge-open';
        if (t.status === 'In Progress') statusBadge = 'badge-progress';
        if (t.status === 'Resolved') statusBadge = 'badge-resolved';
        if (t.status === 'Closed') statusBadge = 'badge-closed';

        return `
            <tr>
                <td><strong>${t.ticketCode}</strong></td>
                <td>
                    <div style="font-weight:600;color:#173b67;">${t.studentName}</div>
                    <div style="font-size:12px;color:#64748b;">${t.studentId} • ${t.department || 'N/A'}</div>
                </td>
                <td>${t.queryType}</td>
                <td><span class="badge badge-priority-${(t.priority || 'medium').toLowerCase()}">${t.priority || 'Medium'}</span></td>
                <td><span class="badge ${statusBadge}">${t.status}</span></td>
                <td style="max-width:250px;white-space:nowrap;overflow:hidden;text-overflow:ellipsis;">${t.subject}</td>
                <td>
                    <button class="btn btn-sm btn-accent" onclick="openRespondModal('${t.ticketCode}')">
                        Respond
                    </button>
                </td>
            </tr>
        `;
    }).join('');
}

function openRespondModal(ticketCode) {
    const ticket = cachedAdminTickets.find(t => t.ticketCode === ticketCode);
    if (!ticket) return;

    document.getElementById("modalTicketCode").value = ticket.ticketCode;
    document.getElementById("modalDisplayCode").textContent = ticket.ticketCode;
    document.getElementById("modalStudentInfo").textContent = `${ticket.studentName} (${ticket.studentId})`;
    document.getElementById("modalSubject").textContent = ticket.subject;
    document.getElementById("modalMessage").textContent = ticket.message;
    document.getElementById("modalTicketStatus").value = ticket.status;
    document.getElementById("modalStaffResponse").value = ticket.staffResponse || "";

    openModal("respondTicketModal");
}

// Export Tickets to CSV
function exportTicketsCSV() {
    if (!cachedAdminTickets || cachedAdminTickets.length === 0) {
        showToast("No ticket data to export.", "warning");
        return;
    }

    const headers = ["Ticket Code", "Student ID", "Student Name", "Email", "Department", "Query Type", "Priority", "Status", "Subject", "Created At"];
    const rows = cachedAdminTickets.map(t => [
        t.ticketCode,
        t.studentId,
        `"${(t.studentName || '').replace(/"/g, '""')}"`,
        t.email,
        `"${(t.department || '').replace(/"/g, '""')}"`,
        t.queryType,
        t.priority,
        t.status,
        `"${(t.subject || '').replace(/"/g, '""')}"`,
        t.createdAt
    ]);

    const csvContent = "data:text/csv;charset=utf-8," + [headers.join(','), ...rows.map(r => r.join(','))].join('\n');
    const encodedUri = encodeURI(csvContent);
    const link = document.createElement("a");
    link.setAttribute("href", encodedUri);
    link.setAttribute("download", `CampusConnect_Tickets_${new Date().toISOString().slice(0,10)}.csv`);
    document.body.appendChild(link);
    link.click();
    document.body.removeChild(link);
    showToast("Tickets exported to CSV!", "success");
}

// ============================================================
// MODAL CONTROLLERS & UTILITIES
// ============================================================

function openModal(id) {
    const modal = document.getElementById(id);
    if (modal) modal.classList.add("active");
}

function closeModal(id) {
    const modal = document.getElementById(id);
    if (modal) modal.classList.remove("active");
}

window.addEventListener("click", (e) => {
    if (e.target.classList.contains("modal-backdrop")) {
        e.target.classList.remove("active");
    }
});

function debounce(func, wait) {
    let timeout;
    return function executedFunction(...args) {
        const later = () => {
            clearTimeout(timeout);
            func(...args);
        };
        clearTimeout(timeout);
        timeout = setTimeout(later, wait);
    };
}

function initFaqAccordion() {
    const faqQuestions = document.querySelectorAll(".faq-question");
    faqQuestions.forEach(q => {
        q.addEventListener("click", () => {
            const item = q.parentElement;
            item.classList.toggle("active");
        });
    });
}
