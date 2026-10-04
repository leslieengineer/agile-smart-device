# Phụ lục — Lệnh BeagleBone Black, OTBR, Matter Controller và WebUI

> **PHỤ LỤC TRA CỨU.** Tài liệu vận hành production chính là [BBB Debian 11 và dashboard.rhophi.uk](05-bbb-debian11-production-dashboard.md).

Tài liệu này giải thích chi tiết các lệnh BBB để tra cứu nhanh. Nó mô tả BBB từ lúc truy cập OS đến khi chạy OTBR, Mosquitto, Matter Controller, Gateway, BFF, WebUI, provisioning và thao tác node.

> BBB bench hiện dùng Debian Bullseye, không phải Ubuntu 24.04. Máy dùng để build/stage artifact có thể là Ubuntu 24.04. Không chạy binary x86_64 trên BBB ARM.

## 1. Mô hình process

```text
Android/WebUI
    │ HTTPS REST + SSE
    ▼
matter-web-auth (BFF)
    │ MQTT
    ▼
Mosquitto ──► matter-gateway
                  │ Unix JSON-lines RPC
                  ▼
            matter-controller
                  │ Matter IPv6
                  ▼
               OTBR/wpan0
                  │ Thread
                  ▼
           ESP32-C6 application node

otbr-agent ── Spinel/HDLC UART ── ESP32-C6 RCP
```

Quy tắc ownership:

- Chỉ `otbr-agent` mở RCP serial.
- `matter-controller` giữ permanent fabric storage.
- `matter-gateway` không mở RCP và không giữ fabric key.
- BFF giữ web/mobile auth và provisioning files.
- Browser không giữ MQTT credential.

## 2. Biến vận hành trên máy Ubuntu 24.04

```bash
export PROJECT_DIR="$HOME/WS/agile-smart-device"
export DASHBOARD_DIR="$PROJECT_DIR/dashboard-reference"
export BBB_HOST="192.168.7.2"
export BBB_USER="<bbb-user>"
export BBB_STAGE="/home/<bbb-user>/agile-dashboard"
```

- `BBB_HOST` là USB Ethernet hoặc LAN IP.
- `BBB_USER` là account SSH, không ghi password trong file.
- `BBB_STAGE` là nơi nhận artifact trước khi sudo install.

Kiểm tra reachability:

```bash
ping -c 3 "$BBB_HOST"
ssh "$BBB_USER@$BBB_HOST" hostname
```

- `ping -c 3` gửi ba ICMP echo rồi dừng.
- `ssh user@host command` chạy một command remote và trả output.

## 3. Truy cập BBB lần đầu

### Qua USB serial console

Liệt kê console:

```bash
ls -l /dev/ttyACM*
```

Mở bằng screen:

```bash
sudo apt install -y screen
screen /dev/ttyACM0 115200
```

- `screen <port> 115200` mở serial terminal.
- Thoát screen bằng `Ctrl+A`, sau đó `K`, xác nhận `y`.

### Qua SSH

```bash
ssh "$BBB_USER@$BBB_HOST"
```

Tạo key trên Ubuntu nếu chưa có:

```bash
ssh-keygen -t ed25519 -a 100 -f "$HOME/.ssh/id_ed25519"
ssh-copy-id "$BBB_USER@$BBB_HOST"
```

- `-t ed25519` chọn key hiện đại.
- `-a 100` tăng KDF rounds bảo vệ private key.
- `-f` chọn file.
- `ssh-copy-id` cài public key, không copy private key.

Test passwordless:

```bash
ssh -o BatchMode=yes "$BBB_USER@$BBB_HOST" 'id && hostname'
```

`BatchMode=yes` không prompt password; fail ngay nếu key chưa hoạt động.

## 4. Baseline OS trên BBB

```bash
cat /etc/os-release
uname -a
uname -m
df -h
free -h
ip -brief address
```

Ý nghĩa:

- `os-release` cho distribution/version.
- `uname -m` cho architecture, thường `armv7l` trên BBB.
- `df -h` kiểm tra filesystem free.
- `free -h` kiểm tra RAM/swap.
- `ip -brief address` tóm tắt interfaces/IP.

