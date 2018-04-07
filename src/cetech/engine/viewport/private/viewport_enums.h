#define _ID64(a) ct_hashlib_a0.id64_from_str(a)

bgfx::TextureFormat::Enum format_id_to_enum(uint64_t id) {
    static struct {
        uint64_t id;
        bgfx::TextureFormat::Enum e;
    } _FormatIdToEnum[] = {
            {.id = _ID64(""), .e = bgfx::TextureFormat::Count},
            {.id = _ID64("BC1"), .e = bgfx::TextureFormat::BC1},
            {.id = _ID64("BC2"), .e = bgfx::TextureFormat::BC2},
            {.id = _ID64("BC3"), .e = bgfx::TextureFormat::BC3},
            {.id = _ID64("BC4"), .e = bgfx::TextureFormat::BC4},
            {.id = _ID64("BC5"), .e = bgfx::TextureFormat::BC5},
            {.id = _ID64("BC6H"), .e = bgfx::TextureFormat::BC6H},
            {.id = _ID64("BC7"), .e = bgfx::TextureFormat::BC7},
            {.id = _ID64("ETC1"), .e = bgfx::TextureFormat::ETC1},
            {.id = _ID64("ETC2"), .e = bgfx::TextureFormat::ETC2},
            {.id = _ID64("ETC2A"), .e = bgfx::TextureFormat::ETC2A},
            {.id = _ID64("ETC2A1"), .e = bgfx::TextureFormat::ETC2A1},
            {.id = _ID64("PTC12"), .e = bgfx::TextureFormat::PTC12},
            {.id = _ID64("PTC14"), .e = bgfx::TextureFormat::PTC14},
            {.id = _ID64("PTC12A"), .e = bgfx::TextureFormat::PTC12A},
            {.id = _ID64("PTC14A"), .e = bgfx::TextureFormat::PTC14A},
            {.id = _ID64("PTC22"), .e = bgfx::TextureFormat::PTC22},
            {.id = _ID64("PTC24"), .e = bgfx::TextureFormat::PTC24},
            {.id = _ID64("R1"), .e = bgfx::TextureFormat::R1},
            {.id = _ID64("A8"), .e = bgfx::TextureFormat::A8},
            {.id = _ID64("R8"), .e = bgfx::TextureFormat::R8},
            {.id = _ID64("R8I"), .e = bgfx::TextureFormat::R8I},
            {.id = _ID64("R8U"), .e = bgfx::TextureFormat::R8U},
            {.id = _ID64("R8S"), .e = bgfx::TextureFormat::R8S},
            {.id = _ID64("R16"), .e = bgfx::TextureFormat::R16},
            {.id = _ID64("R16I"), .e = bgfx::TextureFormat::R16I},
            {.id = _ID64("R16U"), .e = bgfx::TextureFormat::R16U},
            {.id = _ID64("R16F"), .e = bgfx::TextureFormat::R16F},
            {.id = _ID64("R16S"), .e = bgfx::TextureFormat::R16S},
            {.id = _ID64("R32I"), .e = bgfx::TextureFormat::R32I},
            {.id = _ID64("R32U"), .e = bgfx::TextureFormat::R32U},
            {.id = _ID64("R32F"), .e = bgfx::TextureFormat::R32F},
            {.id = _ID64("RG8"), .e = bgfx::TextureFormat::RG8},
            {.id = _ID64("RG8I"), .e = bgfx::TextureFormat::RG8I},
            {.id = _ID64("RG8U"), .e = bgfx::TextureFormat::RG8U},
            {.id = _ID64("RG8S"), .e = bgfx::TextureFormat::RG8S},
            {.id = _ID64("RG16"), .e = bgfx::TextureFormat::RG16},
            {.id = _ID64("RG16I"), .e = bgfx::TextureFormat::RG16I},
            {.id = _ID64("RG16U"), .e = bgfx::TextureFormat::RG16U},
            {.id = _ID64("RG16F"), .e = bgfx::TextureFormat::RG16F},
            {.id = _ID64("RG16S"), .e = bgfx::TextureFormat::RG16S},
            {.id = _ID64("RG32I"), .e = bgfx::TextureFormat::RG32I},
            {.id = _ID64("RG32U"), .e = bgfx::TextureFormat::RG32U},
            {.id = _ID64("RG32F"), .e = bgfx::TextureFormat::RG32F},
            {.id = _ID64("RGB8"), .e = bgfx::TextureFormat::RGB8},
            {.id = _ID64("RGB8I"), .e = bgfx::TextureFormat::RGB8I},
            {.id = _ID64("RGB8U"), .e = bgfx::TextureFormat::RGB8U},
            {.id = _ID64("RGB8S"), .e = bgfx::TextureFormat::RGB8S},
            {.id = _ID64("RGB9E5F"), .e = bgfx::TextureFormat::RGB9E5F},
            {.id = _ID64("BGRA8"), .e = bgfx::TextureFormat::BGRA8},
            {.id = _ID64("RGBA8"), .e = bgfx::TextureFormat::RGBA8},
            {.id = _ID64("RGBA8I"), .e = bgfx::TextureFormat::RGBA8I},
            {.id = _ID64("RGBA8U"), .e = bgfx::TextureFormat::RGBA8U},
            {.id = _ID64("RGBA8S"), .e = bgfx::TextureFormat::RGBA8S},
            {.id = _ID64("RGBA16"), .e = bgfx::TextureFormat::RGBA16},
            {.id = _ID64("RGBA16I"), .e = bgfx::TextureFormat::RGBA16I},
            {.id = _ID64("RGBA16U"), .e = bgfx::TextureFormat::RGBA16U},
            {.id = _ID64("RGBA16F"), .e = bgfx::TextureFormat::RGBA16F},
            {.id = _ID64("RGBA16S"), .e = bgfx::TextureFormat::RGBA16S},
            {.id = _ID64("RGBA32I"), .e = bgfx::TextureFormat::RGBA32I},
            {.id = _ID64("RGBA32U"), .e = bgfx::TextureFormat::RGBA32U},
            {.id = _ID64("RGBA32F"), .e = bgfx::TextureFormat::RGBA32F},
            {.id = _ID64("R5G6B5"), .e = bgfx::TextureFormat::R5G6B5},
            {.id = _ID64("RGBA4"), .e = bgfx::TextureFormat::RGBA4},
            {.id = _ID64("RGB5A1"), .e = bgfx::TextureFormat::RGB5A1},
            {.id = _ID64("RGB10A2"), .e = bgfx::TextureFormat::RGB10A2},
            {.id = _ID64("RG11B10F"), .e = bgfx::TextureFormat::RG11B10F},
            {.id = _ID64("D16"), .e = bgfx::TextureFormat::D16},
            {.id = _ID64("D24"), .e = bgfx::TextureFormat::D24},
            {.id = _ID64("D24S8"), .e = bgfx::TextureFormat::D24S8},
            {.id = _ID64("D32"), .e = bgfx::TextureFormat::D32},
            {.id = _ID64("D16F"), .e = bgfx::TextureFormat::D16F},
            {.id = _ID64("D24F"), .e = bgfx::TextureFormat::D24F},
            {.id = _ID64("D32F"), .e = bgfx::TextureFormat::D32F},
            {.id = _ID64("D0S8"), .e = bgfx::TextureFormat::D0S8},
    };

    for (uint32_t i = 1; i < CT_ARRAY_LEN(_FormatIdToEnum); ++i) {
        if (_FormatIdToEnum[i].id != id) {
            continue;
        }

        return _FormatIdToEnum[i].e;
    }

    return _FormatIdToEnum[0].e;
}

