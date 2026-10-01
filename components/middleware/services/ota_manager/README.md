# OTA Service

Non-blocking signed-image update policy over injected fetcher, firmware-store, digest, signature-verifier, and boot-control ports. Chunks are fixed at 1024 bytes. Unsupported signature verification is denied; no unsigned activation path exists. ESP-IDF OTA, TLS, crypto and partition implementations remain product adapters.
