# Tiêu chuẩn 2 — Mobile App trên Ubuntu 24.04

Tài liệu này hướng dẫn setup Node.js, Android SDK, ConnectedHomeIP native artifact, build/test Ionic Vue, build APK, cài lên Android và theo dõi commissioning.

## 1. Kiến trúc cần hiểu trước

- `packages/client-sdk` — REST/SSE/contracts/state machine.
- `packages/commissioning-plugin` — Capacitor definitions và Kotlin native plugin.
- `apps/mobile` — Ionic Vue UI/Pinia.
- `tools/android` — pin và build ConnectedHomeIP JAR/JNI.

Browser dev server chỉ kiểm tra UI/API. BLE PASE/Thread commissioning chỉ chạy trong APK native.

## 2. Biến môi trường

```bash
export PROJECT_DIR="$HOME/WS/agile-smart-device"
export MOBILE_DIR="$PROJECT_DIR/mobileapp-reference"
export ANDROID_HOME="$HOME/Android/Sdk"
export ANDROID_NDK_HOME="$ANDROID_HOME/ndk/28.2.13676358"
export JAVA_HOME="/usr/lib/jvm/java-17-openjdk-amd64"
export PATH="$ANDROID_HOME/platform-tools:$ANDROID_HOME/cmdline-tools/latest/bin:$PATH"
```

Ý nghĩa:

- `MOBILE_DIR` là npm workspace mobile.
- `ANDROID_HOME` là Android SDK root.
- `ANDROID_NDK_HOME` pin NDK dùng build JNI.
- `JAVA_HOME` chọn JDK cho Gradle/ConnectedHomeIP.
- Dòng `PATH` cho phép gọi `adb` và `sdkmanager` không cần absolute path.

## 3. Cài package cơ bản

```bash
sudo apt update
sudo apt install -y git curl unzip zip xz-utils build-essential \
  openjdk-17-jdk python3 python3-venv
```

- `build-essential` cài GCC/G++/make cho native host tools.
- `openjdk-17-jdk` cài Java compiler/runtime.
- `unzip`, `zip`, `xz-utils` giải nén Android/CHIP dependencies.

Kiểm tra Java:

```bash
java -version
javac -version
printf 'JAVA_HOME=%s\n' "$JAVA_HOME"
```

Nếu `java` không dùng JDK17, gọi Gradle với `JAVA_HOME="$JAVA_HOME"` như các lệnh dưới.

## 4. Cài Node.js 20

Cách dùng NVM, không sửa Node hệ thống:

```bash
curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.40.3/install.sh | bash
export NVM_DIR="$HOME/.nvm"
source "$NVM_DIR/nvm.sh"
nvm install 20
nvm use 20
nvm alias default 20
```

Ý nghĩa:

- `curl -o-` tải script ra stdout.
- `| bash` chạy installer; chỉ dùng URL chính thức và review khi policy yêu cầu.
- `source nvm.sh` nạp function `nvm` vào shell hiện tại.
- `nvm install 20` cài Node major 20.
- `nvm alias default 20` chọn mặc định cho terminal mới.

Kiểm tra:

```bash
node --version
npm --version
```

Project yêu cầu Node `>=20.11`.

## 5. Cài Android command-line tools

Tạo SDK directory:

```bash
mkdir -p "$ANDROID_HOME/cmdline-tools"
```

Tải Android command-line tools từ trang Android Developer rồi giải nén sao cho tồn tại:

```text
$ANDROID_HOME/cmdline-tools/latest/bin/sdkmanager
```

Không hard-code URL archive cũ trong automation dài hạn. Sau khi cài:

```bash
yes | sdkmanager --licenses
sdkmanager \
  "platform-tools" \
  "platforms;android-35" \
  "platforms;android-34" \
  "build-tools;35.0.0" \
  "ndk;28.2.13676358"
```

Ý nghĩa:

- `yes | ... --licenses` gửi `y` cho Android SDK licenses; đọc license theo policy tổ chức trước khi chấp nhận.
- `platform-tools` cung cấp ADB.
- `platforms;android-35` khớp compile SDK app/plugin.
- `android-34` cần cho ConnectedHomeIP build script hiện tại.
- `build-tools` cung cấp aapt2/d8/apksigner.
- NDK pin cung cấp clang/sysroot cho `arm64-v8a` JNI.

Kiểm tra:

```bash
sdkmanager --list_installed
adb version
ls "$ANDROID_NDK_HOME"
```

## 6. Chuẩn bị thiết bị Android

Trên điện thoại:

1. Bật Developer options.
2. Bật USB debugging.
3. Cắm cáp data và chấp nhận RSA prompt.

Trên Ubuntu:

```bash
adb kill-server
adb start-server
adb devices -l
```

- `kill-server` dừng ADB daemon cũ.
- `start-server` tạo daemon mới.
- `devices -l` phải hiện trạng thái `device`, không phải `unauthorized`.

