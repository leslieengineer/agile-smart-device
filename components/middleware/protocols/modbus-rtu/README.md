# Modbus RTU Sample

`ModbusRtuMaster::send_request()` accepts a slave address and a protocol data unit (PDU). The PDU begins with the Modbus function code. The component prepends the slave address before passing the frame to `IUart`.

The sample intentionally stops before CRC generation and response parsing. Add those responsibilities here as the next steps; do not put them in an UART adapter.
