# Security Policy

Fixed-capacity command authorization, credential lifecycle, and image-signature policy. Cryptographic operations are injected through `ISignatureVerifier`; the service never includes a crypto/vendor SDK. An unavailable verifier rejects image activation by returning `denied`.
