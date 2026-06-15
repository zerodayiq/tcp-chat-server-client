// ==================== مكتبات النظام ====================

// إدخال وإخراج (cout / cin)
#include <iostream>

// تشغيل Threads (لتعدد المستخدمين في السيرفر)
#include <thread>

// تخزين قائمة العملاء
#include <vector>

// التعامل مع النصوص string
#include <string>

// خوارزميات مثل remove
#include <algorithm>

// التعامل مع الذاكرة (memset)
#include <cstring>

// mutex لحماية البيانات من التداخل بين threads
#include <mutex>

// مكتبات الشبكات TCP/IP
#include <sys/socket.h>

// تحويل IP و التعامل مع IPv4
#include <arpa/inet.h>

// close() و عمليات النظام
#include <unistd.h>

// signals مثل SIGPIPE
#include <signal.h>

using namespace std;

// ==================== إعدادات ثابتة ====================

// رقم البورت الذي يعمل عليه السيرفر
constexpr int PORT = 54000;

// حجم البافر الذي نستقبل فيه البيانات
constexpr int BUFFER_SIZE = 4096;


// ==================== بيانات السيرفر ====================

// قائمة كل العملاء المتصلين بالسيرفر (كل client = socket)
vector<int> clients;

// mutex يمنع التداخل بين threads عند تعديل clients
mutex clientsMutex;


// ==================== إرسال رسالة كاملة ====================

// TCP لا يضمن إرسال كل البيانات مرة واحدة
// لذلك نعمل loop حتى نرسل الرسالة كاملة
bool sendAll(int sock, const string &msg)
{
    // عدد البايتات التي تم إرسالها حتى الآن
    size_t total = 0;

    // طول الرسالة الكلي
    size_t length = msg.length();

    // نستمر حتى نرسل كل شيء
    while (total < length)
    {
        // إرسال جزء من الرسالة
        ssize_t sent = send(sock, msg.c_str() + total, length - total, 0);

        // إذا فشل الإرسال
        if (sent == -1)
        {
            return false;
        }

        // تحديث عدد البايتات المرسلة
        total += sent;
    }

    // نجاح الإرسال الكامل
    return true;
}


// ==================== بث الرسالة لكل العملاء ====================

// هذه الدالة ترسل الرسالة لكل المستخدمين ما عدا المرسل
void broadcast(int sender, const string &msg)
{
    // نسخة مؤقتة من قائمة العملاء لتجنب مشاكل التزامن
    vector<int> snapshot;

    // بداية block لحماية الوصول للـ clients
    {
        // قفل mutex لمنع أي thread آخر من التعديل
        lock_guard<mutex> lock(clientsMutex);

        // نسخ قائمة العملاء
        snapshot = clients;
    }

    // المرور على كل العملاء
    for (int client : snapshot)
    {
        // لا ترسل الرسالة للمرسل نفسه
        if (client != sender)
        {
            // إرسال الرسالة
            if (!sendAll(client, msg))
            {
                // إذا فشل الإرسال (client انقطع)

                // إغلاق socket
                close(client);

                // حذف العميل من القائمة
                lock_guard<mutex> lock(clientsMutex);

                clients.erase(
                    remove(clients.begin(), clients.end(), client),
                    clients.end());
            }
        }
    }
}


// ==================== حذف عميل ====================

void removeClient(int clientSocket)
{
    // إغلاق الاتصال مع العميل
    close(clientSocket);

    // حماية الوصول للـ vector
    lock_guard<mutex> lock(clientsMutex);

    // حذف العميل من القائمة
    clients.erase(
        remove(clients.begin(), clients.end(), clientSocket),
        clients.end());

    // طباعة رسالة في السيرفر
    cout << "Client disconnected" << endl;
}


// ==================== التعامل مع عميل واحد ====================

// كل عميل يشتغل داخل thread مستقل
void handleClient(int clientSocket)
{
    // buffer لاستقبال البيانات
    char buffer[BUFFER_SIZE];

    while (true)
    {
        // تصفير البافر قبل الاستخدام
        memset(buffer, 0, BUFFER_SIZE);

        // استقبال بيانات من العميل
        ssize_t bytes = recv(clientSocket, buffer, BUFFER_SIZE, 0);

        // إذا صفر أو خطأ → العميل قطع الاتصال
        if (bytes <= 0)
        {
            removeClient(clientSocket);
            break;
        }

        // تحويل البيانات إلى string
        string msg(buffer, bytes);

        // طباعة الرسالة في السيرفر
        cout << msg << endl;

        // إرسال الرسالة لكل باقي العملاء
        broadcast(clientSocket, msg);
    }
}


