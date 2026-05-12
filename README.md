```markdown
# Malware Analysis Project - NT230.Q22 (Nhóm 10)

## 📝 Giới thiệu chung
Dự án này là một mẫu mã độc thực thi (Proof of Concept - PoC) được thiết kế cho mục đích học tập trong môn **Cơ chế mã độc (NT230)** tại **Đại học Công nghệ Thông tin (UIT)**. Mã nguồn mô phỏng các kỹ thuật nâng cao của malware thực tế, bao gồm: Anti-Analysis (phát hiện debugger và máy ảo), Leo thang đặc quyền (UAC Bypass), Duy trì sự hiện diện (Persistence) và Logic Bomb dựa trên thời gian để chống Sandbox.

**⚠️ CẢNH BÁO:** Đây là mã nguồn giáo dục. Việc biên dịch và chạy có thể gây hại cho hệ thống. Chỉ sử dụng trong môi trường kiểm soát (máy ảo) và với mục đích học tập. Không sử dụng cho mục đích bất hợp pháp.

## 🚀 Tính năng chính
- **Anti-Debugging:** Phát hiện debugger qua Windows API và PEB (Process Environment Block).
- **Anti-VM:** Phát hiện máy ảo qua artifact (registry, file, process) và behavior (timing, hardware specs).
- **Anti-Sandbox:** Logic Bomb dựa trên thời gian uptime hệ thống để tránh môi trường phân tích tự động.
- **UAC Bypass:** Sử dụng fodhelper.exe để leo thang quyền Admin mà không cần xác nhận người dùng.
- **Persistence:** Đăng ký chạy tự động qua Registry, Startup Folder và Windows Service.
- **Payload:** Hiển thị thông tin nhóm để xác nhận thực thi thành công.

## 🛠 Yêu cầu hệ thống
- **Hệ điều hành:** Windows 10/11 (64-bit khuyến nghị)
- **Trình biên dịch:**
  - GCC (MinGW-w64) phiên bản 8.0 trở lên
  - Hoặc MSVC (Visual Studio 2019/2022)
- **Thư viện hệ thống:** Windows SDK (bao gồm ole32, shell32, shlwapi, psapi, uuid)
- **Công cụ bổ sung:** Git (tùy chọn), VS Code (khuyến nghị)
- **Môi trường thử nghiệm:** Máy ảo (VirtualBox, VMware) để tránh ảnh hưởng đến hệ thống thật

## 🔧 Hướng dẫn Build (Biên dịch)

Project sử dụng Windows API cấp thấp và COM interfaces, nên yêu cầu liên kết các thư viện hệ thống.

### Phương pháp 1: Sử dụng GCC (MinGW-w64)
1. Cài đặt MinGW-w64 từ [mingw-w64.org](https://mingw-w64.org/) hoặc qua MSYS2.
2. Đảm bảo `gcc` có trong PATH.
3. Mở Terminal (VS Code hoặc CMD) và điều hướng đến thư mục project:
   ```bash
   cd d:\UIT_Subject\CoCheMaDoc\BTN01
   ```
4. Biên dịch file cơ bản:
   ```bash
   gcc Nhom10_setup.c -o Nhom10_setup.exe -lole32 -luuid -lshell32 -lshlwapi -lpsapi
   ```
5. Biên dịch file nâng cao:
   ```bash
   gcc Nhom10_Advanced_setup.c -o Nhom10_Advanced_setup.exe -lole32 -luuid -lshell32 -lshlwapi -lpsapi
   ```

### Phương pháp 2: Sử dụng MSVC (Visual Studio)
1. Cài đặt Visual Studio với workload "Desktop development with C++".
2. Mở Developer Command Prompt hoặc Terminal trong VS Code với môi trường MSVC.
3. Điều hướng đến thư mục project:
   ```bash
   cd d:\UIT_Subject\CoCheMaDoc\BTN01
   ```
4. Biên dịch file cơ bản:
   ```bash
   cl Nhom10_setup.c /link ole32.lib uuid.lib shell32.lib shlwapi.lib psapi.lib /out:Nhom10_setup.exe
   ```
5. Biên dịch file nâng cao:
   ```bash
   cl Nhom10_Advanced_setup.c /link ole32.lib uuid.lib shell32.lib shlwapi.lib /out:Nhom10_Advanced_setup.exe
   ```

### Phương pháp 3: Sử dụng VS Code Tasks
Project có thể cấu hình task build trong VS Code:
1. Mở file `.vscode/tasks.json` (tạo nếu chưa có).
2. Thêm task tương tự như sau:
   ```json
   {
     "version": "2.0.0",
     "tasks": [
       {
         "label": "Build Basic",
         "type": "shell",
         "command": "gcc",
         "args": ["Nhom10_setup.c", "-o", "Nhom10_setup.exe", "-lole32", "-luuid", "-lshell32", "-lshlwapi", "-lpsapi"],
         "group": "build"
       },
       {
         "label": "Build Advanced",
         "type": "shell",
         "command": "gcc",
         "args": ["Nhom10_Advanced_setup.c", "-o", "Nhom10_Advanced_setup.exe", "-lole32", "-luuid", "-lshell32", "-lshlwapi", "-lpsapi"],
         "group": "build"
       }
     ]
   }
   ```
3. Chạy task qua Ctrl+Shift+P > "Tasks: Run Task" > Chọn "Build Basic" hoặc "Build Advanced".

## ▶️ Cách sử dụng
1. **Chuẩn bị môi trường:** Sử dụng máy ảo để tránh rủi ro.
2. **Build executable:** Theo hướng dẫn trên.
3. **Chạy chương trình:**
   - Double-click file `.exe` hoặc chạy từ command line.
   - Nếu không có quyền Admin, chương trình sẽ tự động bypass UAC.
4. **Quan sát:** Payload sẽ hiển thị MessageBox với thông tin nhóm nếu thực thi thành công.
5. **Kiểm tra persistence:** Khởi động lại máy để xem persistence hoạt động.

**Lưu ý:** Chương trình sẽ tự động thiết lập persistence. Để dọn dẹp, xóa registry keys, shortcuts và services theo hướng dẫn trong phần "Dọn dẹp".

## 🧹 Dọn dẹp sau thử nghiệm
Sau khi thử nghiệm, dọn dẹp để tránh ảnh hưởng:
- Xóa registry: `HKEY_CURRENT_USER\Software\Classes\ms-settings`, `HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Run\Nhom10GlobalAgent`, `HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run\Nhom10Popup`
- Xóa shortcut: `%APPDATA%\Microsoft\Windows\Start Menu\Programs\Startup\Nhom10.lnk`
- Dừng và xóa service: `sc stop Nhom10Service && sc delete Nhom10Service`
- Xóa file: `%APPDATA%\Nhom10\Nhom10.exe`

## 👥 Thông tin nhóm
- **Nhóm 10 - NT230.Q22**
- Thành viên:
  - 22520810 - Huynh Bao Long
  - 22520432 - Do Dang Hieu
  - 23520860 - Nguyen Quang Loc

## 📜 License
Dự án này chỉ dành cho mục đích giáo dục. Không có license thương mại. Sử dụng tại rủi ro của bạn.

## 🔗 Tài liệu tham khảo
- [Windows API Documentation](https://docs.microsoft.com/en-us/windows/win32/api/)
- [Malware Analysis Techniques](https://www.malwarebytes.com/malware)
- [GCC MinGW Setup](https://mingw-w64.org/)
- [MalwareSandboxEvasion: Techniques, Principles & Solutions (Apriorit)](https://www.apriorit.com/devblog/545-sandbox-evading-malware)