Chọn device khi có nhiều điện thoại:

```bash
export ANDROID_SERIAL="<serial-from-adb-devices>"
adb -s "$ANDROID_SERIAL" get-state
```

`-s` khóa mọi lệnh sau vào đúng thiết bị.

## 7. Cài dependency npm

```bash
cd "$MOBILE_DIR"
npm install
```

`npm install` đọc workspace/package-lock, cài dependency vào `node_modules` và chạy lifecycle scripts được cho phép. Trong CI nên dùng:

```bash
npm ci
```

`npm ci` yêu cầu lockfile khớp package manifest, xóa node_modules cũ và tạo install tái lập.

Kiểm tra workspace:

```bash
npm query .workspace
```

Lệnh liệt kê packages mà npm nhận diện trong workspace.

## 8. Typecheck, test và build web assets

Typecheck:

```bash
npm run typecheck
```

Script build client SDK trước rồi chạy typecheck ở workspace có khai báo.

Tests:

```bash
npm test -- --run
```

- `npm test` gọi Vitest.
- `--` chuyển tham số sau nó vào script.
- `--run` ép one-shot, không watch.

Build tất cả:

```bash
npm run build
```

Thứ tự là client SDK → commissioning plugin TypeScript → mobile Vue/Vite. Output web ở `apps/mobile/dist`.

## 9. Chạy UI development server

```bash
npm run dev:mobile -- --host 127.0.0.1
```

- `dev:mobile` gọi Vite của app.
- `--host 127.0.0.1` chỉ bind local machine, an toàn hơn `0.0.0.0`.

Nếu cần test từ điện thoại cùng LAN:

```bash
npm run dev:mobile -- --host 0.0.0.0
```

`0.0.0.0` expose server trên mọi interface; chỉ dùng LAN tin cậy và firewall phù hợp. Native BLE plugin không chạy trong browser.

## 10. Cấu hình API

Tạo env local trong `apps/mobile` theo convention Vite, ví dụ:

```bash
cd "$MOBILE_DIR/apps/mobile"
printf 'VITE_API_BASE_URL=%s\n' 'https://<bbb-or-public-bff-host>' > .env.local
```

- `VITE_API_BASE_URL` là BFF base URL.
- `.env.local` không được commit nếu chứa môi trường riêng.
- Không đặt password/token trong biến `VITE_*` vì Vite bundle chúng vào client.

## 11. Chuẩn bị ConnectedHomeIP Android artifact

Kiểm tra pin:

```bash
cd "$MOBILE_DIR"
cat tools/android/chip.lock.json
git -C "$HOME/esp-matter/connectedhomeip/connectedhomeip" rev-parse HEAD
```

`chip.lock.json` là expected commit/SDK/NDK/JDK/ABI. SHA thực tế phải khớp.

Build/stage:

```bash
export CHIP_ROOT="$HOME/esp-matter/connectedhomeip/connectedhomeip"
export RHOPHI_CHIP_ANDROID_DIR="$HOME/.cache/rhophi-chip-controller/efefc94f/arm64-v8a"
bash tools/android/build-chip-controller.sh "$RHOPHI_CHIP_ANDROID_DIR"
```

Script:

1. Kiểm tra ConnectedHomeIP commit.
2. Kiểm tra Android platform/NDK/JDK.
3. Build `android-arm64-chip-tool`.
4. Copy JAR và `libCHIPController.so`/`libc++_shared.so`.
5. Tạo `SHA256SUMS`.

Kiểm tra artifact:

```bash
find "$RHOPHI_CHIP_ANDROID_DIR" -maxdepth 3 -type f -print
sha256sum -c "$RHOPHI_CHIP_ANDROID_DIR/SHA256SUMS"
```

- `find` liệt kê artifact staged.
- `sha256sum -c` xác minh file không đổi sau staging.

Gradle property phải trỏ đúng directory này. Không bật `RHOPHI_MOCK_ONLY=true` cho APK commissioning thật.

## 12. Đồng bộ Capacitor Android project

```bash
cd "$MOBILE_DIR"
npm run build
npm run android:sync
```

`android:sync` thực hiện:

- Copy `apps/mobile/dist` vào Android assets.
- Cập nhật Capacitor plugins.
- Sinh config Android từ `capacitor.config`.

Luôn build web assets trước sync; nếu không APK có JavaScript cũ.

## 13. Build APK

```bash
cd "$MOBILE_DIR"
JAVA_HOME="$JAVA_HOME" \
  ./apps/mobile/android/gradlew -p apps/mobile/android assembleDebug
```

Ý nghĩa:

- `gradlew` dùng Gradle Wrapper của repository.
- `-p apps/mobile/android` chọn Android project directory.
- `assembleDebug` compile Kotlin/Java, merge manifest/JNI/assets, dex và ký debug APK.

APK:

```bash
ls -lh apps/mobile/android/app/build/outputs/apk/debug/app-debug.apk
```

