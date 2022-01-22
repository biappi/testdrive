//
//  Decoders.h
//  testdrive
//
//  Created by Antonio Malara on 07/10/2020.
//

#pragma once

#include <cstddef>
#include <vector>

static std::vector<std::byte> Decode(const std::vector<std::byte> &buf_src) {
    std::vector<std::byte> buf_dst;

    const auto initial_src_idx = 0x400;
    auto top = std::min((int)buf_src.size(), initial_src_idx);

    std::vector<std::byte> buf_private(&buf_src[0], &buf_src[top]);
    for (int i = 0; i < initial_src_idx - top; i++)
        buf_private.push_back(std::byte(0));

    auto src_buf_idx = initial_src_idx;

    std::vector<std::byte> buffer_14(0x300 * 16);
    uint16_t first_word_for_buffer_14 = 0;
    uint8_t  third_byte_for_buffer_14 = 0;

    std::vector<std::byte> to_push;

    uint16_t boh_2 = 0;
    uint8_t byte_306AC = 0;
    uint16_t word_3069E = 0;
    uint16_t word_306A2 = 0;

    uint16_t buffer_4_idx = 0x102;
    uint16_t default_9 = 9;
    uint16_t default_200 = 0x200;

    constexpr uint16_t rest_list[] = {
        0x1ff,
        0x3ff,
        0x7ff,
        0xfff,
    };

    auto reset_params = [&]() {
        buffer_4_idx = 0x102;
        default_9 = 9;
        default_200 = 0x200;
    };

    auto decode_internal = [&]() {
        uint16_t ax = boh_2;
        ax += default_9;
        std::swap(ax, boh_2);

        int temp = ax;
        ax = temp / 8;
        int dx = temp % 8;

        if (ax >= 0x3fd) {
            boh_2 = default_9 + dx;

            for (int i = 0; i < 0x400 - ax; i++) {
                buf_private[i] = buf_private[i + ax];
            }

            for (int i = 0; i < ax; i++) {
                int idx = i + src_buf_idx;
                std::byte data{};

                if (idx < buf_src.size()) {
                    data = buf_src[idx];
                }

                buf_private[0x400 - ax + i] = data;
            }

            src_buf_idx += ax;
            ax = 0;
        }

        uint16_t bx = (std::to_integer<uint16_t>(buf_private[ax + 1]) << 8)
                    + std::to_integer<uint16_t>(buf_private[ax]);

        uint8_t al = std::to_integer<uint8_t>(buf_private[ax + 2]);

        for (int i = 0; i < dx; i++) {
            uint8_t bit = al & 1;
            al = al >> 1;
            bx = ((bit << 15) | (bx >> 1));
        }

        return bx & rest_list[default_9 - 9];
    };

    auto fill_buffer_14 = [&]() {
        auto idx = buffer_4_idx * 3;

        buffer_14[idx    ] = std::byte(first_word_for_buffer_14 & 0x00ff);
        buffer_14[idx + 1] = std::byte((first_word_for_buffer_14 & 0xff00) >> 8);
        buffer_14[idx + 2] = std::byte(third_byte_for_buffer_14);

        buffer_4_idx++;
    };

    while (true) {
        auto decode_internal_result = decode_internal();

        if (decode_internal_result == 0x101) {
            break;
        }
        else if (decode_internal_result == 0x100) {
            reset_params();

            auto ax = decode_internal();
            word_3069E = ax;
            first_word_for_buffer_14 = ax;
            third_byte_for_buffer_14 = ax & 0xff;
            byte_306AC = ax & 0xff;
            buf_dst.push_back(std::byte(ax & 0xff));
        }
        else {
            word_3069E = decode_internal_result;
            word_306A2 = decode_internal_result;

            if (decode_internal_result >= buffer_4_idx) {
                word_3069E = first_word_for_buffer_14;
                to_push.push_back(std::byte(byte_306AC));
            }

            while (word_3069E > 0xff) {
                auto bx = word_3069E * 3;
                to_push.push_back(buffer_14[bx + 2]);

                word_3069E = (
                    std::to_integer<uint8_t>(buffer_14[bx]) +
                    (std::to_integer<uint8_t>(buffer_14[bx + 1]) << 8)
                );
            }

            auto al = word_3069E & 0xff;
            byte_306AC = al;
            third_byte_for_buffer_14 = al;

            to_push.push_back(std::byte(al));

            while (!to_push.empty()) {
                buf_dst.push_back(to_push.back());
                to_push.pop_back();
            }

            fill_buffer_14();

            first_word_for_buffer_14 = word_306A2;

            if (buffer_4_idx < default_200) {
                continue;
            }

            if (default_9 == 0x0c) {
                continue;
            }

            default_9++;
            default_200 = default_200 << 1;
        }
    }

    return buf_dst;
}

static std::vector<std::byte> RLEDecode(const std::vector<std::byte> &buf_src) {
    std::vector<std::byte> buf_dst;
    int i = 0;

    while (i < buf_src.size()) {
        auto color = buf_src[i++];
        auto count = buf_src[i++];

        for (int k = 0; k < std::to_integer<uint8_t>(count); k++) {
            buf_dst.push_back(color);
        }
    }

    return buf_dst;
}