Update package index và security updates:

```bash
sudo apt update
sudo apt upgrade -y
sudo reboot
```

- `upgrade` có thể restart library/service; thực hiện trước production deploy.
- `reboot` ngắt SSH; chờ BBB lên lại và reconnect.

Cài công cụ:

```bash
sudo apt install -y git curl ca-certificates jq socat \
  mosquitto mosquitto-clients avahi-daemon radvd \
  python3 rsync acl
```

- `jq` parse JSON health/API.
- `socat` gửi JSON-lines vào Unix socket.
- `avahi-daemon` hỗ trợ mDNS tools; OTBR/Matter vẫn có mDNS implementation riêng.
- `radvd` dùng cho IPv6 route advertisement workaround.
- `rsync` deploy static assets.
- `acl` hỗ trợ kiểm tra/điều chỉnh file ACL nếu cần.

## 5. Clock và hostname

```bash
sudo hostnamectl set-hostname BeagleBone
sudo timedatectl set-timezone Asia/Ho_Chi_Minh
timedatectl status
```

- `hostnamectl` đặt hostname persistent.
- `timedatectl set-timezone` chỉ đổi timezone hiển thị, không đổi UTC clock.
- Matter certificate/time validation cần clock đúng.

Bật NTP:

```bash
sudo timedatectl set-ntp true
timedatectl show -p NTPSynchronized
```

Expected `NTPSynchronized=yes` sau khi có network.

## 6. RCP và OTBR

### 6.1 Xác định RCP

```bash
ls -l /dev/serial/by-id/
```

Dùng symlink by-id trong production. Không dùng `/dev/ttyUSB0` nếu device có thể re-enumerate.

Xác định process đang mở port:

```bash
sudo fuser -v /dev/serial/by-id/<RCP_DEVICE>
```

- `fuser -v` liệt kê PID/user/access mode.
- Expected chỉ `otbr-agent` sau khi stack chạy.

### 6.2 Cấu hình radio URL

Radio URL điển hình:

```text
spinel+hdlc+uart:///dev/serial/by-id/<RCP_DEVICE>?uart-baudrate=460800
```

Ý nghĩa:

- `spinel+hdlc+uart` chọn Spinel framing qua UART.
- path là RCP serial.
- `uart-baudrate=460800` phải khớp RCP firmware.

Xem command thực tế:

```bash
systemctl cat otbr-agent
ps aux | grep '[o]tbr-agent'
```

- `systemctl cat` in unit/drop-in/env references.
- pattern `[o]tbr-agent` tránh grep tự match chính nó.

### 6.3 Điều khiển OTBR service

```bash
sudo systemctl enable otbr-agent
sudo systemctl start otbr-agent
sudo systemctl status otbr-agent --no-pager --full
```

- `enable` chạy sau boot.
- `start` chạy ngay, không restart nếu đã active.
- `status --no-pager --full` không cắt dòng và không mở pager.

Dừng/tắt:

```bash
sudo systemctl stop otbr-agent
sudo systemctl disable otbr-agent
```

- `stop` dừng runtime, Thread network mất Border Router.
- `disable` chỉ bỏ auto-start; không nhất thiết dừng process đang chạy.

Restart sau đổi RCP/config:

```bash
sudo systemctl restart otbr-agent
```

`restart` gây outage Thread ngắn; không dùng khi commissioning đang chạy.

### 6.4 Kiểm tra Thread

```bash
sudo ot-ctl state
sudo ot-ctl role
sudo ot-ctl dataset active
sudo ot-ctl netdata show
sudo ot-ctl br state
sudo ot-ctl br omrprefix
```

- `state/role` expected `leader` hoặc `router` cho OTBR.
- `dataset active` in dataset dạng field; không lưu master key vào ticket.
- `netdata show` cho prefixes/routes/services.
- `br state` expected `running`.
- `br omrprefix` cho prefix mà Android/Controller cần route.

Không chạy/paste `dataset active -x` vào log chia sẻ vì output chứa full secret dataset.

Xem node Thread:

