Cách hiện tại build được, nhưng chưa tối ưu về ownership. Việc đặt cả products/smart_device trong framework khiến repository project gốc gần như chỉ là ESP-IDF launcher, đồng thời mọi thay đổi sản phẩm phải qua vòng đời Git của submodule.

Khuyến nghị
Không chuyển toàn bộ Layer 4 và Layer 5 vào project gốc.

Nên chia như sau.

Thành phần	Nơi phù hợp
UHAL contracts, common types	Framework
ESP32-C6 adapter dùng chung	Framework
Device driver tái sử dụng như SHT3x	Framework
Protocol như Modbus, frame protocol	Framework
CRC, filter, serialization, retry	Framework
Service thực sự độc lập với sản phẩm	Framework
Product smart_device	Project gốc
Board pinout và peripheral mapping	Project gốc
Feature selection và configuration	Project gốc
Composition root	Project gốc
Logic chỉ có ý nghĩa với smart device	Project gốc
Nói ngắn gọn

Framework chứa Layer 1–4 có thể tái sử dụng. Project firmware sở hữu Layer 5 và những phần Layer 4 chỉ dành riêng cho sản phẩm.

Cấu trúc đề xuất
agile-smart-device/
├─ CMakeLists.txt
├─ main/
│  ├─ CMakeLists.txt
│  └─ main.cpp                     # Chỉ chứa app_main()
├─ components/
│  ├─ framework_uhal_core/         # ESP-IDF bridge tới framework UHAL core
│  ├─ product_smart_device/
│  │  ├─ CMakeLists.txt
│  │  ├─ include/smart_device/
│  │  └─ src/
│  │     └─ SmartDevice.cpp        # Composition root
│  ├─ board_esp32c6/
│  │  ├─ CMakeLists.txt
│  │  ├─ include/
│  │  └─ src/
│  └─ app_services/                # Chỉ các service riêng của sản phẩm
└─ external/
   └─ agile-firmware-framework/
      ├─ components/
      │  ├─ uhal/
      │  ├─ devices/
      │  ├─ protocols/
      │  ├─ services/
      │  ├─ libraries/
      │  └─ platform/esp32c6/
      └─ products/
         └─ env_monitor/           # Sample/reference product
Luồng dependency nên là

app_main
   |
   v
product_smart_device
   |
   +---- board_esp32c6
   +---- framework devices
   +---- framework protocols
   +---- framework services
   +---- framework libraries
              |
              v
        UHAL interfaces
              ^
              |
      ESP32-C6 adapters
main vẫn nên rất mỏng
Ý tưởng main chỉ gọi product là hợp lý. Điểm cần thay đổi là product đó thuộc project gốc, không nằm trong framework.

extern "C" void app_main()
{
    const uhal::Status status = smart_device::start();

    if (status != uhal::Status::ok) {
        ESP_LOGE(kTag, "Failed to start smart device");
    }
}
smart_device::start() nằm tại

agile-smart-device/components/product_smart_device/
Hàm này tạo ESP32-C6 adapters, devices, protocols và services rồi inject dependency.

Layer 4 nên phân loại theo khả năng tái sử dụng
Không phải tất cả service đều tự động thuộc framework.

Giữ trong framework
Sht3x
Modbus RTU
frame protocol
CRC
ring buffer
generic telemetry encoder
retry/backoff
generic data logger
generic OTA state machine
reusable calibration algorithm
Giữ trong project gốc
SmartDeviceController
quy tắc bật/tắt tính năng riêng của sản phẩm
mapping sensor vào telemetry topic cụ thể
product boot sequence
provisioning flow riêng
LED/error policy riêng của board
lịch sampling cố định cho sản phẩm
cấu hình cloud endpoint
logic kết hợp nhiều service thành use case
Một quy tắc thực tế tốt là

