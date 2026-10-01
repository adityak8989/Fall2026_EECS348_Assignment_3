/*
Name of the program = EECS 348 Assignment 3
Description = Priority-queue email inbox simulator using a list-based MaxHeap.
Inputs=  EMAIL sender,subject,date | NEXT | READ | COUNT
Outputs = Printed email details / counts 
Collaborators = None
Other sources = Gemini, ChatGPT, Claude 
Author = Aditya Kulkarni
Creation date = 10/1/2026
Revision date = 10/1/2026
Revision = fixed the crashes and junk values from bad dates, added reading from a .txt file.
*/

#include <iostream> // Includes standard I/O library for console input/output. Authored by gemini
#include <fstream>  // needed to read a .txt file. Authored by Aditya with help of claude.
#include <string> // Includes string library for using the string data type. Authored by gemini
#include <vector> // Includes vector library for dynamic arrays. Authored by gemini
#include <utility>  // for std::swap and std::move (replaces unused <sstream>). Authored by Aditya with help of claude.
 
using namespace std; // Brings the standard namespace into scope so std:: prefix is not needed. Authored by gemini
 
// helper that removes spaces/tabs/\r from both ends (handles " 01-02-2024" and Windows line endings). Authored by Aditya with help of claude.
static string trim(const string& s) { // Defines a static function to remove leading and trailing whitespace. Authored by gemini
    size_t start = s.find_first_not_of(" \t\r\n");          // find first real character. Authored by Aditya with help of claude.
    if (start == string::npos) return "";                   // line was all spaces. Authored by Aditya with help of claude.
    size_t end = s.find_last_not_of(" \t\r\n");             // find last real character. Authored by Aditya with help of claude.
    return s.substr(start, end - start + 1);                // return trimmed text. Authored by Aditya with help of claude.
} // Closes the trim function body. Authored by gemini
 
// Class to parse and compare dates // Comment describing the purpose of the Date class. Authored by gemini
class Date { // Begins the declaration of the Date class. Authored by gemini
private: // Specifies private access modifier for class members. Authored by gemini
    int month = 0, day = 0, year = 0;   // always start at 0, so nothing is ever junk/uninitialized. Authored by Aditya with help of claude.
 
public: // Specifies public access modifier for class members. Authored by gemini
    Date() {}                            // fields already set to 0 above. Authored by Aditya with help of claude.
    Date(const string& dateStr) {        // const reference, no string copy. Authored by Aditya with help of claude.
        if (dateStr.length() >= 10) { // Checks if the provided date string is long enough to hold mm-dd-yyyy. Authored by gemini
            try {                        // bad numbers no longer crash the program. Authored by Aditya with help of claude.
                month = stoi(dateStr.substr(0, 2)); // Parses the first two characters as the month integer. Authored by gemini
                day = stoi(dateStr.substr(3, 2)); // Parses the middle two characters as the day integer. Authored by gemini
                year = stoi(dateStr.substr(6, 4)); // Parses the final four characters as the year integer. Authored by gemini
            } catch (...) {              // if the date is bad, fall back to 0-0-0. Authored by Aditya with help of claude.
                month = day = year = 0; // Assigns 0 to all fields as a safe fallback on parsing failure. Authored by gemini
            } // Closes the catch block. Authored by gemini
        } // Closes the if statement block. Authored by gemini
    } // Closes the Date constructor. Authored by gemini
 
    bool operator<(const Date& other) const { // Overloads the less-than operator to compare two dates. Authored by gemini
        if (year != other.year) return year < other.year; // Compares the years first, returning true if this year is older. Authored by gemini
        if (month != other.month) return month < other.month; // Compares the months next if years match. Authored by gemini
        return day < other.day; // Compares the days if both years and months match. Authored by gemini
    } // Closes the operator< block. Authored by gemini
}; // Closes the Date class definition. Authored by gemini
 
// Class representing an individual Email // Comment describing the purpose of the Email class. Authored by gemini
class Email { // Begins the declaration of the Email class. Authored by gemini
private: // Specifies private access modifier for class members. Authored by gemini
    string sender; // Declares a string to store the sender's name. Authored by gemini
    string subject; // Declares a string to store the email subject. Authored by gemini
    string dateStr; // Declares a string to store the raw date text. Authored by gemini
    Date date; // Declares a Date object to store the parsed date for comparisons. Authored by gemini
    int priorityScore; // Declares an integer to store the calculated priority rank. Authored by gemini
    long long order;                     // arrival number, used to break exact ties. Authored by Aditya with help of claude.
 
