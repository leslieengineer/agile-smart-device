# Configuration Service

Fixed-capacity key/value configuration policy over `uhal::IStorage`. It uses two 2304-byte A/B snapshots with sequence and CRC32 so a corrupt or interrupted new slot can fall back to the previous slot. RAM holds at most 24 records with 24-byte keys and 64-byte values.

Unknown keys return `not_found`, invalid/corrupt snapshots fall back to the valid slot or defaults, capacity exhaustion returns `no_resources`, and storage failures propagate without corrupting the current RAM table. Vendor NVS/flash details stay in the product storage adapter.
