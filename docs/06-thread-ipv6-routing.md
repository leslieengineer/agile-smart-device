# Thread và IPv6 routing

## Trạng thái

| Hành vi | Source | Deployed | HIL |
|---|---|---|---|
| OTBR border routing | Upstream OTBR | Active | Có |
| Android route tới OMR | Runbook/config lab | Active | Route table xác nhận |
| Operational discovery | ConnectedHomeIP | Có | Từng phần |

## Đường dữ liệu

```mermaid
flowchart LR
    Phone[Android Wi-Fi] --> AP[LAN/AP]
    AP --> ETH[BBB eth0]
    ETH --> OTBR[OTBR border routing]
    OTBR --> WPAN[wpan0]
    WPAN --> Node[Thread node OMR address]
    BBBRA[BBB radvd unicast RIO] --> Phone
```

Node nhận OMR prefix từ OTBR. Android phải có route tới prefix này trước stage `FindOperationalForStayActive`, nếu không ConnectedHomeIP trả `ENETUNREACH`.

## Router Advertisement

OTBR gửi RIO multicast trên infra interface. Một số AP không bridge multicast RA từ wired sang Wi-Fi. Bench hiện tại dùng `radvd` gửi unicast RIO tới link-local Android. Đây là workaround hạ tầng, không phải secret hay Matter credential.

## Kiểm chứng read-only

Android:

```bash
adb shell ip -6 addr show wlan0
adb shell ip -6 route show table 1020
```

BBB:

```bash
systemctl is-active otbr-agent radvd
ip -6 addr show eth0
ip -6 addr show wpan0
sudo ot-ctl state
sudo ot-ctl br state
sudo ot-ctl br omrprefix
```

Không đưa output `dataset active -x` vào tài liệu hoặc evidence.

## Điều kiện commissioning

- Phone và BBB phải giao tiếp IPv6 trên cùng infra link.
- BBB forwarding bật và có route upstream.
- Android route OMR trỏ qua BBB.
- mDNS multicast lock permission có trong APK.
- Node đã attach Thread trước operational discovery.

## Giới hạn

Unicast RA client link-local có thể thay đổi sau reconnect Wi-Fi; runbook phải cập nhật client an toàn. Production nên dùng LAN/AP bridge RIO đúng chuẩn thay vì cấu hình per-phone.