```bash
sudo ot-ctl child table
sudo ot-ctl neighbor table
sudo ot-ctl router table
```

- Child table chứa sleepy/end devices trực tiếp.
- Neighbor table chứa adjacent router/child.
- Router table cho Thread routers/RLOC16.

## 7. IPv6 forwarding và Android route

Kiểm tra:

```bash
sysctl net.ipv6.conf.all.forwarding
ip -6 route
ip -6 addr show eth0
ip -6 addr show wpan0
```

Expected forwarding `1`, BBB có infra IPv6 trên `eth0` và OMR/mesh addresses trên `wpan0`.

Bật forwarding persistent:

```bash
printf 'net.ipv6.conf.all.forwarding=1\nnet.ipv6.conf.eth0.accept_ra=2\n' | \
  sudo tee /etc/sysctl.d/90-otbr-routing.conf
sudo sysctl --system
```

- `tee` ghi config với sudo.
- `accept_ra=2` cho phép BBB vẫn nhận upstream RA khi forwarding bật.
- `sysctl --system` reload mọi sysctl config.

### Unicast RIO cho AP không bridge multicast RA

Lấy link-local Android:

```bash
adb shell ip -6 addr show wlan0
```

Lấy OMR:

```bash
ssh "$BBB_USER@$BBB_HOST" 'sudo ot-ctl br omrprefix'
```

Tạo `/etc/radvd.conf` với placeholder đã thay đúng:

```text
interface eth0
{
    AdvSendAdvert on;
    UnicastOnly on;
    MinRtrAdvInterval 10;
    MaxRtrAdvInterval 30;
    AdvDefaultLifetime 90;
    AdvDefaultPreference low;
    route <OMR_PREFIX>/64
    {
        AdvRoutePreference high;
        AdvRouteLifetime 180;
    };
    clients
    {
        <ANDROID_LINK_LOCAL>;
    };
};
```

Validate/start:

```bash
sudo radvd --configtest -C /etc/radvd.conf
sudo systemctl enable --now radvd
sudo systemctl status radvd --no-pager
```

- `--configtest` parse config nhưng không chạy daemon.
- `enable --now` bật boot và start.

Kiểm tra Android:

```bash
adb shell ip -6 route show table 1020
```

Expected OMR prefix `via <BBB_LINK_LOCAL> dev wlan0 proto ra`.

Nếu điện thoại đổi link-local sau reconnect Wi-Fi, cập nhật `clients` rồi restart radvd. Production nên sửa LAN/AP để multicast RIO hoạt động thay vì per-phone config.

## 8. Node.js runtime trên BBB

Repository installer dùng `/opt/node20/bin/node`. Kiểm tra:

```bash
/opt/node20/bin/node --version
```

Nếu chưa có, dùng script production của repository hoặc cài Node 20 ARM phù hợp. Không copy Node binary x86_64 từ Ubuntu.

Kiểm architecture binary:

```bash
file /opt/node20/bin/node
uname -m
```

`file` phải báo ELF ARM phù hợp với `uname -m`.

## 9. Build artifact trên Ubuntu 24.04

Trên workstation:

```bash
cd "$DASHBOARD_DIR"
npm ci
npm test -- --run
npm run build
```

Bundle Node services:

```bash
mkdir -p /tmp/agile-dashboard-stage
npx esbuild packages/matter-controller/src/main.ts \
  --bundle --platform=node --format=cjs --external:bun:sqlite \
  --outfile=/tmp/agile-dashboard-stage/matter-controller.cjs
npx esbuild packages/gateway/src/main.ts \
  --bundle --platform=node --format=cjs \
  --outfile=/tmp/agile-dashboard-stage/gateway.cjs
npx esbuild packages/webui-bff/src/main.ts \
  --bundle --platform=node --format=cjs \
  --outfile=/tmp/agile-dashboard-stage/webui-bff.cjs
rsync -a apps/webui/dist/ /tmp/agile-dashboard-stage/webui/
```

- `/tmp` là staging, không phải source.
- `--external:bun:sqlite` tránh resolve optional Bun module trong Node bundle.
- `rsync -a` giữ directory tree/timestamps/mode cơ bản.