Build sạch khi Gradle cache lỗi:

```bash
JAVA_HOME="$JAVA_HOME" ./apps/mobile/android/gradlew -p apps/mobile/android clean assembleDebug
```

`clean` xóa Android build outputs; không xóa npm dependencies hoặc app data trên phone.

## 14. Cài và chạy APK

```bash
adb install -r apps/mobile/android/app/build/outputs/apk/debug/app-debug.apk
```

- `install` gửi APK qua ADB.
- `-r` replace package hiện có và giữ app data nếu signature giống.

Nếu báo `INSTALL_FAILED_UPDATE_INCOMPATIBLE`, APK cũ có signature khác. Backup cần thiết rồi:

```bash
adb uninstall uk.rhophi.mobile
adb install apps/mobile/android/app/build/outputs/apk/debug/app-debug.apk
```

`uninstall` xóa app data, login token và CHIP fabric storage; đây là thao tác destructive.

Khởi chạy:

```bash
adb shell am start -n uk.rhophi.mobile/.MainActivity
```

- `adb shell` chạy command trên Android.
- `am start` gửi Activity Manager intent.
- `-n` chỉ component package/activity.

Dừng app:

```bash
adb shell am force-stop uk.rhophi.mobile
```

`force-stop` dừng process nhưng giữ app data.

## 15. Logcat tiêu chuẩn

Xóa buffer trước test:

```bash
adb logcat -c
```

Theo dõi log cần thiết:

```bash
adb logcat -v threadtime \
  '*:S' \
  RhophiCommissioning:V \
  RhophiGatt:V \
  BluetoothGatt:V \
  AndroidRuntime:E \
  Capacitor:V
```

- `-v threadtime` thêm timestamp/PID/TID.
- `'*:S'` tắt mọi tag mặc định.
- `Tag:V` bật verbose cho tag cần xem.
- `AndroidRuntime:E` chỉ hiện crash/error.

Không lưu dòng có bearer token, grant/ciphertext, challenge/proof hoặc dataset vào evidence.

## 16. Quy trình commissioning chuẩn

1. Đảm bảo BBB Controller RPC ready.
2. Đảm bảo Android có IPv6 route tới Thread OMR.
3. Mở Add Device và scan.
4. Chọn node sau khi identity được đọc.
5. Bấm Commission một lần.
6. Quan sát PASE → attestation → Thread → operational discovery.
7. App mở ECW, BFF gọi BBB `commissionOnNetwork`.
8. BBB describe/read/subscribe.
9. App RemoveFabric temporary fabric.
10. Inventory xuất hiện và On/Off hoạt động.

Kiểm tra route:

```bash
adb shell ip -6 addr show wlan0
adb shell ip -6 route show table 1020
```

Route OMR phải tồn tại; nếu thiếu, `FindOperationalForStayActive` có thể trả `ENETUNREACH`.

## 17. Test retry/recovery

Reset UI transaction nhưng giữ controller storage qua nút Cancel/Retry trong app. Không xóa CHIP prefs khi temporary fabric cần cleanup.

Liệt kê SharedPreferences debug build:

```bash
adb shell run-as uk.rhophi.mobile ls shared_prefs
```

`run-as` chỉ hoạt động với debuggable app. Không xóa file nếu chưa hiểu fabric nào phụ thuộc nó.

## 18. Lỗi thường gặp

### `unauthorized`

Mở khóa phone, revoke USB debugging authorizations nếu cần, reconnect và accept RSA prompt.

### Commissioning plugin unavailable

APK đang dùng mock-only artifact hoặc chạy trong browser. Kiểm tra Gradle property và `SHA256SUMS`.

### Rhophi characteristic unavailable

Plugin sẽ refresh GATT cache và reconnect một lần. Xem tag `RhophiGatt` để so UUID thực tế.

### Treo tại attestation

`continueCommissioning` phải được post lên Android main thread. Dùng APK mới và xem tag `RhophiCommissioning`.

### Crash multicast lock

Manifest phải có `CHANGE_WIFI_MULTICAST_STATE`; rebuild/sync/install lại APK sau khi sửa manifest.

### `ENETUNREACH`

Phone thiếu route tới OMR. Sửa OTBR/LAN/radvd trước khi retry, không recommission liên tục.

### `Unknown RPC commissionOnNetwork`

BBB đang chạy controller bundle cũ. Verify RPC compatibility và deploy đúng bundle.

## 19. Validation cuối

```bash
cd "$MOBILE_DIR"
npm run typecheck
npm test -- --run
npm run build
npm run android:sync
JAVA_HOME="$JAVA_HOME" ./apps/mobile/android/gradlew -p apps/mobile/android assembleDebug
adb install -r apps/mobile/android/app/build/outputs/apk/debug/app-debug.apk
```

Chỉ đánh HIL pass sau permanent BBB fabric, temporary fabric cleanup, inventory và On/Off hai chiều.