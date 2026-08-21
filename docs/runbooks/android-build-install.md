# Android build và install

## Tiền điều kiện

- Android SDK/platform/NDK và JDK theo `tools/android/chip.lock.json`.
- ConnectedHomeIP checkout đúng commit.
- Điện thoại đã authorize ADB.

## Build CHIP artifact

```bash
cd "${PROJECT_DIR}/mobileapp-reference"
export ANDROID_HOME="${HOME}/Android/Sdk"
export ANDROID_NDK_HOME="${ANDROID_HOME}/ndk/<pinned-version>"
export JAVA_HOME="<jdk-path>"
bash tools/android/build-chip-controller.sh "${HOME}/.cache/rhophi-chip-controller/<commit>/arm64-v8a"
```

Artifact phải có `SHA256SUMS`; Gradle không được fallback mock ngoài build được chọn rõ.

## Build app

```bash
npm install
npm test -- --run
npm run build
npm run android:sync -w @rhophi/mobile
JAVA_HOME="${JAVA_HOME}" ./apps/mobile/android/gradlew -p apps/mobile/android assembleDebug
```

## Install

```bash
adb install -r apps/mobile/android/app/build/outputs/apk/debug/app-debug.apk
adb shell am start -n uk.rhophi.mobile/.MainActivity
```

Signature mismatch yêu cầu uninstall, nhưng uninstall xóa app data/token/controller storage; chỉ làm sau khi xác nhận.

## Log

```bash
adb logcat -v threadtime '*:S' RhophiCommissioning:V RhophiGatt:V BluetoothGatt:V AndroidRuntime:E Capacitor:V
```

Không lưu dòng có bearer, grant hoặc ciphertext vào evidence.