#include <services/SecurityPolicy.hpp>

namespace services
{
namespace
{

std::size_t text_length(const char* text, std::size_t capacity)
{
    if (text == nullptr)
        return capacity;
    std::size_t size = 0U;
    while (size < capacity && text[size] != '\0') ++size;
    return size;
}

bool text_equal(const char* left, const char* right, std::size_t capacity)
{
    if (left == nullptr || right == nullptr)
        return false;
    for (std::size_t index = 0U; index < capacity; ++index)
    {
        if (left[index] != right[index])
            return false;
        if (left[index] == '\0')
            return true;
    }
    return false;
}

}  // namespace

SecurityPolicy::SecurityPolicy(ISignatureVerifier& verifier) : verifier_{verifier} {}

uhal::Status SecurityPolicy::register_command(const char* command, SecurityRole required_role)
{
    const std::size_t size = text_length(command, kConfigKeyCapacity);
    if (size == 0U || size >= kConfigKeyCapacity)
        return uhal::Status::invalid_argument;
    for (auto& authorization : authorizations_)
    {
        if (authorization.used && text_equal(authorization.command, command, kConfigKeyCapacity))
        {
            authorization.required_role = required_role;
            return uhal::Status::ok;
        }
    }
    for (auto& authorization : authorizations_)
    {
        if (authorization.used)
            continue;
        authorization.used          = true;
        authorization.required_role = required_role;
        for (std::size_t index = 0U; index < size; ++index)
            authorization.command[index] = command[index];
        authorization.command[size] = '\0';
        return uhal::Status::ok;
    }
    return uhal::Status::no_resources;
}

uhal::Status SecurityPolicy::authorize(const char* command, SecurityRole actual_role) const
{
    for (const auto& authorization : authorizations_)
    {
        if (!authorization.used || !text_equal(authorization.command, command, kConfigKeyCapacity))
            continue;
        return static_cast<std::uint8_t>(actual_role) >=
                       static_cast<std::uint8_t>(authorization.required_role)
                   ? uhal::Status::ok
                   : uhal::Status::denied;
    }
    return uhal::Status::not_found;
}

uhal::Status SecurityPolicy::stage_credentials()
{
    if (credential_state_ != CredentialState::unset &&
        credential_state_ != CredentialState::rotating)
        return uhal::Status::busy;
    credential_state_ = CredentialState::staged;
    return uhal::Status::ok;
}

uhal::Status SecurityPolicy::activate_credentials()
{
    if (credential_state_ != CredentialState::staged)
        return uhal::Status::not_ready;
    credential_state_ = CredentialState::active;
    return uhal::Status::ok;
}

void SecurityPolicy::begin_rotation()
{
    credential_state_ = CredentialState::rotating;
}
void SecurityPolicy::revoke_credentials()
{
    credential_state_ = CredentialState::revoked;
}
CredentialState SecurityPolicy::credential_state() const
{
    return credential_state_;
}

uhal::Status SecurityPolicy::verify_image(const std::uint8_t* digest, std::size_t digest_size,
                                          const std::uint8_t* signature, std::size_t signature_size,
                                          std::uint8_t key_id)
{
    const uhal::Status status =
        verifier_.verify(digest, digest_size, signature, signature_size, key_id);
    return status == uhal::Status::unsupported ? uhal::Status::denied : status;
}

}  // namespace services
