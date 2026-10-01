#pragma once
#include <cstddef>
#include <cstdint>
#include <services/ISignatureVerifier.hpp>
#include <services/Limits.hpp>
#include <uhal/Status.hpp>
namespace services
{
enum class OtaState : std::uint8_t
{
    idle,
    downloading,
    verifying,
    ready_to_activate,
    activating,
    failed,
    rolled_back
};
struct ImageManifest
{
    char          version[16]{};
    std::uint32_t size_bytes = 0U;
    std::uint8_t  digest[32]{};
    std::uint8_t  signature[64]{};
    std::uint8_t  key_id = 0U;
};
class IFirmwareStore
{
public:
    virtual ~IFirmwareStore()                                                   = default;
    virtual uhal::Status begin(std::uint32_t)                                   = 0;
    virtual uhal::Status write(std::uint32_t, const std::uint8_t*, std::size_t) = 0;
    virtual uhal::Status finish()                                               = 0;
    virtual void         abort()                                                = 0;
};
class IBootControl
{
public:
    virtual ~IBootControl()                = default;
    virtual uhal::Status activate_staged() = 0;
    virtual uhal::Status rollback()        = 0;
};
class IImageFetcher
{
public:
    virtual ~IImageFetcher()                                            = default;
    virtual uhal::Status request(std::uint32_t, std::size_t)            = 0;
    virtual void         poll(std::uint32_t)                            = 0;
    virtual uhal::Status take(std::uint8_t*, std::size_t, std::size_t&) = 0;
};
class IDigestCalculator
{
public:
    virtual ~IDigestCalculator()                                  = default;
    virtual uhal::Status begin()                                  = 0;
    virtual uhal::Status update(const std::uint8_t*, std::size_t) = 0;
    virtual uhal::Status finish(std::uint8_t*, std::size_t)       = 0;
};
class OtaService final
{
public:
    OtaService(IFirmwareStore&, IBootControl&, IImageFetcher&, IDigestCalculator&,
               ISignatureVerifier&);
    uhal::Status  offer(const ImageManifest&);
    void          poll(std::uint32_t now_ms);
    uhal::Status  activate();
    void          cancel();
    OtaState      state() const;
    std::uint32_t received() const;

private:
    void                fail();
    IFirmwareStore&     store_;
    IBootControl&       boot_;
    IImageFetcher&      fetcher_;
    IDigestCalculator&  digest_;
    ISignatureVerifier& verifier_;
    ImageManifest       manifest_{};
    std::uint8_t        chunk_[kOtaChunkBytes]{};
    std::uint32_t       received_  = 0U;
    bool                requested_ = false;
    OtaState            state_     = OtaState::idle;
};
}  // namespace services
