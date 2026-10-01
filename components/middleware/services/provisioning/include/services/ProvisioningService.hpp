#pragma once
#include <services/IConfigStore.hpp>
#include <services/Limits.hpp>
#include <uhal/Status.hpp>
namespace services
{
struct NetworkCredentials
{
    char ssid[kSsidCapacity]{};
    char passphrase[kPassphraseCapacity]{};
    char apn[32]{};
};
struct PortalConfig
{
    std::uint32_t timeout_ms = 300000U;
};
class IProvisioningPortal
{
public:
    virtual ~IProvisioningPortal()                             = default;
    virtual uhal::Status open(const PortalConfig&)             = 0;
    virtual uhal::Status close()                               = 0;
    virtual void         poll(std::uint32_t)                   = 0;
    virtual bool         has_credentials() const               = 0;
    virtual uhal::Status take_credentials(NetworkCredentials&) = 0;
};
class IProvisioningValidator
{
public:
    virtual ~IProvisioningValidator()                        = default;
    virtual uhal::Status validate(const NetworkCredentials&) = 0;
};
enum class ProvisioningState : std::uint8_t
{
    unprovisioned,
    portal_open,
    validating,
    provisioned,
    failed
};
class ProvisioningService final
{
public:
    ProvisioningService(IProvisioningPortal&, IProvisioningValidator&, IConfigStore&,
                        PortalConfig config = {});
    uhal::Status      start(std::uint32_t);
    uhal::Status      stop();
    void              poll(std::uint32_t);
    uhal::Status      factory_reset();
    ProvisioningState state() const;

private:
    void                    fail_attempt();
    void                    clear_pending();
    IProvisioningPortal&    portal_;
    IProvisioningValidator& validator_;
    IConfigStore&           store_;
    PortalConfig            config_{};
    NetworkCredentials      pending_{};
    ProvisioningState       state_     = ProvisioningState::unprovisioned;
    std::uint32_t           opened_at_ = 0U;
    std::uint8_t            failures_  = 0U;
};
}  // namespace services
