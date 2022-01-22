//
//  Scene.h
//  testdrive
//
//  Created by Antonio Malara on 22/01/2022.
//

#pragma once

namespace TD {

struct TileInfo {

    int tileId() const {
        return std::to_integer<int>(tile);
    }

    int rot() const {
        return std::to_integer<int>(info) >> 6;
    }

    int height() const {
        return (std::to_integer<int>(info) & 0x3f) * 250;
    }

    std::byte tile;
    std::byte info;
};

class GameObject {
public:
    GameObject(uint16_t objectId,
               uint16_t x,
               uint16_t y,
               uint16_t z,
               uint16_t rotation)
        : m_objectId(objectId)
        , m_x(x), m_y(y), m_z(z)
        , m_rotation(rotation)
    {
    }

    uint16_t modelId()   const { return m_objectId & 0x003f; }
    uint16_t flags()     const { return m_objectId & 0xffc0; }
    bool     isLOD()     const { return m_objectId & 0x3000; }

    Point    location()  const { return Point(m_x, m_y, m_z); }
    int      rotation()  const { return m_rotation >> 6; }

private:
    uint16_t m_objectId;
    uint16_t m_x;
    uint16_t m_y;
    uint16_t m_z;
    uint16_t m_rotation;
};

class Scene {

private:
    static const int tta_dseg_start_offset = 0x9370;

public:
    static const int XTileCount = 32;
    static const int YTileCount = 16;

    int getSingleCourseDataBoh() const {
        static int offset = 0x939d - tta_dseg_start_offset;
        return std::to_integer<uint8_t>(a_dat[offset]);
    }

    uint8_t getCourseDataLut2(int i) const {
        static int offset = 0x93bf - tta_dseg_start_offset;
        return GetByte(a_dat, offset + i);
    }

    TileInfo getTileInfo(int l) const {
        static int offset = 0x944f - tta_dseg_start_offset;

        return {
            .tile = a_dat[offset + l * 2    ],
            .info = a_dat[offset + l * 2 + 1],
        };
    }

    TileInfo getTileInfo(int tileX, int tileY) const {
        return getTileInfo(tileY * XTileCount + tileX);
    }

    Color mapColor(uint8_t colorHi, uint8_t colorLo, const GamePalette &palette) const {
        static int doubleColorTable = 0xb297 - tta_dseg_start_offset;
        static int singleColorTable = 0xb497 - tta_dseg_start_offset;

        if (colorHi & 0x10) {
            colorHi = std::to_integer<uint8_t>(a_dat[singleColorTable + (colorHi & 0x0f)]);
            if (colorLo & 0x10) {
                colorLo = std::to_integer<uint8_t>(a_dat[singleColorTable + (colorLo & 0x0f)]);
            }
        }
        else if (colorLo & 0x10) {
            colorLo = std::to_integer<uint8_t>(a_dat[singleColorTable + (colorLo & 0x0f)]);
        }
        else {
            auto idx = (colorHi << 4) | colorLo;

            colorLo = std::to_integer<uint8_t>(a_dat[doubleColorTable + (idx * 2) + 0]);
            colorHi = std::to_integer<uint8_t>(a_dat[doubleColorTable + (idx * 2) + 1]);
        }

        auto pattern0rgb = palette.get(colorLo);
        auto pattern1rgb = palette.get(colorHi);

        Color color;
        color.r = (pattern0rgb.r + pattern1rgb.r) / 2;
        color.g = (pattern0rgb.g + pattern1rgb.g) / 2;
        color.b = (pattern0rgb.b + pattern1rgb.b) / 2;
        color.a = 0xff;
        return color;
    }

    void loadObjectData(const std::vector<std::byte> &a_dat)
    {
        const auto objectIdOffset    = 0xa257 - tta_dseg_start_offset;
        const auto xOffset           = 0xa397 - tta_dseg_start_offset;
        const auto yOffset           = 0xa4d7 - tta_dseg_start_offset;
        const auto zOffset           = 0xa617 - tta_dseg_start_offset;
        const auto orientationOffset = 0xa757 - tta_dseg_start_offset;

        /*
        auto normalObjectsCount = GetWord(a_dat, 0xa251 - 0x9370);
        auto activeObjectsCount = GetWord(a_dat, 0xa253 - 0x9370);
        auto lodObjectsCount    = GetWord(a_dat, 0xa255 - 0x9370);
        */

        for (int i = 0; i < 0xa0; i++)
        {
            m_objects.emplace_back(
                GetWord(a_dat, objectIdOffset + i * 2),
                GetWord(a_dat, xOffset + i * 2),
                GetWord(a_dat, yOffset + i * 2),
                GetWord(a_dat, zOffset + i * 2),
                GetWord(a_dat, orientationOffset + i * 2)
            );
        }
    }

public:
    std::vector<std::byte> a_dat;
    std::vector<std::byte> one_dat;
    std::vector<std::byte> t_bin;
    std::vector<std::byte> o_bin;
    std::vector<std::byte> p_bin;

    std::vector<Model> tiles;

    std::vector<GameObject> m_objects;
};

}