    void determinePriority() { // Defines a private method to calculate priority score based on sender role. Authored by gemini
        if (sender == "Boss") priorityScore = 5; // Sets top priority of 5 if the sender is the boss. Authored by gemini
        else if (sender == "Subordinate") priorityScore = 4; // Sets priority of 4 if the sender is a subordinate. Authored by gemini
        else if (sender == "Peer") priorityScore = 3; // Sets priority of 3 if the sender is a peer. Authored by gemini
        else if (sender == "ImportantPerson") priorityScore = 2; // Sets priority of 2 if the sender is an ImportantPerson. Authored by gemini
        else if (sender == "OtherPerson") priorityScore = 1; // Sets priority of 1 if the sender is OtherPerson. Authored by gemini
        else priorityScore = 0; // Defaults priority to 0 if the sender is unrecognized. Authored by gemini
    } // Closes the determinePriority block. Authored by gemini
 
public: // Specifies public access modifier for class members. Authored by gemini
    // takes const references and moves them (fewer copies). Authored by Aditya with help of claude.
    Email(const string& s, const string& sub, const string& d, long long ord) // Defines parameterized constructor for Email. Authored by gemini
        : sender(s), subject(sub), dateStr(d), date(d), order(ord) {   // added order. Authored by Aditya with help of claude.
        determinePriority(); // Evaluates and assigns the appropriate priority score upon instantiation. Authored by gemini
    } // Closes the Email constructor body. Authored by gemini
 
    // Compares priority first. If tied, newer date is read first. // Comment explaining the logic for comparing two emails. Authored by gemini
    bool operator<(const Email& other) const { // Overloads less-than operator to determine email ordering in the heap. Authored by gemini
        if (priorityScore != other.priorityScore) { // Checks if emails have different priority levels. Authored by gemini
            return priorityScore < other.priorityScore; // Returns true if this email has lower priority than the other. Authored by gemini
        } // Closes the initial priority check block. Authored by gemini
        if (date < other.date) return true;          // split compare so we can test both directions. Authored by Aditya with help of claude.
        if (other.date < date) return false;         // other email is newer, so this one is not "less". Authored by Aditya with help of claude.
        return order > other.order;                  // same sender type and date: earlier arrival goes first. Authored by Aditya with help of claude.
    } // Closes the operator< block for Email. Authored by gemini
 
    void print() const { // Defines a public constant method to display email data. Authored by gemini
        cout << "    Sender: " << sender << "\n" // Prints the formatted sender string to the console. Authored by gemini
             << "    Subject: " << subject << "\n" // Prints the formatted subject string to the console. Authored by gemini
             << "    Date: " << dateStr << "\n\n"; // Prints the formatted date string and spacing to the console. Authored by gemini
    } // Closes the print block. Authored by gemini
}; // Closes the Email class definition. Authored by gemini
 
// List-based MaxHeap implementation from scratch // Comment detailing what structure the MaxHeap class implements. Authored by gemini
class MaxHeap { // Begins the declaration of the MaxHeap class. Authored by gemini
private: // Specifies private access modifier for class members. Authored by gemini
    vector<Email> heap; // Declares a vector to act as the internal storage for the heap. Authored by gemini
 
    size_t parent(size_t i) { return (i - 1) / 2; }          // size_t avoids signed/unsigned mixing. Authored by Aditya with help of claude.
    size_t leftChild(size_t i) { return (2 * i) + 1; }       // size_t. Authored by Aditya with help of claude.
    size_t rightChild(size_t i) { return (2 * i) + 2; }      // size_t. Authored by Aditya with help of claude.
 
    void heapifyUp(size_t i) { // Defines a method to bubble up an item to maintain the max-heap property. Authored by gemini
        while (i > 0 && heap[parent(i)] < heap[i]) { // Loops while the node is not root and is greater than its parent. Authored by gemini
            swap(heap[parent(i)], heap[i]); // Swaps the current node with its parent to fix the violation. Authored by gemini
            i = parent(i); // Updates the current index to the parent's index to continue the check. Authored by gemini
        } // Closes the while loop. Authored by gemini
    } // Closes the heapifyUp block. Authored by gemini
 
