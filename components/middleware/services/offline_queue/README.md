# Offline Queue

Fixed 32-record store-and-forward queue. Each record owns a 96-byte topic and 256-byte payload, so the default RAM budget is approximately 11.5 KiB. Full telemetry drops the oldest record and increments a counter; command responses return `no_resources`.
