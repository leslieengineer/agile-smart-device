# Build, flash và deploy

## Trạng thái

| Hạng mục | Source | Deployed | HIL |
|---|---|---|---|
| Firmware build | Có | Có | Build gate |
| Dashboard build | Có | Có | Automated tests |
| Android native build | Có | Có | Native compile/install |
| Release deploy | Một phần | Lab | Chưa |

## Trạng thái toolchain

| Target | Source pin |
|---|---|
| Firmware | `tools/build_matter_node.sh` |
| ESP-IDF | SHA được script kiểm tra dưới `${HOME}/esp/v6.0.2/esp-idf` |
| ESP-Matter | SHA được script kiểm tra dưới `${HOME}/esp-matter` |
| Android CHIP | `mobileapp-reference/tools/android/chip.lock.json` |
| Node workspace | lockfile của từng reference workspace |

Không chép version từ tài liệu legacy; kiểm tra script/lockfile trước build.

## Build firmware

```bash
python3 tools/check_layer_boundaries.py
cmake -S tests/host -B build/host-tests -G Ninja
cmake --build build/host-tests
ctest --test-dir build/host-tests --output-on-failure
bash tools/build_matter_node.sh
```

Matter build dùng `sdkconfig.defaults.matter_node`, profile `matter_node` và mặc định tắt `RHOPHI_CLAIM_DEV_BYPASS`.

## Build dashboard

```bash
cd dashboard-reference
npm install
npm test -- --run
npm run build
```

## Build Android

```bash
cd mobileapp-reference
npm install
npm test -- --run
npm run build
npm run android:sync -w @rhophi/mobile
JAVA_HOME=<JDK17_OR_COMPATIBLE> ./apps/mobile/android/gradlew -p apps/mobile/android assembleDebug
```

ConnectedHomeIP JAR/JNI phải được stage bằng script pin trước khi Gradle build native plugin.

## Artifact

- Firmware ELF/bin trong `build-matter/`.
- WebUI dist và Node bundles được stage ngoài source tree trước deploy.
- Debug APK tại `apps/mobile/android/app/build/outputs/apk/debug/app-debug.apk`.

## Thao tác thay đổi trạng thái

Flash, erase, factory reset, cài APK và systemd deploy nằm trong [runbooks](runbooks/README.md). Luôn xác minh port/host, backup state cần giữ và có rollback trước thao tác.

## Giới hạn

Build pass không tương đương HIL pass. Generated artifact không phải source of truth và không nên commit trừ khi release process yêu cầu.