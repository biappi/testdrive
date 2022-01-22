//
//  ReadUtils.h
//  testdrive
//
//  Created by Antonio Malara on 11/10/2020.
//

#pragma once

#include <vector>
#include <cstdint>

namespace TD {

inline uint8_t ReadByte(const std::vector<std::byte> &src, int &i) {
    return std::to_integer<uint8_t>(src[i++]);
};

inline uint16_t ReadWord(const std::vector<std::byte> &src, int &i) {
    auto l = ReadByte(src, i);
    auto h = ReadByte(src, i);
    return (uint16_t)((h << 8) + l);
};

inline uint16_t GetByte(const std::vector<std::byte> &src, int i) {
    return ReadByte(src, i);
};

inline uint16_t GetWord(const std::vector<std::byte> &src, int i) {
    return ReadWord(src, i);
};

}
