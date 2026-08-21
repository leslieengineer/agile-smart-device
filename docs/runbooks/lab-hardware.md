# Bench lab

Thông tin dưới đây chỉ áp dụng bench ngày 2026-08-19 và không chứa secret.

| Vai trò | Định danh lab | Cách xác minh |
|---|---|---|
| ESP32-C6 node | `/dev/ttyACM1` | `python -m esptool --port /dev/ttyACM1 read-mac` |
| BBB console | `/dev/ttyACM0` | `ls -l /dev/ttyACM0` |
| BBB SSH | `192.168.7.2` | `ssh ${BBB_USER}@${BBB_HOST} hostname` |
| Android | A101SH | `adb devices -l` |
| RCP | USB by-id trên BBB | `systemctl cat otbr-agent` |

Port có thể đổi sau reconnect; không hard-code vào automation production.

## Biến shell

```bash
export PROJECT_DIR="$HOME/WS/agile-smart-device"
export BBB_HOST="192.168.7.2"
export BBB_USER="<bbb-user>"
export ESP_PORT="/dev/ttyACM1"
export BBB_CONSOLE="/dev/ttyACM0"
export ANDROID_SERIAL="<adb-serial>"
```

Không lưu password trong file này. Dùng SSH key/credential manager.

## Kiểm chứng bench

```bash
adb devices -l
ssh "${BBB_USER}@${BBB_HOST}" 'systemctl is-active otbr-agent matter-controller matter-gateway matter-web-auth'
ssh "${BBB_USER}@${BBB_HOST}" 'ip -6 addr show wpan0'
adb shell ip -6 route show table 1020
```

Expected: services active, `wpan0` có IPv6 và Android có route OMR qua BBB trước operational discovery.