Nếu thành phần có thể được dùng nguyên trạng bởi sản phẩm thứ hai, đưa nó vào framework. Nếu nó mô tả hành vi của smart_device, giữ nó trong product repository.

Không nên đưa một service vào framework chỉ vì nó nằm trong thư mục tên services.

Vấn đề của cách hiện tại
Product nằm trong submodule tạo ra một số bất tiện.

Hai vòng đời Git

Thay đổi sản phẩm phải commit trong framework trước, sau đó cập nhật gitlink tại project gốc.

Framework và sản phẩm bị release cùng nhau

Một thay đổi nhỏ trong smart device tạo version framework mới dù các component reusable không thay đổi.

Ownership không rõ

Framework trở thành monorepo chứa cả catalog và firmware sản phẩm, trong khi project gốc chỉ còn build shell.

CI phức tạp

Phải test framework commit, cập nhật submodule commit, rồi test firmware parent.

Khó tạo sản phẩm khác

Mỗi product mới lại phải được thêm vào framework root CMake và làm repository framework tăng phạm vi trách nhiệm.

Khi nào cách hiện tại vẫn hợp lý
Giữ product trong framework hợp lý nếu chủ đích của repository framework là một firmware monorepo, quản lý đồng thời

reusable components,
platform adapters,
nhiều products,
board targets,
release của từng firmware.
Khi đó project agile-smart-device bên ngoài gần như không cần tồn tại. Có thể build ESP-IDF product trực tiếp từ framework monorepo.

Nhưng nếu agile-firmware-framework được định nghĩa là reusable catalog dùng bởi nhiều firmware repository, Layer 5 không nên nằm trong đó.

Cách port hiệu quả
Tôi khuyến nghị chuyển sang mô hình này.

Giữ submodule tại external/agile-firmware-framework.
Giữ Layer 2 và reusable Layer 4 trong framework.
Hoàn thiện generic ESP32-C6 low-level/adapters trong framework.
Di chuyển products/smart_device ra project gốc thành components/product_smart_device.
Giữ main/app_main() mỏng.
Chỉ link các framework component mà sản phẩm thực sự sử dụng.
Khi một component trong project chứng minh được khả năng tái sử dụng, chuyển nó ngược vào framework trong một thay đổi riêng.
Đây là ranh giới cân bằng nhất

Framework = capability catalog
Project   = product definition
main      = platform entry
Do đó, không nên chuyển toàn bộ devices, protocols, services, libraries vào project gốc. Chỉ nên chuyển product Layer 5 và những service Layer 4 mang tính sản phẩm.

## Trạng thái hiện tại

- Layer 5 đã được đặt tại `components/product_smart_device` trong project gốc.
- `main/` chỉ giữ `app_main()` và ESP-IDF logging.
- `components/framework_uhal_core` là nơi duy nhất biết đường dẫn vật lý tới UHAL core trong submodule.
- Framework submodule chỉ chứa capability tái sử dụng; không sở hữu product `smart_device`.
- Logic riêng của smart device ở parent. Chỉ chuyển một component vào framework bằng thay đổi riêng sau khi đã chứng minh khả năng tái sử dụng.

## Smart switch milestone 1

Implemented:

- Framework pure `ButtonInput` debounce and short-press classifier with host tests.
- ESP32-C6 Layer 1 GPIO, interrupt, and system clock using ESP-IDF 6.0.2.
- ESP32-C6 Layer 3 `OutputPin`, `InputPin`, `PinInterrupt`, and `Clock` UHAL adapters.
- nanoESP32-C6 board mapping: button GPIO9, relay GPIO10, LED GPIO2.
- Product `SwitchController`, NVS relay repository, FreeRTOS runtime, and safe composition order.
- 16 MB DIO flash configuration and 1 kHz FreeRTOS tick.

Deferred: SoftAP, Wi-Fi, Web, BLE Mesh, MQTT, OTA, scenes, dimmer, multi-channel, long/double press, factory reset, and on-target hardware acceptance.