Copy lên BBB:

```bash
ssh "$BBB_USER@$BBB_HOST" "mkdir -p '$BBB_STAGE'"
scp -r /tmp/agile-dashboard-stage/. "$BBB_USER@$BBB_HOST:$BBB_STAGE/"
```

- `scp -r` copy directory recursively qua SSH.
- Dấu `/.` copy nội dung thay vì tạo thêm directory level.

## 10. Mosquitto production-style

### 10.1 Tạo user/password

```bash
sudo install -d -m 0750 -o root -g mosquitto /etc/mosquitto
sudo mosquitto_passwd -c /etc/mosquitto/passwd gateway
sudo mosquitto_passwd /etc/mosquitto/passwd webui
```

- `install -d` tạo directory với owner/group/mode.
- `mosquitto_passwd -c` tạo file mới; chỉ dùng `-c` lần đầu vì nó overwrite file.
- Lệnh sau thêm/cập nhật user.

### 10.2 ACL

Ví dụ `/etc/mosquitto/aclfile`:

```text
user gateway
topic readwrite home/control/#

user webui
topic write home/control/tx
topic read home/control/rx
topic read home/control/status
```

Gateway cần đọc/write command bus. BFF/WebUI user chỉ cần publish tx và đọc response/status.

### 10.3 Listener config

Ví dụ `/etc/mosquitto/conf.d/matter.conf`:

```text
per_listener_settings true
max_packet_size 16384

listener 1883 127.0.0.1
protocol mqtt
allow_anonymous false
password_file /etc/mosquitto/passwd
acl_file /etc/mosquitto/aclfile
```

Port 1883 chỉ bind loopback; không expose MQTT thô ra LAN/Internet.

Validate và restart:

```bash
sudo mosquitto -c /etc/mosquitto/mosquitto.conf -t
sudo systemctl enable --now mosquitto
sudo systemctl restart mosquitto
sudo systemctl status mosquitto --no-pager
```

- `-t` kiểm config rồi thoát.
- Restart chỉ sau validate pass.

Xem log:

```bash
sudo journalctl -u mosquitto -n 100 --no-pager
```

## 11. System users và quyền

Các service nên chạy bằng system user riêng:

```bash
getent group matter-rpc || sudo groupadd --system matter-rpc
id matter-controller || sudo useradd --system --no-create-home --shell /usr/sbin/nologin matter-controller
id matter-gateway || sudo useradd --system --no-create-home --shell /usr/sbin/nologin matter-gateway
id matter-webui || sudo useradd --system --no-create-home --shell /usr/sbin/nologin matter-webui
sudo usermod -aG matter-rpc matter-controller
sudo usermod -aG matter-rpc matter-gateway
sudo usermod -aG matter-rpc matter-webui
```

- `getent/id ... ||` chỉ tạo khi chưa tồn tại.
- `--system` tạo account không dành login người dùng.
- `--no-create-home` tránh home không cần thiết.
- `nologin` chặn interactive shell.
- `matter-rpc` cho phép truy cập controller socket.

## 12. Install Matter Controller

Stage expected bởi script hiện tại là `/home/leslie/agile-dashboard`; nếu dùng user/path khác, sửa script/config có review hoặc copy artifact đúng path.

```bash
sudo bash "$BBB_STAGE/install-matter-controller.sh"
```

Script:

1. Kiểm tra bundle/unit/Node.
2. Tạo group/user.
3. Install `/opt/matter-controller/matter-controller.cjs`.
4. Install systemd unit.
5. Thêm Gateway supplementary group.
6. daemon-reload, enable, restart.

Điều khiển:

```bash
sudo systemctl start matter-controller
sudo systemctl stop matter-controller
sudo systemctl restart matter-controller
sudo systemctl enable matter-controller
sudo systemctl disable matter-controller
sudo systemctl status matter-controller --no-pager --full
```

- `start/stop` bật/tắt runtime.
- `restart` stop rồi start, giữ persistent storage.
- `enable/disable` chỉ đổi boot policy.

