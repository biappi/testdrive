//
//  barfs.h
//  testdrive
//
//  Created by Antonio Malara on 21/01/2022.
//

#pragma once

// Maybe some sort of sprite decompression / decrption routine
// used for billboards in the 3d world

const uint8_t barfs_course_data_lut_1[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x0A, 0x0C, 0x0E, 0x10, 0x12, 0x14, 0x17,
    0x1A, 0x1D, 0x20, 0x24, 0x28, 0x2C
};

const uint8_t barfs_boh_lut_2[] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x7F, 0xFF, 0xFE, 0xFF, 0xFF, 0xF7, 0xFF, 0xFF, 0xBF, 0xEF, 0x7F, 0xFF,
    0xFE, 0xFD, 0xFD, 0xF7, 0xFF, 0xDF, 0xBD, 0xEF, 0x7F, 0xF7, 0xFE, 0xFD,
    0xBD, 0x77, 0xFF, 0xDF, 0xBD, 0xEF, 0x7B, 0xB7, 0xDE, 0xFD, 0xBD, 0x77,
    0xFF, 0xDD, 0xBD, 0xEB, 0x7B, 0xB7, 0xDE, 0xBD, 0xBD, 0x77, 0xFF, 0xDD,
    0xAD, 0xEB, 0x7A, 0xB7, 0x5E, 0xBD, 0xB5, 0x77, 0x7F, 0xDD, 0xAD, 0xAB,
    0x7A, 0xB5, 0x5E, 0xB5, 0xB5, 0x75, 0x7D, 0xDD, 0xA5, 0xAB, 0x5A, 0xB5,
    0x5A, 0xB5, 0xA5, 0x75, 0x7D, 0x5D, 0xA5, 0xAA, 0x5A, 0xA5, 0x5A, 0xA5,
    0xA5, 0x55, 0x7D, 0x5D, 0x25, 0xAA, 0x52, 0xA5, 0x4A, 0xA5, 0xA4, 0x55,
    0x7D, 0x55, 0x25, 0x8A, 0x52, 0xA5, 0x4A, 0xA1, 0xA4, 0x55, 0x5D, 0x55,
    0x25, 0x8A, 0x52, 0xA4, 0x4A, 0xA1, 0xA4, 0x55, 0x55, 0x55, 0x25, 0x8A,
    0x12, 0xA4, 0x42, 0xA1, 0xA4, 0x15, 0x55, 0x55, 0x21, 0x8A, 0x12, 0xA4,
    0x42, 0xA1, 0x84, 0x15, 0x55, 0x45, 0x21, 0x82, 0x12, 0xA4, 0x42, 0x21,
    0x84, 0x11, 0x55, 0x45, 0x21, 0x82, 0x12, 0x24, 0x42, 0x21, 0x84, 0x11,
    0x55, 0x44, 0x01, 0x82, 0x12, 0x24, 0x42, 0x21, 0x80, 0x11, 0x15, 0x44,
    0x01, 0x80, 0x12, 0x24, 0x42, 0x01, 0x80, 0x11, 0x14, 0x44, 0x01, 0x80,
    0x02, 0x24, 0x40, 0x01, 0x80, 0x10, 0x14, 0x44, 0x01, 0x80, 0x02, 0x20,
    0x40, 0x01, 0x80, 0x10, 0x14, 0x04, 0x00, 0x80, 0x02, 0x20, 0x40, 0x01,
    0x00, 0x00, 0x14, 0x04, 0x00, 0x80, 0x00, 0x20, 0x00, 0x01, 0x00, 0x00,
    0x14, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00,
};

const int buffer2_size = 0xd000;
uint8_t buffer2[buffer2_size];

uint8_t rol_buffer[30] = { 0 }; // ds:b7cb

