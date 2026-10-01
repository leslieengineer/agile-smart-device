#pragma once

#include <services/ISignatureVerifier.hpp>
#include <services/Limits.hpp>

namespace services
{

enum class SecurityRole : std::uint8_t
{
    guest,
    operator_role,
    administrator
};
enum class CredentialState : std::uint8_t
{
    unset,
    staged,
    active,
    rotating,
    revoked
};

class SecurityPolicy final
{
public:
    explicit SecurityPolicy(ISignatureVerifier& verifier);

    uhal::Status    register_command(const char* command, SecurityRole required_role);
    uhal::Status    authorize(const char* command, SecurityRole actual_role) const;
    uhal::Status    stage_credentials();
    uhal::Status    activate_credentials();
    void            begin_rotation();
    void            revoke_credentials();
    CredentialState credential_state() const;
    uhal::Status    verify_image(const std::uint8_t* digest, std::size_t digest_size,
                                 const std::uint8_t* signature, std::size_t signature_size,
                                 std::uint8_t key_id);

private:
    struct Authorization
    {
        bool         used = false;
        char         command[kConfigKeyCapacity]{};
        SecurityRole required_role = SecurityRole::administrator;
    };

    ISignatureVerifier& verifier_;
    Authorization       authorizations_[kCommandHandlerCapacity]{};
    CredentialState     credential_state_ = CredentialState::unset;
};

}  // namespace services