// ==================== تشغيل السيرفر ====================

void runServer()
{
    // إنشاء socket (TCP)
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    // إذا فشل الإنشاء
    if (serverSocket == -1)
    {
        cerr << "Socket creation failed!" << endl;
        return;
    }

    // السماح بإعادة استخدام البورت مباشرة
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // إعداد معلومات السيرفر
    sockaddr_in serverAddr;

    // IPv4
    serverAddr.sin_family = AF_INET;

    // تحويل رقم البورت
    serverAddr.sin_port = htons(PORT);

    // استقبال من أي IP
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // ربط السيرفر بالبورت
    if (bind(serverSocket, (sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        cerr << "Bind failed!" << endl;
        close(serverSocket);
        return;
    }

    // بدء الاستماع
    if (listen(serverSocket, SOMAXCONN) == -1)
    {
        cerr << "Listen failed!" << endl;
        close(serverSocket);
        return;
    }

    cout << "Server running on port " << PORT << endl;

    // حلقة لا نهائية لاستقبال العملاء
    while (true)
    {
        sockaddr_in clientAddr;  // معلومات العميل
        socklen_t clientLen = sizeof(clientAddr);

        // قبول اتصال جديد
        int clientSocket = accept(serverSocket,
                                  (sockaddr *)&clientAddr,
                                  &clientLen);

        if (clientSocket == -1)
        {
            cerr << "Accept failed!" << endl;
            continue;
        }

        // إضافة العميل للقائمة
        {
            lock_guard<mutex> lock(clientsMutex);
            clients.push_back(clientSocket);
        }

        // طباعة IP العميل
        cout << "New client connected: "
             << inet_ntoa(clientAddr.sin_addr) << endl;

        // تشغيل thread لكل عميل
        thread(handleClient, clientSocket).detach();
    }
}


// ==================== استقبال الرسائل (Client) ====================

void receiveMessages(int sock)
{
    char buffer[BUFFER_SIZE];

    while (true)
    {
        // استقبال من السيرفر
        ssize_t bytes = recv(sock, buffer, BUFFER_SIZE, 0);

        // إذا انقطع الاتصال
        if (bytes <= 0)
        {
            cout << "\nDisconnected from server!" << endl;
            exit(0);
        }

        // طباعة الرسالة
        cout << string(buffer, bytes) << endl;
    }
}


// ==================== تشغيل العميل ====================

void runClient(const string &ip_str)
{
    // إنشاء socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock == -1)
    {
        cerr << "Socket creation failed!" << endl;
        return;
    }

    sockaddr_in serverAddr;

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);

    // تحويل IP من string إلى network format
    if (inet_pton(AF_INET, ip_str.c_str(), &serverAddr.sin_addr) <= 0)
    {
        cerr << "Invalid IP address!" << endl;
        close(sock);
        return;
    }

    // الاتصال بالسيرفر
    if (connect(sock, (sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        cerr << "Connection failed!" << endl;
        close(sock);
        return;
    }

    cout << "Connected to server!" << endl;

    // اسم المستخدم
    string username;
    cout << "Enter username: ";
    getline(cin, username);

    // تشغيل thread للاستقبال
    thread(receiveMessages, sock).detach();

    string line;

    // إرسال الرسائل
    while (getline(cin, line))
    {
        if (line == "/quit")
            break;

        if (line.empty())
            continue;

        // دمج اسم المستخدم مع الرسالة
        string fullMsg = username + ": " + line;

        // إرسالها للسيرفر
        if (!sendAll(sock, fullMsg))
        {
            cerr << "Send failed!" << endl;
            break;
        }
    }

    close(sock);
}


// ==================== main ====================

int main(int argc, char *argv[])
{
    // منع crash عند انقطاع اتصال مفاجئ
    signal(SIGPIPE, SIG_IGN);

    // التأكد من وجود arguments
    if (argc < 2)
    {
        cout << "Usage:\n"
             << "  ./chat server\n"
             << "  ./chat client <IP>\n";
        return 0;
    }

    string mode = argv[1];

    // تشغيل السيرفر
    if (mode == "server")
    {
        runServer();
    }
    // تشغيل العميل
    else if (mode == "client")
    {
        if (argc < 3)
        {
            cout << "Please specify server IP!" << endl;
            return 0;
        }

        runClient(argv[2]);
    }
    else
    {
        cout << "Unknown mode!" << endl;
    }

    return 0;
}