Log realtime:

```bash
sudo journalctl -u matter-controller -f
```

`-f` follow log; thoát `Ctrl+C`.

Persistent paths:

```text
/var/lib/matter-controller
/run/matter-controller/controller.sock
/opt/matter-controller/matter-controller.cjs
```

Không xóa `/var/lib/matter-controller` nếu còn node thuộc BBB fabric.

## 13. Matter Controller RPC

Cài `socat` rồi dùng user thuộc `matter-rpc`.

Health:

```bash
printf '%s\n' '{"id":"health-1","method":"health"}' | \
  sudo -u matter-gateway socat - UNIX-CONNECT:/run/matter-controller/controller.sock
```

- `printf` gửi một JSON line.
- `sudo -u matter-gateway` dùng quyền thật của Gateway.
- `socat - UNIX-CONNECT:` nối stdin/stdout với Unix socket.

List node:

```bash
printf '%s\n' '{"id":"list-1","method":"listNodes"}' | \
  sudo -u matter-gateway socat - UNIX-CONNECT:/run/matter-controller/controller.sock
```

Describe node:

```bash
NODE_ID="0x0000000000000001"
printf '%s\n' "{\"id\":\"describe-1\",\"method\":\"describeNode\",\"params\":{\"node_id\":\"$NODE_ID\"}}" | \
  sudo -u matter-gateway socat - UNIX-CONNECT:/run/matter-controller/controller.sock
```

`NODE_ID` luôn là text 64-bit hex để tránh JavaScript number precision loss.

Read OnOff attribute:

```bash
printf '%s\n' "{\"id\":\"read-1\",\"method\":\"read\",\"params\":{\"node_id\":\"$NODE_ID\",\"endpoint\":1,\"cluster\":6,\"attribute\":0}}" | \
  sudo -u matter-gateway socat - UNIX-CONNECT:/run/matter-controller/controller.sock
```

- cluster 6 = OnOff `0x0006`.
- attribute 0 = OnOff `0x0000`.

Subscribe:

```bash
printf '%s\n' "{\"id\":\"sub-1\",\"method\":\"subscribe\",\"params\":{\"node_id\":\"$NODE_ID\"}}" | \
  sudo -u matter-gateway socat - UNIX-CONNECT:/run/matter-controller/controller.sock
```

Invoke On:

```bash
printf '%s\n' "{\"id\":\"on-1\",\"method\":\"invoke\",\"params\":{\"node_id\":\"$NODE_ID\",\"endpoint\":1,\"cluster\":6,\"command\":1,\"payload\":{}}}" | \
  sudo -u matter-gateway socat - UNIX-CONNECT:/run/matter-controller/controller.sock
```

Command IDs OnOff:

- `0` = Off.
- `1` = On.
- `2` = Toggle.

Chỉ invoke sau khi xác minh đúng node/endpoint và relay an toàn.

## 14. Thêm node vào BBB

### Cách khuyến nghị — Mobile handoff

1. Mobile claim node.
2. Mobile BLE PASE và cấp Thread dataset.
3. Node attach OTBR.
4. Mobile mở ECW.
5. Mobile gọi BFF `/window`.
6. BFF gọi controller `commissionOnNetwork`.
7. Controller describe/read/subscribe.
8. Mobile RemoveFabric temporary fabric.
9. BFF complete và inventory cập nhật.

Đây là cách chuẩn vì BBB bench không có BLE.

### RPC trực tiếp khi ECW đã mở

ECW phải được mở bởi một commissioner đang có fabric. Không dùng setup passcode cũ nếu ECW trả passcode mới.

```bash
ECW_PASSCODE="<one-time-ecw-passcode>"
ECW_DISCRIMINATOR="<ecw-discriminator>"
printf '%s\n' "{\"id\":\"commission-1\",\"method\":\"commissionOnNetwork\",\"params\":{\"setup_passcode\":$ECW_PASSCODE,\"discriminator\":$ECW_DISCRIMINATOR}}" | \
  sudo -u matter-gateway socat - UNIX-CONNECT:/run/matter-controller/controller.sock
unset ECW_PASSCODE ECW_DISCRIMINATOR
```

