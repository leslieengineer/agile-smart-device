# Giao thức Xác thực Rhophi Claim và Ranh giới Tin cậy

> Trạng thái: Chỉ khả dụng trên hồ sơ `matter_node`. Lớp phần cứng đóng vai trò thuần túy là thực thể chứng minh (Prover): Tạo bằng chứng số (Proof) khi có thách thức hợp lệ thông qua thuật toán mật mã đối xứng, hoàn toàn cách ly, không tự thực hiện kết nối ra ngoài Internet.

## 1. Chu trình Thử thách - Phản hồi (Challenge-Response) qua cổng GATT BLE

```mermaid
sequenceDiagram
    autonumber
    participant Client as Ứng dụng mobile
    participant GATT as Dịch vụ GATT Claim
    participant Protocol as Giao thức Claim
    participant Material as Dữ liệu Claim nhà máy trong NVS
    participant Crypto as Mật mã ESP
    participant Store as Bộ lưu Claim trong NVS
    participant Node as Quá trình khởi động MatterNode

    Node->>Protocol: initialize()
    Protocol->>Material: Nạp material nhà máy
    Material-->>Protocol: Dữ liệu định danh và khóa bí mật
    Protocol->>Store: Nạp trạng thái Claim đã lưu
    Store-->>Protocol: Các cờ đã lưu
    Protocol-->>Node: Khởi tạo thành công hoặc lỗi
    opt Khởi tạo protocol thành công
        Node->>GATT: Khởi tạo và đăng ký service
        GATT-->>Node: Sẵn sàng hoặc lỗi đăng ký
    end
    Client->>GATT: Đọc Identity (36 byte)
    GATT->>Protocol: identity(now_ms)
    Protocol->>Protocol: Kiểm tra cửa sổ và cập nhật cờ
    Protocol-->>GATT: Phiên bản, product ID, claim ID, nonce và cờ
    GATT-->>Client: Identity theo định dạng truyền dữ liệu

    Client->>GATT: Ghi challenge (32 byte)
    GATT->>GATT: Kiểm tra độ dài, khóa mutex và gắn kết nối đầu tiên
    GATT->>Protocol: respond(challenge, now_ms)
    Protocol->>Protocol: Yêu cầu cửa sổ đang mở và từ chối phát lại challenge
    Protocol->>Crypto: HMAC-SHA256(secret, nonce || challenge || claim_id)
    Crypto-->>Protocol: Proof 32 byte
    Protocol->>Crypto: Xoay nonce
    Protocol->>Protocol: Ghi challenge vào replay cache
    Protocol-->>GATT: Proof
    GATT-->>Client: Gửi thông báo Response và trạng thái active
    opt Kết nối đã được gắn đọc Response
        Client->>GATT: Đọc Response
        GATT-->>Client: Proof 32 byte
    end
```

---

## 2. Quản lý Cửa sổ Giao thức và Cơ chế Chống tấn công phát lại (Replay Guard)

Hệ thống bảo vệ an ninh lớp vật lý thông qua việc kiểm soát chặt chẽ máy trạng thái của Cửa sổ Protocol và phân vùng lưu trữ bền vững:

### Máy trạng thái Cửa sổ giao thức (Protocol Window State):
```mermaid
stateDiagram-v2
    state "Đang khởi tạo" as Initializing
    state "Đóng" as Closed
    state "Đang hoạt động" as Active
    state "Vô hiệu" as Disabled
    [*] --> Initializing
    Initializing --> Closed: Material/state sẵn sàng và GATT đăng ký thành công
    Initializing --> Disabled: Khởi tạo hoặc đăng ký GATT thất bại
    Closed --> Active: open_window hợp lệ và tạo được nonce
    Active --> Active: Tạo proof, xoay nonce, ghi challenge vào replay cache
    Active --> Active: Từ chối replay hoặc request không hợp lệ
    Active --> Closed: Hết hạn, cancel, commissioning complete hoặc Matter window đóng
```

### Máy trạng thái Cờ quyền sở hữu bền vững (Persisted Ownership State):
```mermaid
stateDiagram-v2
    state "Chưa claim" as FactoryNew
    state "Chờ worker ghi" as PersistPending
    state "Đã claim" as Claimed
    state "Đang xóa" as Clearing
    [*] --> FactoryNew
    FactoryNew --> PersistPending: mark_commissioned đặt claimed và enqueue save
    PersistPending --> Claimed: Worker commit NVS thành công
    PersistPending --> PersistPending: Commit lỗi được log, caller không được báo
    Claimed --> Clearing: factory_reset xóa protocol state và NVS namespace
    Clearing --> FactoryNew: Xóa và commit thành công
    Clearing --> Clearing: Xóa lỗi, caller nhận io_error
```

### Các quy tắc quản lý an ninh tầng thấp:
*   **Đánh giá trễ thời hạn Cửa sổ (Lazy Evaluation):** Cửa sổ Protocol Claim không chạy một bộ định thời (Timer) độc lập. Trạng thái hết hạn chỉ được phát hiện một cách thụ động khi hàm `is_active(now_ms)` được kích hoạt bởi một yêu cầu truy cập GATT từ bên ngoài.
*   **Chống phát lại (Anti-Replay Cache):** Chuỗi Thách thức (`Challenge`) sau khi được xử lý thành công sẽ bị khóa chặt vào bộ đệm Replay Cache có dung lượng giới hạn. Bộ đệm này chỉ được làm sạch khi một chu kỳ mở cửa sổ mới được thiết lập.
*   **Ranh giới Bypass kiểm thử (`RHOPHI_CLAIM_DEV_BYPASS`):** Khi bật cờ biên dịch này trong môi trường phòng Lab, hàm `respond()` sẽ tự động bỏ qua thuật toán mã hóa đối xứng và trả về chuỗi Proof chứa toàn số 0. Bản tin này không có giá trị xác minh tính chính hãng trên BFF thực tế.
