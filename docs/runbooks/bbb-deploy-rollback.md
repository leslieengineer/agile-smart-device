# BBB deploy và rollback

## Preflight

```bash
ssh "${BBB_USER}@${BBB_HOST}" 'systemctl is-active mosquitto otbr-agent matter-controller matter-gateway matter-web-auth'
ssh "${BBB_USER}@${BBB_HOST}" 'ip -6 addr show wpan0'
```

Backup bundle, env và persistent state metadata trước cutover. Không copy registry key qua kênh không bảo mật.

## Build/stage

Build dashboard bằng `npm test -- --run && npm run build`. Bundle Node entrypoints bằng project-pinned bundler. Stage artifact vào operator directory; không build trực tiếp trong `/opt`.

## Install order

1. Matter Controller.
2. Đợi Unix socket và RPC `health.ready=true`.
3. Gateway với `CONTROLLER_MODE=matterjs`.
4. BFF/provisioning.
5. WebUI static dist.

Gateway không được start trước controller socket readiness.

## Provisioning files

Expected protected paths:

```text
/etc/matter-provisioning/devices.registry.enc
/etc/matter-provisioning/registry.key
/etc/matter-provisioning/thread-dataset.hex
/var/lib/matter-web-auth/provisioning-transactions.json
/run/matter-controller/controller.sock
```

## Verify

```bash
ssh "${BBB_USER}@${BBB_HOST}" 'systemctl is-active matter-controller matter-gateway matter-web-auth'
curl -fsS "https://<public-host>/api/health"
```

Chạy RPC health bằng user thuộc `matter-rpc`; `active` không chứng minh bundle hỗ trợ `commissionOnNetwork`.

## Rollback

Restore bundle/env backup, restart controller rồi gateway/BFF. Không xóa `/var/lib/matter-controller` hoặc Thread state. Rollback provisioning bằng `PROVISIONING_ENABLED=false` khi cần cô lập.