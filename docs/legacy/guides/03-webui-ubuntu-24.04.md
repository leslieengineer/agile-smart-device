# Tiêu chuẩn 3 — WebUI trên Ubuntu 24.04

Tài liệu này hướng dẫn build/test/chạy WebUI cùng BFF trên Ubuntu 24.04. WebUI không kết nối trực tiếp Matter hoặc giữ MQTT credential; mọi command/inventory/realtime đi qua BFF.

## 1. Thành phần

- `apps/webui` — Vue/Vite/Tailwind UI.
- `packages/webui-bff` — HTTP auth, REST, SSE và static file server.
- `packages/contracts` — schema dùng chung.
- `packages/gateway` — MQTT → Matter Controller RPC.
- `packages/matter-controller` — Matter.js controller.
- `packages/provisioning` — claim/grant/transaction.

Chạy UI đơn lẻ chỉ kiểm tra giao diện. Muốn login, inventory và command phải chạy BFF, MQTT và Gateway/Controller hoặc dùng integration backend tương ứng.

## 2. Biến môi trường

```bash
export PROJECT_DIR="$HOME/WS/agile-smart-device"
export DASHBOARD_DIR="$PROJECT_DIR/dashboard-reference"
export WEBUI_HOST="127.0.0.1"
export WEBUI_PORT="8082"
export MQTT_URL="mqtt://127.0.0.1:1883"
```

- `DASHBOARD_DIR` là npm workspace dashboard.
- `WEBUI_HOST` bind local-only cho môi trường phát triển.
- `WEBUI_PORT` là port BFF/static WebUI.
- `MQTT_URL` là broker mà BFF dùng để gửi command.

## 3. Cài package Ubuntu

```bash
sudo apt update
sudo apt install -y git curl build-essential python3 mosquitto mosquitto-clients
```

Ý nghĩa:

- `build-essential` hỗ trợ dependency native nếu npm cần compile.
- `mosquitto` là MQTT broker local.
- `mosquitto-clients` cung cấp `mosquitto_pub/sub` để kiểm tra broker.

Cài Node 20 bằng NVM:

```bash
curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.40.3/install.sh | bash
export NVM_DIR="$HOME/.nvm"
source "$NVM_DIR/nvm.sh"
nvm install 20
nvm use 20
nvm alias default 20
```

Project yêu cầu Node `>=20.11`.

Kiểm tra:

```bash
node --version
npm --version
mosquitto -h | sed -n '1,3p'
```

`sed -n '1,3p'` chỉ in ba dòng đầu của help để xác nhận binary.

## 4. Cài npm dependency

```bash
cd "$DASHBOARD_DIR"
npm install
```

`npm install` cài dependency cho toàn bộ workspace theo package-lock. Trong CI dùng:

```bash
npm ci
```

`npm ci` tạo install tái lập và fail nếu lockfile không khớp.

## 5. Typecheck, test và build

Typecheck:

```bash
npm run typecheck
```

Script chạy `typecheck` ở từng workspace có khai báo.

Tests one-shot:

```bash
npm test -- --run
```

Vitest chạy contracts, provisioning, auth, Gateway, MQTT integration và WebUI component tests.

Watch mode khi phát triển:

```bash
npm run test:watch
```

Vitest giữ process, theo dõi file và chỉ rerun test liên quan. Thoát bằng `q` hoặc `Ctrl+C`.

Build toàn bộ:

```bash
npm run build
```

Thứ tự build:

1. contracts,
2. provisioning,
3. Matter Controller,
4. BFF,
5. Gateway,
6. WebUI.

WebUI output:

```bash
ls -lh apps/webui/dist
```

`dist/index.html` và `dist/assets/` là static artifact.

## 6. Chạy Vite dev server

```bash
npm run dev:webui -- --host 127.0.0.1
```

- Gọi Vite của `@agile/webui`.
- Port source config là 5173.
- `127.0.0.1` chỉ cho truy cập local.

Mở:

```text
http://127.0.0.1:5173
```

Vite config hiện không có `/api` proxy. Vì API client dùng relative `/api`, dev server này phù hợp kiểm tra UI/component; integration đầy đủ nên để BFF phục vụ built dist hoặc đặt reverse proxy dev rõ ràng.

Expose LAN tạm thời:

