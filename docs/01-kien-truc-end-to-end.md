# Kiến trúc end-to-end

## Trạng thái

| Khối | Source | Deployed | HIL |
|---|---|---|---|
| Firmware node | Có | ESP32-C6 | Từng phần |
| Android app | Có | A101SH debug APK | Từng phần |
| BBB platform | Có | BBB systemd | Từng phần |
| Full commissioning | Có | Đang chạy thử | Đang debug |

## Topology

```mermaid
flowchart LR
    APP[Android app] -->|HTTPS REST + SSE| BFF[BBB BFF]
    UI[WebUI] -->|HTTPS + SSE| BFF
    BFF -->|MQTT| GW[Gateway]
    GW -->|Unix RPC| MC[Matter.js Controller]
    MC -->|Matter IPv6| OTBR[OTBR]
    OTBR -->|Spinel HDLC| RCP[ESP32-C6 RCP]
    OTBR -->|Thread| NODE[ESP32-C6 node]
    APP -->|BLE PASE + Rhophi GATT| NODE
```

RCP chỉ cung cấp radio 802.15.4 cho OTBR. Nó không phải application node và không nhận JSON command.

## Trust boundary và dữ liệu

| Dữ liệu | Owner | Không được đi qua |
|---|---|---|
| Claim secret | Factory partition + encrypted BBB registry | Git, MQTT, log |
| Thread dataset | OTBR + protected provider + encrypted grant | UI, log, evidence |
| Mobile fabric key | Android CHIP storage | BFF/MQTT |
| BBB fabric key | Matter Controller storage | Browser/mobile |
| MQTT credential | Gateway/BFF | Browser |
| Inventory | Controller + BFF metadata | Static hard-code |

## On/Off round trip

```mermaid
sequenceDiagram
    participant UI as WebUI/Mobile
    participant BFF as BFF
    participant GW as Gateway
    participant MC as Matter Controller
    participant N as ESP32-C6 node
    UI->>BFF: POST /api/command
    BFF->>GW: MQTT home/control/tx
    GW->>MC: Unix RPC invoke
    MC->>N: Matter OnOff command
    N-->>MC: Attribute report
    MC-->>GW: attributeChanged
    GW-->>BFF: MQTT response/event
    BFF-->>UI: SSE
```

Local button đi qua cùng `SwitchRuntime`, sau đó publish Matter attribute report nên UI phải hội tụ về cùng state.

## Process ownership

- `otbr-agent` là process duy nhất được sở hữu serial RCP.
- `matter-controller` sở hữu fabric storage và Unix socket.
- `matter-gateway` chuyển contract MQTT sang RPC.
- `matter-web-auth` giữ credential server-side và phục vụ REST/SSE/WebUI.
- Mosquitto chỉ là message transport, không sở hữu device state.

## Giới hạn

Commissioning chỉ được đánh HIL pass khi node còn duy nhất permanent BBB fabric, inventory xuất hiện và On/Off hoạt động hai chiều.