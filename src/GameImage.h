//
//  GameImage.h
//  testdrive
//
//  Created by Antonio Malara on 21/01/2022.
//

#pragma once

#include "Decoders.h"

namespace TD {

struct Color {
    uint8_t r, g, b, a;
};

static const std::vector<Color> DefaultPalette = {
    { 0x00, 0x00, 0x00, 0xff },
    { 0x00, 0x00, 0xa0, 0xff },
    { 0x00, 0xa0, 0x00, 0xff },
    { 0x00, 0xa0, 0xa0, 0xff },
    { 0xa0, 0x00, 0x00, 0xff },
    { 0xa0, 0x00, 0xa0, 0xff },
    { 0xa0, 0x50, 0x00, 0xff },
    { 0xa0, 0xa0, 0xa0, 0xff },
    { 0x50, 0x50, 0x50, 0xff },
    { 0x50, 0x50, 0xf0, 0xff },
    { 0x50, 0xf0, 0x50, 0xff },
    { 0x50, 0xf0, 0xf0, 0xff },
    { 0xf0, 0x50, 0x50, 0xff },
    { 0xf0, 0x50, 0xf0, 0xff },
    { 0xf0, 0xf0, 0x50, 0xff },
    { 0xf0, 0xf0, 0xf0, 0xff },
};

class GamePalette {
public:
    GamePalette()
        : palette(0x100)
    {
        copy(DefaultPalette, 0);
    }

    GamePalette(const std::vector<std::byte> &data, int at)
        : palette(0x100)
    {
        auto palette = PaletteFromData(data);
        copy(DefaultPalette, 0);
        copy(palette, at);
    }

    void copy(std::vector<Color> src, int at) {
        for (int i = 0; i < src.size(); i++) {
            palette[at + i] = src[i];
        }
    }

    Color get(int at) const {
        return palette[at];
    }

private:
    std::vector<Color> palette;

    static std::vector<Color> PaletteFromData(const std::vector<std::byte> &data) {
        auto count = data.size() / 3;
        std::vector<Color> palette(count);

        for (int i = 0; i < count; i++) {
            palette[i].r = std::to_integer<uint8_t>(data[(i * 3) + 0]) << 2;
            palette[i].g = std::to_integer<uint8_t>(data[(i * 3) + 1]) << 2;
            palette[i].b = std::to_integer<uint8_t>(data[(i * 3) + 2]) << 2;
            palette[i].a = 255;
        }

        return palette;
    }

};

class GameImage {
public:
    GameImage(const std::vector<std::byte> &imageLz, int width, const GamePalette &palette, int colorBase = 0)
        : m_textureLoaded(false)
    {
        auto decoded    = Decode(imageLz);
        auto bitmap8bpp = RLEDecode(decoded);

        int height = static_cast<int>(bitmap8bpp.size()) / width;

        m_bitmap.resize(width * height);

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                auto srcIndex = width * (height - y - 1) + x;
                auto dstIndex = width * y + x;
                auto pixel = bitmap8bpp[srcIndex];
                auto color = std::to_integer<uint8_t>(pixel);

                m_bitmap[dstIndex] = palette.get(color + colorBase);
            }
        }

        m_image = {
            .data = &m_bitmap[0],
            .width = width,
            .height = height,
            .mipmaps = 1,
            .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
        };
    }

    Texture2D texture() {
        if (!m_textureLoaded) {
            m_texture = LoadTextureFromImage(m_image);
            m_textureLoaded = true;
        }

        return m_texture;
    }

    void unloadTexture() {
        if (m_textureLoaded)
            UnloadTexture(m_texture);
    }

private:
    std::vector<TD::Color> m_bitmap;
    Image m_image;
    Texture2D m_texture;
    bool m_textureLoaded;
};

};
