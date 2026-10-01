# Bounded Serialization

Header-only byte and flat-JSON readers/writers over caller-owned buffers. Operations fail explicitly on overflow or malformed input and perform no dynamic allocation. JSON support intentionally handles bounded flat objects only; complex schemas belong to dedicated protocol codecs.