- Passcode/discriminator là one-time lab secret, không ghi vào shared history.
- `unset` xóa shell variables sau dùng.
- Response trả permanent `node_id` khi thành công.

### Xác minh thêm node

```bash
sudo ot-ctl child table
sudo ot-ctl neighbor table
```

Sau đó gọi `listNodes`, `describeNode`, `read`, `subscribe` như phần RPC.

## 15. Xóa node

Ưu tiên API/UI remove để metadata và controller cùng cập nhật. RPC trực tiếp:

```bash
printf '%s\n' "{\"id\":\"remove-1\",\"method\":\"removeNode\",\"params\":{\"node_id\":\"$NODE_ID\"}}" | \
  sudo -u matter-gateway socat - UNIX-CONNECT:/run/matter-controller/controller.sock
```

Remove có thể gửi RemoveFabric tới node rồi xóa local controller state. Backup/evidence trước; không thể điều khiển node bằng BBB fabric sau khi xóa.

Xác minh:

```bash
printf '%s\n' '{"id":"list-after-remove","method":"listNodes"}' | \
  sudo -u matter-gateway socat - UNIX-CONNECT:/run/matter-controller/controller.sock
```

## 16. Install và điều khiển Gateway

Expected files:

```text
/opt/matter-gateway/gateway.cjs
/etc/matter-gateway/gateway.env
/etc/systemd/system/matter-gateway.service
```

Env quan trọng:

```text
MQTT_URL=mqtt://127.0.0.1:1883
MQTT_USERNAME=gateway
MQTT_PASSWORD=<protected>
MQTT_CLIENT_ID=matter-gateway
MQTT_TX_TOPIC=home/control/tx
MQTT_RX_TOPIC=home/control/rx
CONTROLLER_MODE=matterjs
CONTROLLER_TIMEOUT_MS=5000
LOG_LEVEL=info
```

- `CONTROLLER_MODE=matterjs` điều khiển hardware thật.
- `mock` chỉ cho test, không dùng acceptance.
- env file nên mode 0640, owner root:service-group.

Điều khiển:

```bash
sudo systemctl enable matter-gateway
sudo systemctl restart matter-gateway
sudo systemctl status matter-gateway --no-pager --full
sudo journalctl -u matter-gateway -n 100 --no-pager
```

Gateway phải start sau controller socket. Unit drop-in có `After/Wants=matter-controller.service`.

## 17. Install BFF và WebUI

Expected paths:

```text
/opt/matter-web-auth/webui-bff.cjs
/opt/matter-web-auth/public/
/etc/matter-web-auth/webui.env
/etc/matter-web-auth/admin.env
/var/lib/matter-web-auth/
```

Env BFF chính:

```text
WEBUI_BIND=127.0.0.1
WEBUI_PORT=8082
WEBUI_PUBLIC_ORIGIN=https://<public-host>
WEBUI_ROOT=/opt/matter-web-auth/public
MOBILE_ALLOWED_ORIGINS=https://localhost
MQTT_URL=mqtt://127.0.0.1:1883
PROVISIONING_ENABLED=true
PROVISIONING_REGISTRY_PATH=/etc/matter-provisioning/devices.registry.enc
PROVISIONING_REGISTRY_KEY_FILE=/etc/matter-provisioning/registry.key
THREAD_DATASET_PATH=/etc/matter-provisioning/thread-dataset.hex
PROVISIONING_TRANSACTION_PATH=/var/lib/matter-web-auth/provisioning-transactions.json
MATTER_SOCKET_PATH=/run/matter-controller/controller.sock
```

`WEBUI_BIND=127.0.0.1` yêu cầu reverse proxy/tunnel trên cùng BBB. Không bind public trực tiếp nếu chưa có TLS/auth hardening.

Điều khiển:

```bash
sudo systemctl enable matter-web-auth
sudo systemctl restart matter-web-auth
sudo systemctl status matter-web-auth --no-pager --full
curl -fsS http://127.0.0.1:8082/api/health | jq
```