void init_rol_buffer(TD::Scene& scene,
                     uint8_t *byte_3a8f2,
                     uint8_t *byte_3a8f3)
{
    printf("[init_rol_buffer]\n\n");
    printf("  0F68:2d1a");

    uint8_t ah = *byte_3a8f2;

    if (ah >= 0x18) {
        printf("    0F68:2d23");

        ah = ah - 0x18;
        *byte_3a8f3 = *byte_3a8f3 + 1;

        if (ah >= 0x18) {
            printf("      0F68:2d2f");
            ah = ah - 0x18;
            *byte_3a8f3 = *byte_3a8f3 + 1;
        }
    }

    printf("  0F68:2d36");

    uint16_t bx = 0;

    if (ah < 0x18) {
        printf("    0F68:2d3d");
        ah = 0x17 - ah;
        bx = ah * 10;
    }

    rol_buffer[0x00] = barfs_boh_lut_2[bx + 0];
    rol_buffer[0x01] = barfs_boh_lut_2[bx + 1];

    rol_buffer[0x07] = barfs_boh_lut_2[bx + 0];
    rol_buffer[0x08] = barfs_boh_lut_2[bx + 0];

    rol_buffer[0x0e] = barfs_boh_lut_2[bx + 0];
    rol_buffer[0x0f] = barfs_boh_lut_2[bx + 0];

    rol_buffer[0x15] = barfs_boh_lut_2[bx + 0];
    rol_buffer[0x16] = barfs_boh_lut_2[bx + 0];



    rol_buffer[0x02] = barfs_boh_lut_2[bx + 2];
    rol_buffer[0x03] = barfs_boh_lut_2[bx + 3];

    rol_buffer[0x09] = barfs_boh_lut_2[bx + 2];
    rol_buffer[0x0a] = barfs_boh_lut_2[bx + 3];

    rol_buffer[0x10] = barfs_boh_lut_2[bx + 2];
    rol_buffer[0x11] = barfs_boh_lut_2[bx + 3];

    rol_buffer[0x17] = barfs_boh_lut_2[bx + 2];
    rol_buffer[0x18] = barfs_boh_lut_2[bx + 3];



    rol_buffer[0x04] = barfs_boh_lut_2[bx + 4];
    rol_buffer[0x05] = barfs_boh_lut_2[bx + 5];

    rol_buffer[0x0b] = barfs_boh_lut_2[bx + 4];
    rol_buffer[0x0c] = barfs_boh_lut_2[bx + 5];

    rol_buffer[0x12] = barfs_boh_lut_2[bx + 4];
    rol_buffer[0x13] = barfs_boh_lut_2[bx + 5];

    rol_buffer[0x19] = barfs_boh_lut_2[bx + 5];


    rol_buffer[0x06] = barfs_boh_lut_2[bx + 7];
    rol_buffer[0x0d] = barfs_boh_lut_2[bx + 7];
    rol_buffer[0x14] = barfs_boh_lut_2[bx + 7];

    uint8_t ch = barfs_boh_lut_2[bx + 6];

    if (ah >= 0x18) {
        printf("    0F68:2db2");
        ah = ah - 0x18;
        *byte_3a8f3 = 3;

        bx = 0;

        if (ah < 0x18) {
            printf("    0F68:2dc1");
            ah = 0x17 - ah;
            bx = ah * 10;
        }

        ch = barfs_boh_lut_2[bx + 7];
    }

    rol_buffer[0x1b] = ch;

    rol_buffer[0x1c] = barfs_boh_lut_2[bx + 8];
    rol_buffer[0x1d] = barfs_boh_lut_2[bx + 9];

    printf("    0F68:2de7");
    printf("[END -- single_course_data_element_inner]\n\n");
}

void single_course_data_element_inner_1(TD::Scene& scene) {

}

void single_course_data_element_inner_2(TD::Scene& scene,
                                        uint16_t element_header_second_word)
{
    printf("\n                [single_course_data_element_inner_2]\n");
    printf("                0F68:2bb8\n");

    uint8_t byte_3a7f6 = 0;

    {
        printf("                    0F68:2bc6\n");
        uint8_t c0 = (rol_buffer[0x1d] & 0x80) >> 8;

        uint8_t c1 = (rol_buffer[0x1b] & 0x80) >> 8;
        rol_buffer[0x1b] = (rol_buffer[0x1b] << 1) | c0;

        uint8_t c2 = (rol_buffer[0x1c] & 0x80) >> 8;
        rol_buffer[0x1c] = (rol_buffer[0x1c] << 1) | c1;

        uint8_t c3 = (rol_buffer[0x1d] & 0x80) >> 8;
        rol_buffer[0x1d] = (rol_buffer[0x1d] << 1) | c2;

        if (c3) {
            printf("                        0F68:2bdc\n");

            byte_3a7f6 = byte_3a7f6 + 1;
        }
    }

}

void single_course_data_element(TD::Scene& scene,
                                int element_index,
                                uint16_t *output,
                                uint8_t *byte_3a8f2,
                                uint8_t *byte_3a8f3,
                                uint16_t *word_3a7ff)
{
    /*
    printf("\n            [single_course_data_element]\n");
    printf("            0F68:26fe\n");

    init_rol_buffer(scene, byte_3a8f2, byte_3a8f3);

    uint8_t  element_header_first_byte  = TD::GetByte(scene.one_dat, ((element_index * 8) + 0));
    uint8_t  element_header_second_byte = TD::GetByte(scene.one_dat, ((element_index * 8) + 1));

    uint16_t element_header_second_word = TD::GetWord(scene.one_dat, ((element_index * 8) + 2));
    uint16_t element_header_third_word  = TD::GetWord(scene.one_dat, ((element_index * 8) + 4));

    buffer2[*output] = element_header_first_byte;
    *output = *output + 1;

    *word_3a7ff = *output;
    *output = *output + 2;

    uint16_t word_3a7fd = *output;
    *output = *output + element_header_first_byte;

    if (*byte_3a8f3 < 1) {
        printf("                0F68:274b\n");
        single_course_data_element_inner_2(scene);
    }
    else {
        printf("                0F68:2746\n");
        single_course_data_element_inner_1(scene);
    }

    // ** //

    (word_3a7fd = word_3a7fd);
    (element_header_second_byte = element_header_second_byte);
    (element_header_second_word = element_header_second_word);
    (element_header_third_word = element_header_third_word);
     */
}