```bash
npm run dev:webui -- --host 0.0.0.0
```

`0.0.0.0` cho máy khác truy cập; chỉ dùng LAN tin cậy và không coi đây là production server.

## 7. Chạy Mosquitto local an toàn

Bật service:

```bash
sudo systemctl enable --now mosquitto
```

- `enable` tạo boot-time link.
- `--now` start ngay.

Kiểm tra:

```bash
systemctl is-active mosquitto
systemctl is-enabled mosquitto
ss -ltnp | grep 1883
```

- `is-active` kiểm tra runtime.
- `is-enabled` kiểm tra boot policy.
- `ss -ltnp` xem TCP listener/process; port 1883 là MQTT TCP.

Broker package mặc định có thể không cho anonymous remote. Production phải dùng password/ACL; xem BBB guide.

## 8. Tạo password hash cho BFF

Sau `npm run build`:

```bash
read -rsp 'Web admin password: ' WEB_ADMIN_PASSWORD
printf '\n'
WEBUI_ADMIN_PASSWORD_HASH="$(printf '%s' "$WEB_ADMIN_PASSWORD" | node packages/webui-bff/dist/main.js hash-password)"
unset WEB_ADMIN_PASSWORD
```

Ý nghĩa:

- `read -s` không echo password; `-p` hiển thị prompt; `-r` không xử lý backslash.
- `printf` đưa password qua stdin, không qua command argument.
- `$(...)` lấy stdout hash vào biến.
- `unset` xóa plaintext khỏi shell variable.

Không in/hash vào Git.

## 9. Chạy BFF local với built WebUI

Tạo state directory local:

```bash
mkdir -p "$DASHBOARD_DIR/.local-state/webui"
```

Export config không secret trước:

```bash
export WEBUI_BIND="127.0.0.1"
export WEBUI_PORT="8082"
export WEBUI_PUBLIC_ORIGIN="http://127.0.0.1:8082"
export MOBILE_ALLOWED_ORIGINS="https://localhost"
export WEBUI_ROOT="$DASHBOARD_DIR/apps/webui/dist"
export WEBUI_ADMIN_USERNAME="admin"
export WEBUI_SESSION_TTL_S="604800"
export WEBUI_SESSION_IDLE_S="86400"
export WEBUI_STATE_DIR="$DASHBOARD_DIR/.local-state/webui"
export MQTT_URL="mqtt://127.0.0.1:1883"
export MQTT_USERNAME="<local-mqtt-user>"
export MQTT_PASSWORD="<load-from-secure-local-file>"
export PROVISIONING_ENABLED="false"
export LOG_LEVEL="debug"
```

- `WEBUI_PUBLIC_ORIGIN` phải khớp Origin/Host mà browser dùng.
- `WEBUI_ROOT` trỏ built static assets.
- session TTL/idle tính bằng giây.
- provisioning tắt khi local machine không có registry/dataset/controller socket.

Export hash:

```bash
export WEBUI_ADMIN_PASSWORD_HASH="$WEBUI_ADMIN_PASSWORD_HASH"
```

Chạy BFF:

```bash
node packages/webui-bff/dist/main.js
```

Process bind port 8082, serve REST/SSE và static WebUI. Dừng bằng `Ctrl+C`.

Mở:

```text
http://127.0.0.1:8082
```

Lưu ý cookie production dùng `Secure`; HTTP local có thể không tái hiện đầy đủ production auth. Dùng HTTPS reverse proxy khi test cookie policy chính xác.

## 10. Chạy Gateway local

Gateway cần MQTT và Controller implementation. Mock mode chỉ dùng phát triển contract:

```bash
export MQTT_URL="mqtt://127.0.0.1:1883"
export MQTT_USERNAME="<local-mqtt-user>"
export MQTT_PASSWORD="<local-mqtt-password>"
export MQTT_CLIENT_ID="matter-gateway-dev"
export MQTT_TX_TOPIC="home/control/tx"
export MQTT_RX_TOPIC="home/control/rx"
export CONTROLLER_MODE="mock"
export MOCK_LATENCY_MS="30"
export LOG_LEVEL="debug"
npm run dev:gateway
```

- `CONTROLLER_MODE=mock` không điều khiển hardware.
- `MOCK_LATENCY_MS` mô phỏng độ trễ.
- Dev script dùng watcher TypeScript.