- `curl -f` fail trên HTTP error.
- `-sS` ẩn progress nhưng in lỗi.
- `jq` format JSON.

## 18. Bật provisioning

Files cần stage an toàn:

```text
devices.registry.enc
registry.key
thread-dataset.hex
webui-bff.cjs
```

Repository có `enable-hil-provisioning.sh`. Chạy sau khi review hard-coded source path:

```bash
sudo bash "$BBB_STAGE/enable-hil-provisioning.sh"
```

Script backup BFF/env, cài registry/key/dataset mode chặt, set env, add RPC group và restart Controller/BFF.

Kiểm permission:

```bash
sudo ls -l /etc/matter-provisioning
sudo namei -l /etc/matter-provisioning/registry.key
```

- `namei -l` cho permission từng parent/path component.
- Không dùng `cat` với key/dataset.

Tắt provisioning nhưng giữ WebUI:

```bash
sudo sed -i 's/^PROVISIONING_ENABLED=.*/PROVISIONING_ENABLED=false/' /etc/matter-web-auth/webui.env
sudo systemctl restart matter-web-auth
```

- `sed -i` sửa file tại chỗ.
- Tắt route commissioning, không xóa registry/state.

Bật lại:

```bash
sudo sed -i 's/^PROVISIONING_ENABLED=.*/PROVISIONING_ENABLED=true/' /etc/matter-web-auth/webui.env
sudo systemctl restart matter-web-auth
```

## 19. Start/stop toàn stack

Start theo dependency:

```bash
sudo systemctl start mosquitto
sudo systemctl start otbr-agent
sudo systemctl start matter-controller
sudo systemctl start matter-gateway
sudo systemctl start matter-web-auth
```

Stop theo thứ tự ngược:

```bash
sudo systemctl stop matter-web-auth
sudo systemctl stop matter-gateway
sudo systemctl stop matter-controller
sudo systemctl stop otbr-agent
sudo systemctl stop mosquitto
```

Không cần stop OTBR/Mosquitto khi chỉ update WebUI.

Enable boot toàn stack:

```bash
sudo systemctl enable mosquitto otbr-agent matter-controller matter-gateway matter-web-auth
```

Disable boot:

```bash
sudo systemctl disable matter-web-auth matter-gateway matter-controller
```

Giữ OTBR/Mosquitto enabled nếu các workload khác cần chúng.

Kiểm tra đồng thời:

```bash
systemctl is-active mosquitto otbr-agent matter-controller matter-gateway matter-web-auth
systemctl --failed
```

`systemctl --failed` liệt kê unit fail toàn hệ thống.

## 20. Journal và log

100 dòng gần nhất:

```bash
sudo journalctl -u matter-controller -u matter-gateway -u matter-web-auth -n 100 --no-pager
```

Theo dõi realtime nhiều service:

```bash
sudo journalctl -f -u otbr-agent -u matter-controller -u matter-gateway -u matter-web-auth
```

Theo thời gian:

```bash
sudo journalctl -u matter-controller --since '10 minutes ago' --no-pager
```

Không copy credential/grant/dataset nếu log vô tình chứa chúng; redact trước chia sẻ.

## 21. Health checklist hàng ngày

```bash
systemctl is-active mosquitto otbr-agent matter-controller matter-gateway matter-web-auth
sudo ot-ctl state
ip -6 addr show wpan0
ip -6 route
curl -fsS http://127.0.0.1:8082/api/health | jq
printf '%s\n' '{"id":"daily-health","method":"health"}' | \
  sudo -u matter-gateway socat - UNIX-CONNECT:/run/matter-controller/controller.sock | jq
```

Expected:

- Services `active`.
- OTBR `leader` hoặc `router`.
- `wpan0` có IPv6.
- BFF `ok=true`, MQTT connected.
- Controller `ready=true` và node list hợp lý.

## 22. Backup

Tạo directory backup root-only:

```bash
sudo install -d -m 0700 /var/backups/agile-smart-device
```

Backup config/state metadata:

