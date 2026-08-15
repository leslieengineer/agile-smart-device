# Framework UHAL Core Bridge

This header-only ESP-IDF component exposes the framework UHAL core without leaking the physical submodule path into product components.

Create separate bridge components for additional reusable capabilities, such as `framework_uhal_interfaces` or `framework_sht3x`, instead of adding unrelated include paths here.
