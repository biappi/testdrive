//
//  Decoders.h
//  TestDrive
//
//  Created by Antonio Malara on 07/10/2020.
//

#pragma once

#include <cstddef>
#include <vector>

std::vector<std::byte> Decode(const std::vector<std::byte> &buf_src);
std::vector<std::byte> RLEDecode(const std::vector<std::byte> &buf_src);