    // loop instead of recursion, so no extra stack memory is used. Authored by Aditya with help of claude.
    void heapifyDown(size_t i) { // Defines a method to bubble down an item to maintain the max-heap property. Authored by gemini
        while (true) {                                       // iterative version. Authored by Aditya with help of claude.
            size_t maxIndex = i; // Initializes maxIndex to the current node index. Authored by gemini
            size_t left = leftChild(i); // Calculates and stores the left child's index. Authored by gemini
            size_t right = rightChild(i); // Calculates and stores the right child's index. Authored by gemini
 
            if (left < heap.size() && heap[maxIndex] < heap[left]) { // Checks if left child exists and is strictly greater than maxIndex node. Authored by gemini
                maxIndex = left; // Sets the left child as the new maxIndex. Authored by gemini
            } // Closes the left child if block. Authored by gemini
            if (right < heap.size() && heap[maxIndex] < heap[right]) { // Checks if right child exists and is strictly greater than maxIndex node. Authored by gemini
                maxIndex = right; // Sets the right child as the new maxIndex. Authored by gemini
            } // Closes the right child if block. Authored by gemini
            if (i == maxIndex) break;                        // stop when heap order is fine. Authored by Aditya with help of claude.
            swap(heap[i], heap[maxIndex]); // Swaps the current node with the larger of its two children. Authored by gemini
            i = maxIndex;                                    // move down and repeat. Authored by Aditya with help of claude.
        } // Closes the while block. Authored by gemini
    } // Closes the heapifyDown block. Authored by gemini
 
public: // Specifies public access modifier for class members. Authored by gemini
    void insert(const Email& email) { // Defines a public method to insert a new email into the priority queue. Authored by gemini
        heap.push_back(email); // Appends the new email to the end of the heap vector. Authored by gemini
        heapifyUp(heap.size() - 1); // Restores heap order by percolating the newly inserted element up. Authored by gemini
    } // Closes the insert block. Authored by gemini
 
    void extractMax() { // Defines a method to remove the highest priority email from the heap. Authored by gemini
        if (heap.empty()) return; // Exits early if there are no items in the heap. Authored by gemini
        heap[0] = move(heap.back());                         // move instead of copy. Authored by Aditya with help of claude.
        heap.pop_back(); // Shrinks the vector by removing the duplicated last element. Authored by gemini
        if (!heap.empty()) { // Checks if there are still elements remaining in the heap. Authored by gemini
            heapifyDown(0); // Restores heap order by percolating the root element down. Authored by gemini
        } // Closes the if block. Authored by gemini
    } // Closes the extractMax block. Authored by gemini
 
    const Email* getMax() const { // Defines a method to view the highest priority email without removing it. Authored by gemini
        if (heap.empty()) return nullptr; // Returns null pointer if the heap is empty to avoid out of bounds errors. Authored by gemini
        return &heap[0]; // Returns a pointer to the first element (max item) in the vector. Authored by gemini
    } // Closes the getMax block. Authored by gemini
 
    int getSize() const { // Defines a public method returning the current number of emails in the queue. Authored by gemini
        return (int)heap.size();                             // explicit cast, no warning. Authored by Aditya with help of claude.
    } // Closes the getSize block. Authored by gemini
 
    bool isEmpty() const { // Defines a public method returning true if the queue has zero emails. Authored by gemini
        return heap.empty(); // Returns the boolean result of vector's empty function. Authored by gemini
    } // Closes the isEmpty block. Authored by gemini
}; // Closes the MaxHeap class definition. Authored by gemini
 
