//
//  GameImage.h
//  testdrive
//
//  Created by Antonio Malara on 21/01/2022.
//

#pragma once

namespace TD {

class GameImage {
public:
    GameImage(const std::vector<std::byte> &imageLz, int width, GamePalette &palette, int colorBase = 0)
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
                m_bitmap[dstIndex] = palette.palette[std::to_integer<uint8_t>(bitmap8bpp[srcIndex]) + colorBase];
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
