# Smart Device Product

This ESP-IDF component owns the Layer 5 composition root for the smart-device firmware.

Dependency direction:

```text
main - - - submodule
```

Board configuration and product-specific services belong in this repository. Reusable devices, protocols, services, and libraries remain in `external/agile-firmware-framework` and are imported through dedicated bridge components.
