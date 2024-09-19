#include <iostream>
#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <psapi.h>
#include <fstream>
#include <sstream>
#include <ctime>

// Function prototypes
DWORD findProcessId(const std::wstring& processName);
void displayProcessInfo(DWORD processId);
void optimizeProcess(DWORD processId);
void logSystemInfo(const std::wstring& filename, DWORD processId);

void boostProcess() {
    // اسم التطبيق الذي تريد تسريعه
    std::wstring processName = L"RobloxPlayerBeta.exe";

    // العثور على PID للتطبيق
    DWORD processId = findProcessId(processName);
    if (processId == 0) {
        std::wcerr << L"Failed to find process ID for " << processName << std::endl;
        return;
    }

    // عرض تفاصيل العملية
    displayProcessInfo(processId);

    // تحسين العملية
    optimizeProcess(processId);

    // سجل معلومات النظام
    logSystemInfo(L"BoostInfoLog.txt", processId);
}

// Function to find the process ID by process name
DWORD findProcessId(const std::wstring& processName) {
    DWORD processId = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE) {
        std::wcerr << L"Failed to take snapshot of processes." << std::endl;
        return 0;
    }

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnap, &pe)) {
        do {
            if (processName == pe.szExeFile) {
                processId = pe.th32ProcessID;
                break;
            }
        } while (Process32Next(hSnap, &pe));
    }

    CloseHandle(hSnap);
    return processId;
}

// Function to display information about the process
void displayProcessInfo(DWORD processId) {
    std::wcout << L"Displaying information for process ID: " << processId << std::endl;

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    if (hProcess) {
        TCHAR processName[MAX_PATH];
        if (GetModuleFileNameEx(hProcess, NULL, processName, MAX_PATH)) {
            std::wcout << L"Process Name: " << processName << std::endl;
        }
        else {
            std::wcerr << L"Failed to get process name." << std::endl;
        }
        CloseHandle(hProcess);
    }
    else {
        std::wcerr << L"Failed to open process." << std::endl;
    }
}

// Function to optimize the process
void optimizeProcess(DWORD processId) {
    std::wcout << L"Optimizing process ID: " << processId << std::endl;

    HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION | PROCESS_QUERY_INFORMATION, FALSE, processId);
    if (hProcess == NULL) {
        std::wcerr << L"Failed to open process." << std::endl;
        return;
    }

    // تغيير أولوية العملية إلى أعلى أولوية
    if (SetPriorityClass(hProcess, HIGH_PRIORITY_CLASS)) {
        std::wcout << L"Process priority set to high." << std::endl;
    }
    else {
        std::wcerr << L"Failed to set process priority." << std::endl;
    }

    CloseHandle(hProcess);
}

// Function to log system information to a file
void logSystemInfo(const std::wstring& filename, DWORD processId) {
    SYSTEM_INFO sysInfo;
    MEMORYSTATUSEX memInfo;
    FILETIME ftCreation, ftExit, ftKernel, ftUser;
    SYSTEMTIME stKernel, stUser;
    std::wostringstream logStream;

    // Get system information
    GetSystemInfo(&sysInfo);
    memInfo.dwLength = sizeof(memInfo);
    GlobalMemoryStatusEx(&memInfo);

    // Get process creation time
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processId);
    if (hProcess) {
        FILETIME ftCreation, ftExit, ftKernel, ftUser;
        GetProcessTimes(hProcess, &ftCreation, &ftExit, &ftKernel, &ftUser);

        SYSTEMTIME stKernel, stUser;
        FileTimeToSystemTime(&ftKernel, &stKernel);
        FileTimeToSystemTime(&ftUser, &stUser);

        logStream << L"Process ID: " << processId << L"\n";
        logStream << L"Process Start Time: " << stKernel.wYear << L"/" << stKernel.wMonth << L"/" << stKernel.wDay << L" "
            << stKernel.wHour << L":" << stKernel.wMinute << L":" << stKernel.wSecond << L"\n";

        CloseHandle(hProcess);
    }
    else {
        logStream << L"Failed to open process to get creation time.\n";
    }

    // Log system information
    logStream << L"Processor Architecture: " << sysInfo.wProcessorArchitecture << L"\n";
    logStream << L"Number of Processors: " << sysInfo.dwNumberOfProcessors << L"\n";
    logStream << L"Page Size: " << sysInfo.dwPageSize << L"\n";
    logStream << L"Total Physical Memory: " << memInfo.ullTotalPhys / (1024 * 1024) << L" MB\n";
    logStream << L"Available Physical Memory: " << memInfo.ullAvailPhys / (1024 * 1024) << L" MB\n";
    logStream << L"Total Virtual Memory: " << memInfo.ullTotalVirtual / (1024 * 1024) << L" MB\n";
    logStream << L"Available Virtual Memory: " << memInfo.ullAvailVirtual / (1024 * 1024) << L" MB\n";

    // Write to file
    std::wofstream logFile(filename, std::ios::app);
    if (logFile.is_open()) {
        logFile << logStream.str();
        logFile.close();
    }
    else {
        std::wcerr << L"Failed to open file for logging." << std::endl;
    }
}