```bash
sudo tar -C / -czf "/var/backups/agile-smart-device/bbb-$(date +%Y%m%d-%H%M%S).tar.gz" \
  etc/matter-gateway \
  etc/matter-web-auth \
  etc/matter-provisioning \
  etc/systemd/system/matter-controller.service \
  var/lib/matter-controller \
  var/lib/matter-web-auth
```

- `tar -C /` làm paths trong archive relative.
- `-c` create, `-z` gzip, `-f` output file.
- Archive chứa secret; giữ mode 0600/root-only, mã hóa khi chuyển máy.

Kiểm archive mà không extract:

```bash
sudo tar -tzf /var/backups/agile-smart-device/<backup>.tar.gz
```

`-t` list content.

## 23. Rollback bundle

Trước deploy:

```bash
sudo cp -a /opt/matter-controller/matter-controller.cjs \
  /opt/matter-controller/matter-controller.cjs.pre-update
sudo cp -a /opt/matter-gateway/gateway.cjs \
  /opt/matter-gateway/gateway.cjs.pre-update
sudo cp -a /opt/matter-web-auth/webui-bff.cjs \
  /opt/matter-web-auth/webui-bff.cjs.pre-update
```

`cp -a` giữ mode/owner/timestamps.

Rollback:

```bash
sudo cp -a /opt/matter-controller/matter-controller.cjs.pre-update \
  /opt/matter-controller/matter-controller.cjs
sudo systemctl restart matter-controller
sudo systemctl restart matter-gateway matter-web-auth
```

Đợi controller socket ready trước restart Gateway nếu BBB chậm.

## 24. Troubleshooting

### Gateway restart loop `ENOENT controller.sock`

```bash
sudo systemctl status matter-controller --no-pager
sudo test -S /run/matter-controller/controller.sock && echo ready
```

Controller chưa ready. Không đổi về mock để che lỗi; đợi/fix controller rồi restart Gateway.

### `Unknown RPC method commissionOnNetwork`

Deployed controller bundle cũ. Build/deploy bundle từ source có method, restart và probe RPC compatibility.

### Controller active nhưng CPU cao/socket chậm

BBB có thể load/migrate persistent Matter state. Theo dõi `ps`, journal và chờ bounded thời gian; rollback nếu socket không xuất hiện.

```bash
ps -p "$(systemctl show -p MainPID --value matter-controller)" -o pid,etime,pcpu,pmem,stat,cmd
```

### Android `ENETUNREACH`

Kiểm OMR route table 1020; cập nhật radvd client link-local và restart radvd.

### Node attach nhưng controller không discover

Kiểm Android/BBB route, mDNS/SRP log, ECW timeout và discriminator/passcode. Không paste dataset.

### MQTT command timeout

Theo thứ tự:

1. Mosquitto active/ACL.
2. Gateway connected.
3. Controller socket ready.
4. Node commissioned/CASE reachable.
5. Endpoint/cluster đúng.

### WebUI login được nhưng không có device

Kiểm `/api/devices`, controller `listNodes`, BFF provisioning metadata và browser SSE.

## 25. Security tối thiểu

- SSH key, tắt password login khi vận hành ổn định.
- MQTT 1883 chỉ loopback.
- BFF bind loopback sau HTTPS reverse proxy/tunnel.
- Env/registry/key/dataset mode 0400/0600 phù hợp.
- Mỗi service có system user riêng và systemd sandbox.
- Không expose controller Unix socket qua TCP.
- Không log Thread dataset, claim secret, passcode, bearer/token.
- Backup chứa secret phải mã hóa và kiểm soát access.

## 26. Quy trình thêm node chuẩn tóm tắt

```text
1. systemctl health pass
2. ot-ctl state = leader/router
3. Android OMR route tồn tại
4. Node claim window active
5. Mobile claim + BLE PASE + Thread
6. Mobile ECW
7. BFF -> commissionOnNetwork
8. Controller describe/read/subscribe
9. Mobile RemoveFabric
10. /api/devices + OnOff test
11. Restart recovery test
```

Không đánh commissioning thành công nếu chỉ thấy node attach Thread; phải đạt permanent BBB fabric, cleanup và điều khiển được.