# Binary Switch Service

`BinarySwitchService` is a Layer 4 policy for a persisted on/off load with a mirrored indicator. It supports restore, explicit set, toggle, and state queries through injected UHAL GPIO capabilities.

The service owns the `IBinaryStateStore` port because restore and persistence ordering are part of the policy. Products provide the storage adapter and retain ownership of storage media, namespaces, keys, schemas, scheduling, and board wiring.

The service depends only on `uhal::IGpio` and `uhal::Status`. It does not include vendor SDK, RTOS, board, or storage implementation headers.

| Failure | Behavior |
|---|---|
| Load fails during restore | Defaults to OFF, applies OFF, and returns the apply status |
| Either output apply fails | Keeps the requested runtime state, skips save, and returns `io_error` |
| Save fails | Keeps the applied runtime and output state and returns the store status |