Matter.js mode cần Unix socket và controller process; dùng BBB guide thay vì tạo socket giả trên workstation.

## 11. Kiểm tra API và static assets

Health:

```bash
curl -i http://127.0.0.1:8082/api/health
```

- `curl -i` in response headers và body.
- Expected HTTP 200 cùng `ok`, MQTT state và SSE client count.

Static index:

```bash
curl -I http://127.0.0.1:8082/
```

`-I` chỉ gửi HEAD, dùng kiểm status/content-type/cache header.

## 12. Kiểm tra MQTT thủ công

Terminal 1:

```bash
mosquitto_sub -h 127.0.0.1 -p 1883 \
  -u '<user>' -P '<password>' \
  -t 'home/control/#' -v
```

- `-h/-p` broker host/port.
- `-u/-P` auth; tránh đặt password thật vào shared shell history.
- `-t` topic filter; `#` là wildcard.
- `-v` in topic cùng payload.

Terminal 2 publish test status/non-production payload:

```bash
mosquitto_pub -h 127.0.0.1 -p 1883 \
  -u '<user>' -P '<password>' \
  -t 'home/control/tx' \
  -m '<VALID_TEST_ENVELOPE>'
```

`-m` là payload string. Không gửi command hardware nếu chưa chọn node/test environment.

## 13. WebUI flow

1. Login qua BFF.
2. BFF đặt cookie session và trả CSRF token.
3. WebUI gọi `/api/devices` để lấy inventory.
4. OnOff card gửi `/api/command`.
5. BFF publish MQTT; Gateway gọi Matter RPC.
6. response/event quay về SSE và cập nhật store.

Browser không nhập MQTT user/password.

## 14. Debug browser

Mở Developer Tools:

- Network → kiểm `/api/session`, `/api/devices`, `/api/events`.
- Console → kiểm JavaScript exception.
- Application → kiểm cookie attributes; không copy cookie vào ticket.

SSE bằng curl sau khi có auth không nên ghi token vào command history. Ưu tiên browser Network panel hoặc token file permission 0600.

## 15. Production build artifact

```bash
cd "$DASHBOARD_DIR"
npm ci
npm test -- --run
npm run build
```

Bundle BFF/Gateway/Controller bằng esbuild pin của workspace, ví dụ BFF:

```bash
npx esbuild packages/webui-bff/src/main.ts \
  --bundle \
  --platform=node \
  --format=cjs \
  --outfile=/tmp/webui-bff.cjs
```

- `--bundle` gom dependency JS.
- `--platform=node` giữ Node built-ins đúng.
- `--format=cjs` tạo artifact chạy bằng Node service hiện tại.
- `--outfile` chọn staged artifact, không ghi đè source.

Matter Controller có optional `bun:sqlite`; bundle command phải external module này nếu esbuild yêu cầu:

```bash
npx esbuild packages/matter-controller/src/main.ts \
  --bundle --platform=node --format=cjs \
  --external:bun:sqlite \
  --outfile=/tmp/matter-controller.cjs
```

## 16. Lỗi thường gặp

### `/api` trả 404 ở Vite port 5173

Vite không proxy API. Dùng BFF port 8082 với built dist hoặc cấu hình reverse proxy dev có chủ đích.

### Login loop/cookie không lưu

Kiểm tra HTTPS, `WEBUI_PUBLIC_ORIGIN`, Host và Secure cookie. HTTP localhost không giống public HTTPS.

### UI không có device

Kiểm `/api/devices`, controller node list và provisioning metadata. Không hard-code node vào UI.

### Command timeout

Kiểm MQTT, Gateway log, controller socket và node CASE connectivity theo thứ tự.

### SSE mất kết nối

Kiểm `/api/events`, bearer/cookie expiry, reverse proxy buffering và BFF ping. Client phải reconcile REST sau reconnect.

## 17. Validation cuối

```bash
cd "$DASHBOARD_DIR"
npm run typecheck
npm test -- --run
npm run build
curl -fsS http://127.0.0.1:8082/api/health
```

`curl -f` fail khi HTTP >=400, `-sS` giảm progress nhưng vẫn in error. WebUI chỉ đạt HIL khi inventory node thật và On/Off hai chiều hoạt động qua permanent BBB fabric.