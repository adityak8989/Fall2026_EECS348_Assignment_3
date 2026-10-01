#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <utility>

using namespace std;

static string trim(const string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

class Date {
private:
    int month = 0, day = 0, year = 0;

public:
    Date() {}
    Date(const string& dateStr) {
        if (dateStr.length() >= 10) {
            try {
                month = stoi(dateStr.substr(0, 2));
                day = stoi(dateStr.substr(3, 2));
                year = stoi(dateStr.substr(6, 4));
            } catch (...) {
                month = day = year = 0;
            }
        }
    }

    bool operator<(const Date& other) const {
        if (year != other.year) return year < other.year;
        if (month != other.month) return month < other.month;
        return day < other.day;
    }
};

class Email {
private:
    string sender;
    string subject;
    string dateStr;
    Date date;
    int priorityScore;
    long long order;

    void determinePriority() {
        if (sender == "Boss") priorityScore = 5;
        else if (sender == "Subordinate") priorityScore = 4;
        else if (sender == "Peer") priorityScore = 3;
        else if (sender == "ImportantPerson") priorityScore = 2;
        else if (sender == "OtherPerson") priorityScore = 1;
        else priorityScore = 0;
    }

public:
    Email(const string& s, const string& sub, const string& d, long long ord)
        : sender(s), subject(sub), dateStr(d), date(d), order(ord) {
        determinePriority();
    }

    bool operator<(const Email& other) const {
        if (priorityScore != other.priorityScore) {
            return priorityScore < other.priorityScore;
        }
        if (date < other.date) return true;
        if (other.date < date) return false;
        return order > other.order;
    }

    void print() const {
        cout << "    Sender: " << sender << "\n"
             << "    Subject: " << subject << "\n"
             << "    Date: " << dateStr << "\n\n";
    }
};

class MaxHeap {
private:
    vector<Email> heap;

    size_t parent(size_t i) { return (i - 1) / 2; }
    size_t leftChild(size_t i) { return (2 * i) + 1; }
    size_t rightChild(size_t i) { return (2 * i) + 2; }

    void heapifyUp(size_t i) {
        while (i > 0 && heap[parent(i)] < heap[i]) {
            swap(heap[parent(i)], heap[i]);
            i = parent(i);
        }
    }

    void heapifyDown(size_t i) {
        while (true) {
            size_t maxIndex = i;
            size_t left = leftChild(i);
            size_t right = rightChild(i);

            if (left < heap.size() && heap[maxIndex] < heap[left]) {
                maxIndex = left;
            }
            if (right < heap.size() && heap[maxIndex] < heap[right]) {
                maxIndex = right;
            }
            if (i == maxIndex) break;
            swap(heap[i], heap[maxIndex]);
            i = maxIndex;
        }
    }

public:
    void insert(const Email& email) {
        heap.push_back(email);
        heapifyUp(heap.size() - 1);
    }

    void extractMax() {
        if (heap.empty()) return;
        heap[0] = move(heap.back());
        heap.pop_back();
        if (!heap.empty()) {
            heapifyDown(0);
        }
    }

    const Email* getMax() const {
        if (heap.empty()) return nullptr;
        return &heap[0];
    }

    int getSize() const {
        return (int)heap.size();
    }

    bool isEmpty() const {
        return heap.empty();
    }
};

class CEOInbox {
private:
    MaxHeap emailQueue;
    long long nextOrder = 0;

public:
    void processCommand(const string& rawLine) {
        string commandLine = trim(rawLine);

        if (commandLine.compare(0, 6, "EMAIL ") == 0) {
            string data = commandLine.substr(6);

            size_t firstComma = data.find(',');
            size_t secondComma = data.find(',', firstComma + 1);

            if (firstComma != string::npos && secondComma != string::npos) {
                string sender = trim(data.substr(0, firstComma));
                string subject = trim(data.substr(firstComma + 1, secondComma - firstComma - 1));
                string dateStr = trim(data.substr(secondComma + 1));

                Email newEmail(sender, subject, dateStr, nextOrder++);
                emailQueue.insert(newEmail);
            }
        }
        else if (commandLine == "NEXT") {
            const Email* nextEmail = emailQueue.getMax();
            if (nextEmail != nullptr) {
                cout << "Next email:\n";
                nextEmail->print();
            }
        }
        else if (commandLine == "READ") {
            emailQueue.extractMax();
        }
        else if (commandLine == "COUNT") {
            cout << "There are " << emailQueue.getSize() << " emails to read.\n\n";
        }
    }
};

static void run(istream& in) {
    CEOInbox inbox;
    string line;
    while (getline(in, line)) {
        if (trim(line).empty()) continue;
        inbox.processCommand(line);
    }
}

int main(int argc, char* argv[]) {
    string fileName = (argc > 1) ? argv[1] : "input.txt";
    ifstream file(fileName);
    if (file.is_open()) {
        run(file);
    } else {
        cerr << "Could not open " << fileName
             << ", reading from keyboard/redirected input instead.\n";
        run(cin);
    }
    return 0;
}