#include <cstdio>
#include <cstring>
#include <services/EnvironmentMonitor.hpp>

namespace
{

bool is_due(std::uint32_t now_ms, std::uint32_t last_run_ms, std::uint32_t period_ms)
{
    return static_cast<std::uint32_t>(now_ms - last_run_ms) >= period_ms;
}

}  // namespace

namespace services
{

EnvironmentMonitor::EnvironmentMonitor(uhal::IUart& cli_uart, uhal::IGpio& status_led,
                                       uhal::IClock& clock, std::uint32_t random_seed)
    : cli_uart_(cli_uart), status_led_(status_led), clock_(clock), random_state_(random_seed)
{
}

void EnvironmentMonitor::run_once()
{
    const std::uint32_t now_ms = clock_.now_ms();

    if (is_due(now_ms, last_cli_ms_, kCliPeriodMs))
    {
        last_cli_ms_ = now_ms;
        run_cli_task();
    }
    if (is_due(now_ms, last_sensor_ms_, kSensorPeriodMs))
    {
        last_sensor_ms_ = now_ms;
        run_sensor_task();
    }
    if (telemetry_pending_ || is_due(now_ms, last_telemetry_ms_, kTelemetryPeriodMs))
    {
        last_telemetry_ms_ = now_ms;
        telemetry_pending_ = false;
        run_telemetry_task();
    }
}

void EnvironmentMonitor::run_cli_task()
{
    std::uint8_t received_character = 0U;
    std::size_t  received_count     = 0U;
    if (cli_uart_.read(&received_character, 1U, received_count, 0U) != uhal::Status::ok ||
        received_count == 0U)
    {
        return;
    }

    if (received_character == '\r' || received_character == '\n')
    {
        if (command_length_ > 0U)
        {
            command_[command_length_] = '\0';
            process_command();
            command_length_ = 0U;
        }
        return;
    }
    if (command_length_ + 1U >= kCommandCapacity)
    {
        command_length_ = 0U;
        write_text("ERR command too long\r\n");
        return;
    }
    command_[command_length_++] = static_cast<char>(received_character);
}

void EnvironmentMonitor::run_sensor_task()
{
    temperature_celsius_ = next_temperature_celsius();
    led_level_ = temperature_celsius_ > 25U ? uhal::GpioLevel::high : uhal::GpioLevel::low;
    static_cast<void>(status_led_.set(led_level_));
}

void EnvironmentMonitor::run_telemetry_task()
{
    char message[32]{};
    std::snprintf(message, sizeof(message), "Temperature: %u C\r\n",
                  static_cast<unsigned>(temperature_celsius_));
    write_text(message);
}

void EnvironmentMonitor::process_command()
{
    if (std::strcmp(command_, "help") == 0)
    {
        write_text("Commands: help, status\r\n");
        return;
    }
    if (std::strcmp(command_, "status") == 0)
    {
        char        response[64]{};
        const char* led_status = led_level_ == uhal::GpioLevel::high ? "ON" : "OFF";
        std::snprintf(response, sizeof(response), "Temperature: %u C, LED: %s\r\n",
                      static_cast<unsigned>(temperature_celsius_), led_status);
        write_text(response);
        return;
    }
    write_text("ERR unknown command\r\n");
}

void EnvironmentMonitor::write_text(const char* text)
{
    static_cast<void>(
        cli_uart_.write(reinterpret_cast<const std::uint8_t*>(text), std::strlen(text)));
}

std::uint8_t EnvironmentMonitor::next_temperature_celsius()
{
    random_state_ = random_state_ * 1664525U + 1013904223U;
    return static_cast<std::uint8_t>(23U + random_state_ % 5U);
}

}  // namespace services
