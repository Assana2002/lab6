#include <windows.h>
#include <iostream>
#include <vector>

bool isPrime(int n) {
    if (n <= 1) return false;
    for (int i = 2; i * i <= n; i++) {
        if (n % i == 0) return false;
    }
    return true;
}

void findPrimesInRange(int start, int end, HANDLE writeHandle) {
    std::vector<int> primes;
    for (int i = start; i <= end; i++) {
        if (isPrime(i)) primes.push_back(i);
    }
    for (int prime : primes) {
        DWORD written;
        WriteFile(writeHandle, &prime, sizeof(prime), &written, nullptr);
    }
    int end_signal = -1;
    DWORD written;
    WriteFile(writeHandle, &end_signal, sizeof(end_signal), &written, nullptr);
}

int main() {
    const int RANGE = 1000;
    const int PROCESS_COUNT = 10;

    HANDLE pipes[PROCESS_COUNT][2];

    for (int i = 0; i < PROCESS_COUNT; i++) {
        SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE };
        if (!CreatePipe(&pipes[i][0], &pipes[i][1], &sa, 0)) {
            std::cerr << "Pipe creation failed\n";
            return 1;
        }

        STARTUPINFO si = { sizeof(STARTUPINFO) };
        PROCESS_INFORMATION pi = { nullptr };

        std::string command = std::to_string(i * RANGE + 1) + " " + std::to_string((i + 1) * RANGE);
        char cmdline[256];
        snprintf(cmdline, sizeof(cmdline), "child.exe %s", command.c_str());

        si.hStdOutput = pipes[i][1];
        si.dwFlags |= STARTF_USESTDHANDLES;

        if (!CreateProcess(nullptr, cmdline, nullptr, nullptr, TRUE, 0, nullptr, nullptr, &si, &pi)) {
            std::cerr << "Process creation failed\n";
            return 1;
        }

        CloseHandle(pipes[i][1]); // Închidem capătul de scriere din procesul părinte
    }

    std::cout << "Numere prime găsite:\n";
    for (int i = 0; i < PROCESS_COUNT; i++) {
        int prime;
        DWORD bytesRead;
        while (ReadFile(pipes[i][0], &prime, sizeof(prime), &bytesRead, nullptr) && bytesRead > 0) {
            if (prime == -1) break; // Sfârșit transmisie
            std::cout << prime << " ";
        }
        CloseHandle(pipes[i][0]);
    }

    std::cout << "\nProgram terminat cu succes.\n";
    return 0;
}
