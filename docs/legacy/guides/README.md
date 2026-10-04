# Bốn tài liệu tiêu chuẩn triển khai và vận hành

Các tài liệu này giải thích đầy đủ lệnh setup, build, test, run và vận hành. Ubuntu 24.04 áp dụng cho workstation của ESP32-C6, Mobile và WebUI; BBB target dùng Debian của board.

1. [ESP32-C6 trên Ubuntu 24.04](01-esp32c6-ubuntu-24.04.md)
2. [Mobile App trên Ubuntu 24.04](02-mobile-app-ubuntu-24.04.md)
3. [WebUI trên Ubuntu 24.04](03-webui-ubuntu-24.04.md)
4. [BeagleBone Black Debian 11 production và dashboard.rhophi.uk](05-bbb-debian11-production-dashboard.md)

Phụ lục: [Tra cứu lệnh BBB/OTBR/Controller](04-beaglebone-black-operations.md).

## Quy tắc sử dụng

- Chạy lệnh từ đúng directory được ghi trong từng mục.
- Thay placeholder `<...>`; không copy nguyên placeholder vào production.
- Không đưa password, token, Thread dataset, registry key hoặc private key vào Git/shell history.
- Lệnh flash, erase, uninstall, factory reset, remove node và xóa storage là destructive.
- Build pass không đồng nghĩa HIL pass.