bgfx::BackbufferRatio::Enum ratio_id_to_enum(uint64_t id) {
    static struct {
        uint64_t id;
        bgfx::BackbufferRatio::Enum e;
    } _RatioIdToEnum[] = {
            {.id = _ID64(""), .e = bgfx::BackbufferRatio::Count},
            {.id = _ID64("equal"), .e = bgfx::BackbufferRatio::Equal},
            {.id = _ID64("half"), .e = bgfx::BackbufferRatio::Half},
            {.id = _ID64("quarter"), .e = bgfx::BackbufferRatio::Quarter},
            {.id = _ID64("eighth"), .e = bgfx::BackbufferRatio::Eighth},
            {.id = _ID64("sixteenth"), .e = bgfx::BackbufferRatio::Sixteenth},
            {.id = _ID64("double"), .e = bgfx::BackbufferRatio::Double},
    };

    for (uint32_t i = 1; i < CT_ARRAY_LEN(_RatioIdToEnum); ++i) {
        if (_RatioIdToEnum[i].id != id) {
            continue;
        }

        return _RatioIdToEnum[i].e;
    }

    return _RatioIdToEnum[0].e;
}

float ratio_id_to_coef(uint64_t id) {
    static struct {
        uint64_t id;
        float coef;
    } _RatioIdToEnum[] = {
            {.id = _ID64(""), .coef = 1.0f},
            {.id = _ID64("equal"), .coef = 1.0f},
            {.id = _ID64("half"), .coef = 1.0f / 2.0f},
            {.id = _ID64("quarter"), .coef = 1.0f / 4.0f},
            {.id = _ID64("eighth"), .coef = 1.0f / 8.0f},
            {.id = _ID64("sixteenth"), .coef = 1.0f / 16.0f},
            {.id = _ID64("double"), .coef = 2.0f},
    };

    for (uint32_t i = 1; i < CT_ARRAY_LEN(_RatioIdToEnum); ++i) {
        if (_RatioIdToEnum[i].id != id) {
            continue;
        }

        return _RatioIdToEnum[i].coef;
    }

    return _RatioIdToEnum[0].coef;
}
