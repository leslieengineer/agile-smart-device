"""Record a Monitor Debug tab transport until the workflow is stopped."""

import asyncio
from typing import Any


def _first_value(*sources: dict[str, Any], keys: tuple[str, ...]) -> Any:
    for source in sources:
        for key in keys:
            value = source.get(key)
            if value is not None and value != "":
                return value
    return None


def _require_serial_settings(profile: dict[str, Any]) -> tuple[str, int]:
    tab = profile.get("tab") or {}
    serial = profile.get("serial") or {}
    transport = profile.get("transport") or {}
    port = _first_value(serial, transport, tab, profile, keys=("port", "path", "serialPort"))
    baud = _first_value(serial, transport, tab, profile, keys=("baudRate", "baud_rate", "baud"))
    if not isinstance(port, str) or not port.strip():
        raise ValueError("Monitor capture requires a serial port in capture_profile")
    try:
        baud_rate = int(baud)
    except (TypeError, ValueError) as error:
        raise ValueError("Monitor capture requires a valid baud rate in capture_profile") from error
    if baud_rate <= 0:
        raise ValueError("Monitor capture baud rate must be positive")
    return port.strip(), baud_rate


async def _capture_uart(port: str, baud_rate: int) -> None:
    while True:
        for line in await serial_read(port, timeout_seconds=1.0, baud_rate=baud_rate):
            print(line, flush=True)


def _publish_capture_if_supported() -> None:
    publish = globals().get("publish_capture")
    if callable(publish):
        publish()


def main() -> None:
    if not isinstance(capture_profile, dict):
        raise ValueError("capture_profile must be a dictionary")
    port, baud_rate = _require_serial_settings(capture_profile)
    try:
        print("__EMBEDDER_CAPTURE_READY__", flush=True)
        asyncio.run(_capture_uart(port, baud_rate))
    except KeyboardInterrupt:
        pass
    finally:
        _publish_capture_if_supported()


if __name__ == "__main__":
    main()
