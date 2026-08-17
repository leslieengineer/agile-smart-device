# Rhophi manufacturing pipeline

All output from these tools is private production material and must be written outside every Git worktree.

1. Read the chip base MAC/eFuse identity from the physical device. Run `generate_device.py --serial <serial> --chip-mac <base-mac> --product-id <pid> --out <private-dir>` once per physical device. The tool derives a public per-chip `claim_id` from the MAC and generates a random 32-byte `claim_secret`; it emits a private device record and the `fctry/rhophi` NVS CSV consumed by firmware.
2. Run `build_factory_partition.py` with the pinned ESP-IDF `IDF_PATH` to create the factory NVS binary.
3. Use the separately reviewed Espressif `esp-matter-tools` manufacturing tool for DAC/PAI/Certification Declaration and secure-cert output. Set `ESP_MATTER_MFG_TOOL`; `build_attestation.py` refuses to invent or fall back to development attestation credentials.
4. Store a random 32-byte registry master key in the operator secret store. Run `export_registry.py --key <key-file> --out <devices.registry.enc> <device.json...>`.
5. Install the encrypted registry and key on BBB with owner-only mode `0400`. Install the active Thread Operational Dataset separately with mode `0400`.
6. Flashing factory/secure-cert partitions is a separate consequential action and requires explicit operator approval.

Never print, paste, commit, email, or place claim secrets, setup passcodes, DAC private keys, registry keys, or Thread Datasets in MQTT/log output. Test vectors are not production credentials.
