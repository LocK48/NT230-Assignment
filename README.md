```markdown
# Malware Analysis Project - NT230.Q22 (Nhóm 10)

## 📝 Giới thiệu chung
Dự án này là một mẫu mã độc thực thi (PoC) được thiết kế cho mục đích học tập trong môn **Cơ chế mã độc (NT230)** tại **UIT**. Mã nguồn tập trung vào việc mô phỏng các kỹ thuật nâng cao bao gồm: Anti-Analysis (Gỡ lỗi & Máy ảo), Leo thang đặc quyền (UAC Bypass) và Duy trì sự hiện diện (Persistence).

## 🛠 Hướng dẫn Build (Biên dịch)

Do project sử dụng các hàm Windows API cấp thấp và giao diện COM, quá trình biên dịch yêu cầu liên kết (link) các thư viện hệ thống của Windows.

### Yêu cầu hệ thống:
* **Trình biên dịch:** GCC (MinGW-w64) hoặc MSVC (Visual Studio).
* **Môi trường:** Windows 10/11 (Khuyến nghị sử dụng máy ảo Lab để thử nghiệm).

### Lệnh biên dịch (GCC):
Sử dụng Terminal trong VS Code hoặc CMD để chạy lệnh sau:

```bash
gcc Nhom10_setup.c -o Nhom10_setup.exe -lole32 -luuid -lshell32 -lshlwapi -lpsapi