#include <services/OtaService.hpp>
namespace services
{
OtaService::OtaService(IFirmwareStore& s, IBootControl& b, IImageFetcher& f, IDigestCalculator& d,
                       ISignatureVerifier& v)
    : store_{s}, boot_{b}, fetcher_{f}, digest_{d}, verifier_{v}
{
}
uhal::Status OtaService::offer(const ImageManifest& m)
{
    if (state_ != OtaState::idle || m.size_bytes == 0U)
        return uhal::Status::invalid_argument;
    if (store_.begin(m.size_bytes) != uhal::Status::ok || digest_.begin() != uhal::Status::ok)
    {
        fail();
        return uhal::Status::io_error;
    }
    manifest_  = m;
    received_  = 0U;
    requested_ = false;
    state_     = OtaState::downloading;
    return uhal::Status::ok;
}
void OtaService::poll(std::uint32_t now)
{
    if (state_ != OtaState::downloading)
        return;
    fetcher_.poll(now);
    if (!requested_)
    {
        const std::size_t  remaining = manifest_.size_bytes - received_;
        const std::size_t  wanted    = remaining < sizeof(chunk_) ? remaining : sizeof(chunk_);
        const uhal::Status s         = fetcher_.request(received_, wanted);
        if (s == uhal::Status::ok || s == uhal::Status::busy)
            requested_ = true;
        else
            fail();
        return;
    }
    std::size_t        size = 0U;
    const uhal::Status take = fetcher_.take(chunk_, sizeof(chunk_), size);
    if (take == uhal::Status::busy)
        return;
    if (take != uhal::Status::ok || size == 0U || received_ + size > manifest_.size_bytes ||
        store_.write(received_, chunk_, size) != uhal::Status::ok ||
        digest_.update(chunk_, size) != uhal::Status::ok)
    {
        fail();
        return;
    }
    received_ += static_cast<std::uint32_t>(size);
    requested_ = false;
    if (received_ != manifest_.size_bytes)
        return;
    state_ = OtaState::verifying;
    std::uint8_t calculated[32]{};
    if (digest_.finish(calculated, sizeof(calculated)) != uhal::Status::ok)
    {
        fail();
        return;
    }
    for (std::size_t i = 0U; i < sizeof(calculated); ++i)
        if (calculated[i] != manifest_.digest[i])
        {
            fail();
            return;
        }
    const uhal::Status verified =
        verifier_.verify(calculated, sizeof(calculated), manifest_.signature,
                         sizeof(manifest_.signature), manifest_.key_id);
    if (verified != uhal::Status::ok || store_.finish() != uhal::Status::ok)
    {
        fail();
        return;
    }
    state_ = OtaState::ready_to_activate;
}
uhal::Status OtaService::activate()
{
    if (state_ != OtaState::ready_to_activate)
        return uhal::Status::not_ready;
    state_               = OtaState::activating;
    const uhal::Status s = boot_.activate_staged();
    if (s != uhal::Status::ok)
        fail();
    return s;
}
void OtaService::cancel()
{
    store_.abort();
    state_     = OtaState::idle;
    received_  = 0U;
    requested_ = false;
}
OtaState OtaService::state() const
{
    return state_;
}
std::uint32_t OtaService::received() const
{
    return received_;
}
void OtaService::fail()
{
    store_.abort();
    state_     = OtaState::failed;
    requested_ = false;
}
}  // namespace services
