# UHAL Core

`components/uhal/core` contains small platform-neutral value types shared by all components. The sample `Status.hpp` defines transport outcomes such as `timeout` and `nack` without exposing an SDK error code.

Keep this layer small. Add types such as `Result<T>` or a fixed-capacity byte buffer here only when multiple interface or logic components need them.
