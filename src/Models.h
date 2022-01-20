//
//  Models.hpp
//  TestScene
//
//  Created by Antonio Malara on 11/10/2020.
//

#pragma once

#include "Resources.h"

namespace TD {

struct Point {
    int16_t x, y, z;
    
    Point()
        : x(0), y(0), z(0) { }
    
    Point(int16_t x, int16_t y, int16_t z)
        : x(x), y(y), z(z) { }
};

class Poly {
public:
    Poly()
        : a(0), b(0), c(0), d(0) { }

    Poly(uint16_t a, uint16_t b, uint16_t c, uint16_t d)
        : a(a), b(b), c(c), d(d) { }
    
    uint8_t type()   const { return a >> 13; }
    uint8_t color0() const { return b >> 11; }
    uint8_t color1() const { return c >> 11; }
    uint8_t role()   const { return d >> 13; }
    
    uint8_t idx0() const { return a & 0x7ff; }
    uint8_t idx1() const { return b & 0x7ff; }
    uint8_t idx2() const { return c & 0x7ff; }
    uint8_t idx3() const { return d & 0x7ff; }
    
    uint8_t T() const { return a & 0xf800; }
    uint8_t R() const { return d & 0xf800; }

private:
    uint16_t a, b, c, d;
};

struct Sprite {
    uint16_t a, b, c, d;
};

class Model {
public:
    
    Model(const std::vector<std::byte> &modelData, int offset, bool has_lod = false);
    
    const std::vector<Poly>   &polys()  const { return m_polys; };
    const std::vector<Point>  &points() const { return m_points; };
    const std::vector<Sprite> &sprites() const { return m_sprites; };
    
private:
    std::vector<Poly> m_polys;
    std::vector<Point> m_points;
    std::vector<Sprite> m_sprites;
};


inline Model LoadModel(const std::vector<std::byte> data, const int idx, bool has_lod = false)
{
    auto offset = GetWord(data, idx * 2);
    return Model(data, offset, has_lod);
}


inline std::vector<Model> LoadModels(const std::vector<std::byte> data, const int count, bool has_lod = false)
{
    std::vector<Model> res;
    
    int i = 0;
    
    /*
    auto firstOffset = ReadWord(scene_t_bin, i);
    auto tileCount = (firstOffset - 4) / 2;
    i = 0;
     */
    
    for (int k = 0; k < count; k++) {
        auto offset = ReadWord(data, i);
        
        if (offset < 0x10) {
            auto j = i - 4;
            offset = ReadWord(data, j);
        }
        
        auto tile = Model(data, offset, has_lod);
        res.push_back(tile);
    }

    return res;
}



}