// Class to manage the CEO's inbox operations // Comment describing the intent of the CEOInbox class. Authored by gemini
class CEOInbox { // Begins the declaration of the CEOInbox class. Authored by gemini
private: // Specifies private access modifier for class members. Authored by gemini
    MaxHeap emailQueue; // Declares the max-heap instance that acts as the prioritized inbox. Authored by gemini
    long long nextOrder = 0;                                 // counts emails as they arrive. Authored by Aditya with help of claude.
 
public: // Specifies public access modifier for class members. Authored by gemini
    void processCommand(const string& rawLine) { // Defines a method to parse and execute inbox instructions. Authored by gemini
        string commandLine = trim(rawLine);                  // cleans \r and extra spaces for every command. Authored by Aditya with help of claude.
 
        if (commandLine.compare(0, 6, "EMAIL ") == 0) {      // safe check, never throws on short lines. Authored by Aditya with help of claude.
            string data = commandLine.substr(6); // Slices out the prefix to get the CSV email string. Authored by gemini
 
            size_t firstComma = data.find(','); // Finds the index position of the first comma character. Authored by gemini
            size_t secondComma = data.find(',', firstComma + 1); // Finds the index position of the second comma character. Authored by gemini
 
            if (firstComma != string::npos && secondComma != string::npos) { // Validates that the input line actually contains two commas. Authored by gemini
                string sender = trim(data.substr(0, firstComma));                                  // trim. Authored by Aditya with help of claude.
                string subject = trim(data.substr(firstComma + 1, secondComma - firstComma - 1));  // trim. Authored by Aditya with help of claude.
                string dateStr = trim(data.substr(secondComma + 1));                               // trim removes leading space. Authored by Aditya with help of claude.
 
                Email newEmail(sender, subject, dateStr, nextOrder++);   // pass arrival number. Authored by Aditya with help of claude.
                emailQueue.insert(newEmail); // Inserts the dynamically created email object into the inbox max-heap. Authored by gemini
            } // Closes the valid input block. Authored by gemini
        } // Closes the EMAIL command block. Authored by gemini
        else if (commandLine == "NEXT") { // Checks if the command instructs the system to view the next email. Authored by gemini
            const Email* nextEmail = emailQueue.getMax(); // Points a pointer to the highest-ranked email in the queue. Authored by gemini
            if (nextEmail != nullptr) { // Checks that the pointer is valid (heap is not empty). Authored by gemini
                cout << "Next email:\n"; // Prints a header text indicating what follows. Authored by gemini
                nextEmail->print(); // Accesses the email pointer and triggers its print function. Authored by gemini
            } // Closes the pointer validation block. Authored by gemini
        } // Closes the NEXT command block. Authored by gemini
        else if (commandLine == "READ") { // Checks if the command instructs the system to mark the top email as read. Authored by gemini
            emailQueue.extractMax(); // Evicts the highest-priority email from the queue entirely. Authored by gemini
        } // Closes the READ command block. Authored by gemini
        else if (commandLine == "COUNT") { // Checks if the command asks for the total pending email count. Authored by gemini
            cout << "There are " << emailQueue.getSize() << " emails to read.\n\n"; // Prints out the exact size of the heap instance to the user. Authored by gemini
        } // Closes the COUNT command block. Authored by gemini
    } // Closes the processCommand block. Authored by gemini
}; // Closes the CEOInbox class definition. Authored by gemini
 
// reads every line from any input stream (file or keyboard). Authored by Aditya with help of claude.
static void run(istream& in) { // Defines a static helper to read from an abstract stream line by line. Authored by gemini
    CEOInbox inbox; // Initializes an instance of the inbox for this run session. Authored by gemini
    string line; // Declares a string buffer to store the current line. Authored by gemini
    while (getline(in, line)) { // Loops repeatedly as long as the stream yields lines to parse. Authored by gemini
        if (trim(line).empty()) continue;                    // skip blank lines (trim also removes \r). Authored by Aditya with help of claude.
        inbox.processCommand(line); // Delegates the valid command line over to the inbox object to process. Authored by gemini
    } // Closes the stream reading loop. Authored by gemini
} // Closes the run block. Authored by gemini
 
// main now opens a .txt file from the same folder. Authored by Aditya with help of claude.
int main(int argc, char* argv[]) { // Main entry point containing argument count and argument vector array. Authored by gemini
    string fileName = (argc > 1) ? argv[1] : "input.txt";    // use a file name given by the user, else input.txt. Authored by Aditya with help of claude.
    ifstream file(fileName);                                 // open the .txt file. Authored by Aditya with help of claude.
    if (file.is_open()) { // Evaluates to true if the file stream is currently associated with an open file. Authored by gemini
        run(file);                                           // read commands from the file. Authored by Aditya with help of claude.
    } else { // Fallback execution path if the file failed to open properly. Authored by gemini
        cerr << "Could not open " << fileName // Starts streaming a fallback error message to the standard error console. Authored by gemini
             << ", reading from keyboard/redirected input instead.\n";   // clear message instead of silent failure. Authored by Aditya with help of claude.
        run(cin);                                            // old behavior still works (./a.out < input.txt). Authored by Aditya with help of claude.
    } // Closes the if/else stream evaluation block. Authored by gemini
    return 0; // Signals successful execution to the host environment by returning a zero code. Authored by gemini
} // Terminates the main function and concludes the program script. Authored by gemini