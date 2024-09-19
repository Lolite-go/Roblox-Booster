#include <iostream>
#include <vector>
#include <memory>
#include <thread>
#include <chrono>
#include <mutex>
#include <windows.h>
#include <tlhelp32.h>
#include <cstdlib> // For rand and srand
#include <ctime>   // For time

// Function prototypes
void boostProcess();
void setPriorityOfOtherProcesses(DWORD targetProcessId);
void setNormalPriorityForAllProcesses();
DWORD findProcessId(const std::wstring& processName);
void showMenuAndHandleSelection();

// Mutex for protecting shared resources
std::mutex mtx;

// Function to set console text color
void setConsoleColor(WORD color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

// Function to display the menu with a flashing message
void showFlashingMessage() {
    // Seed the random number generator
    srand(static_cast<unsigned int>(time(0)));

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    WORD colors[] = { FOREGROUND_RED, FOREGROUND_GREEN, FOREGROUND_BLUE, FOREGROUND_RED | FOREGROUND_GREEN, FOREGROUND_RED | FOREGROUND_BLUE, FOREGROUND_GREEN | FOREGROUND_BLUE, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE };

    std::cout << "Roblox Bosster V1\n\n";

    // Flash the message with random colors
    for (int i = 0; i < 38; ++i) { // Display the message for a short time
        WORD color = colors[rand() % (sizeof(colors) / sizeof(colors[0]))];
        setConsoleColor(color);
        std::cout << "\rmade by Lolite For Roblox V1.0"; // Use \r to overwrite the line
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
    }

    // Reset color to default
    setConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    std::cout << "\n";
}

// Function to simulate a heavy task
DWORD WINAPI processHeavyTask(LPVOID taskId) {
    int id = *static_cast<int*>(taskId);
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Simulate a heavy task

    std::lock_guard<std::mutex> lock(mtx); // Protect shared resource with a mutex
    std::cout << "Task " << id << " completed.\n";
    return 0;
}

// Function to write data to a file asynchronously using Windows API (Unicode version)
void writeToFile(const std::wstring& filename, const std::wstring& data) {
    HANDLE hFile = CreateFileW(
        filename.c_str(),
        GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD written;
        WriteFile(hFile, data.c_str(), static_cast<DWORD>(data.size() * sizeof(wchar_t)), &written, NULL);
        CloseHandle(hFile);
    }
    else {
        std::cerr << "Failed to open file for writing.\n";
    }
}

// Function to manage resources
void manageResources() {
    // Using unique_ptr for automatic memory management
    std::unique_ptr<int> uniquePtr = std::make_unique<int>(100);
    std::cout << "Unique Pointer Value: " << *uniquePtr << std::endl;

    // Using shared_ptr to share ownership of a resource
    std::shared_ptr<int> sharedPtr1 = std::make_shared<int>(200);
    std::shared_ptr<int> sharedPtr2 = sharedPtr1; // shared ownership
    std::cout << "Shared Pointer Value: " << *sharedPtr1 << std::endl;

    // Allocate a large block of memory using VirtualAlloc
    SIZE_T size = 1024 * 1024; // 1 MB
    void* pMemory = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (pMemory) {
        std::cout << "Memory allocated successfully.\n";
        VirtualFree(pMemory, 0, MEM_RELEASE);
    }
    else {
        std::cerr << "Failed to allocate memory.\n";
    }
}

// Function to set priority of all processes except the target one
void setPriorityOfOtherProcesses(DWORD targetProcessId) {
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to take snapshot of processes." << std::endl;
        return;
    }

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnap, &pe)) {
        do {
            if (pe.th32ProcessID != targetProcessId) {
                HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, pe.th32ProcessID);
                if (hProcess) {
                    if (SetPriorityClass(hProcess, IDLE_PRIORITY_CLASS)) {
                        std::wcout << L"Set priority of process " << pe.szExeFile << L" to low.\n";
                    }
                    else {
                        std::wcerr << L"Failed to set priority for process " << pe.szExeFile << L".\n";
                    }
                    CloseHandle(hProcess);
                }
            }
        } while (Process32Next(hSnap, &pe));
    }

    CloseHandle(hSnap);
}

// Function to set priority of all processes to normal
void setNormalPriorityForAllProcesses() {
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to take snapshot of processes." << std::endl;
        return;
    }

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnap, &pe)) {
        do {
            HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, pe.th32ProcessID);
            if (hProcess) {
                if (SetPriorityClass(hProcess, NORMAL_PRIORITY_CLASS)) {
                    std::wcout << L"Set priority of process " << pe.szExeFile << L" to normal.\n";
                }
                else {
                    std::wcerr << L"Failed to set priority for process " << pe.szExeFile << L".\n";
                }
                CloseHandle(hProcess);
            }
        } while (Process32Next(hSnap, &pe));
    }

    CloseHandle(hSnap);
}

// Function to display the menu and handle user selection
void showMenuAndHandleSelection() {
    int choice = 0;
    DWORD robloxProcessId = 0; // Declare variable here

    // Display the flashing message
    showFlashingMessage();

    std::cout << "Select an option:\n";
    std::cout << "1. Boost Roblox\n";
    std::cout << "2. Restart Apps\n";
    std::cout << "Enter your choice (1 or 2): ";
    std::cin >> choice;

    switch (choice) {
    case 1:
        // Get the PID of the Roblox process
        robloxProcessId = findProcessId(L"RobloxPlayerBeta.exe");

        // Set priority of other processes to low
        setPriorityOfOtherProcesses(robloxProcessId);

        // Call boostProcess function from BoostInfo.cpp
        boostProcess();

        // Display notifications
        MessageBox(NULL, L"Roblox Boosting set fps to 120 done", L"Notification", MB_OK | MB_ICONINFORMATION);
        std::this_thread::sleep_for(std::chrono::seconds(1));

        MessageBox(NULL, L"Roblox Boosting allow more page memory done", L"Notification", MB_OK | MB_ICONINFORMATION);
        std::this_thread::sleep_for(std::chrono::seconds(1));

        MessageBox(NULL, L"Roblox Boosting lower other apps memory done", L"Notification", MB_OK | MB_ICONINFORMATION);
        std::this_thread::sleep_for(std::chrono::seconds(1));

        MessageBox(NULL, L"Roblox Is Boosted", L"Notification", MB_OK | MB_ICONINFORMATION);

        // Wait for 6 seconds before closing the application
        Sleep(6000); // 6000 milliseconds = 6 seconds

        break;

    case 2:
        // Reset priority of all processes to normal
        setNormalPriorityForAllProcesses();

        std::cout << "All processes priority reset to normal.\n";
        break;

    default:
        std::cerr << "Invalid choice.\n";
        break;
    }
}

int main() {
    // Display menu and handle user selection
    showMenuAndHandleSelection();

    return 0;
}
