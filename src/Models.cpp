//
//  Models.cpp
//  TestScene
//
//  Created by Antonio Malara on 11/10/2020.
//

#include "Models.h"

namespace TD {

Model::Model(const std::vector<std::byte> &modelData, int ofs, bool has_lod)
{
    auto polyCount    = ReadByte(modelData, ofs);
    auto pointCount   = ReadByte(modelData, ofs);
    
    auto spritesCount = 0;
    auto x            = 0;

    if (modelData.size() < (ofs + pointCount * 6 + pointCount * 8)) {
        return;
    }
    
    if (has_lod) {
        ofs += 6;
//        polyCount    = ReadByte(modelData, ofs);
//        pointCount   = ReadByte(modelData, ofs);
//
//        ofs += ReadWord(modelData, ofs);
    }
    else {
        spritesCount = ReadByte(modelData, ofs);
        x = ReadByte(modelData, ofs);
    }
    
    m_points.resize(pointCount);
        
    for (int i = 0; i < pointCount; i++) {
        m_points[i].z = (int16_t)ReadWord(modelData, ofs);
    }

    for (int i = 0; i < pointCount; i++) {
        m_points[i].x = (int16_t)ReadWord(modelData, ofs);
    }

    for (int i = 0; i < pointCount; i++) {
        m_points[i].y = (int16_t)ReadWord(modelData, ofs);
    }
    
    for (int i = 0; i < polyCount; i++) {
        auto a = ReadWord(modelData, ofs);
        auto b = ReadWord(modelData, ofs);
        auto c = ReadWord(modelData, ofs);
        auto d = ReadWord(modelData, ofs);

        m_polys.push_back(Poly(a, b, c, d));
    }
    
    for (int i = 0; i < spritesCount; i++) {
        Sprite s;
        s.a = ReadWord(modelData, ofs);
        s.b = ReadWord(modelData, ofs);
        s.c = ReadWord(modelData, ofs);
        s.d = ReadWord(modelData, ofs);
        
        m_sprites.push_back(s);
    }
    
}

}