void single_course_data(TD::Scene& scene, int element_index, uint16_t *output) {
    printf("        [single_course_data]\n\n");

    printf("        0F68:2674  --  al = %2x\n", element_index);
    printf("        0F68:2677  --  cl = %2x\n", scene.getSingleCourseDataBoh());

    int single_course_data_boh = scene.getSingleCourseDataBoh();

    int some_offset = single_course_data_boh * 2;

    printf("        0F68:2681  --  cl = %2x\n", some_offset);

    uint8_t lut2 = scene.getCourseDataLut2(element_index);
    printf("        0F68:2685  --  ah = %2x\n", lut2);

    lut2 = lut2 & 7;
    printf("        0F68:2688  --  ah = %2x\n", lut2);

    if (lut2 == 0) {
        printf("        0F68:26AD taken\n");

        uint16_t bx = *output;
        printf("        0F68:26AF  -- bx = %4x\n", bx);

        *output += some_offset;
        printf("        0F68:26B1  -- di = %4x\n", *output);

        uint16_t word_3a805 = 0xffff;

        for (int i = 0; i < single_course_data_boh; i++) {
            buffer2[(bx + (i * 2)) + 0] = (*output & 0xff00) >> 8;
            buffer2[(bx + (i * 2)) + 1] = (*output & 0x00ff) >> 0;

            printf("            0F68:26B9  -- bx = %4x, di = %4x\n", bx + (i * 2), *output);


            uint8_t byte_3a8f2 = barfs_course_data_lut_1[i];
            uint8_t byte_3a8f3 = 0;

            auto saved_output = *output;

            uint16_t word_3a7ff = 0; // (output from _element)

            single_course_data_element(scene, element_index, output, &byte_3a8f2, &byte_3a8f3, &word_3a7ff);

            auto temp = ((buffer2[word_3a7ff + 0] << 8) +
                         (buffer2[word_3a7ff + 1] << 0));

            if (temp == word_3a805) {
                printf("                0F68:26E0 taken\n");
                *output = saved_output;

                buffer2[(bx + (i * 2)) + 0] = buffer2[(bx + ((i - 1) * 2)) + 0];
                buffer2[(bx + (i * 2)) + 1] = buffer2[(bx + ((i - 1) * 2)) + 1];
            }
            else {
                printf("                0F68:26EC taken\n");

                word_3a805 = temp;
            }

            printf("            0F68:26FB  -- ah = %2x bx = %4x\n", i + 1, bx + ((i + 1)* 2));
        }
    }
    else {
        printf("        0F68:268A taken\n");

        if (lut2 >= 2) {
            printf("            0F68:268F taken\n");

            uint8_t byte_3a8f2 = 0x17;
            uint8_t byte_3a8f3 = 0;

            uint16_t bx = *output;
            printf("        0F68:269B  -- bx = %4x\n", bx);

            *output += some_offset;
            printf("        0F68:269D  -- di = %4x\n", *output);

            for (int i = 0; i < single_course_data_boh; i++) {
                buffer2[(bx + (i * 2)) + 0] = (*output & 0xff00) >> 8;
                buffer2[(bx + (i * 2)) + 1] = (*output & 0x00ff) >> 0;
            }

            uint16_t word_3a7ff = 0; // (output from _element)
            single_course_data_element(scene, element_index, output, &byte_3a8f2, &byte_3a8f3, &word_3a7ff);
        }

        printf("            0F68:26AC\n");

    }
}

void barfs(TD::Scene& scene)
{
    auto unk1  = TD::GetWord(scene.one_dat, 0);
    auto unk2  = TD::GetWord(scene.one_dat, 2);
    auto unk3  = TD::GetWord(scene.one_dat, 4);
    auto count = TD::GetWord(scene.one_dat, 6);

    printf("scene011.dat > %04x %04x %04x %04x\n", unk1, unk2, unk3, count);
    printf("getSingleCourseDataBoh 0x%x\n\n", scene.getSingleCourseDataBoh());

    uint16_t output = 0x0040;

    for (int i = 0; i < count; i++) {
        printf("\n");

        printf("    0F68:263A  --  bx = %4x\n", i);
        printf("    0F68:263C  --  es:[bx] = di => %4x, %4x\n", i * 2, output);

        buffer2[(i * 2) + 0] = (output & 0xff00) >> 8;
        buffer2[(i * 2) + 1] = (output & 0x00ff) >> 0;

        if (i == 0)
            continue;

        single_course_data(scene, i, &output);
    }

}

