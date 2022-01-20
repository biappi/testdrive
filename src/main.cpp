//
//  main.m
//  ray
//
//  Created by Antonio Malara on 11/10/2020.
//

#include "raylib.h"
#include "rlgl.h"
#include "main.h"

#include <utility>

/*
 
 tiles are scaled to be 1x1 in gl world
 gl vetors { x, y, z }, y => up the monitor, z => inside the monitor
 
 //--//

 seg001 = 1a2
 seg006 = f68
 ds     = 20a6
 
 barfs_if_bad_course_data => seg006:25ed -- f68:25ed (ends at f68:2662)
 scratch_buffer           =>     ds:22de

 
 bp f68:25ed
 bp f68:2662
 
 
 at barfs:
    ds:scratch_buffer == contents of scene011.dat
 
 */

namespace TD {

class GameImage {
public:
    GameImage(const std::vector<std::byte> &imageLz, int width, GamePalette &palette, int colorBase = 0);
    Texture texture();
    void unloadTexture();
    
private:
    std::vector<TD::Color> m_bitmap;
    Image m_image;
    Texture2D m_texture;
    bool m_textureLoaded;
};

GameImage::GameImage(const std::vector<std::byte> &imageLz, int width, GamePalette &palette, int colorBase)
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
        .format = UNCOMPRESSED_R8G8B8A8,
        .mipmaps = 1,
    };
}

Texture2D GameImage::texture() {
    if (!m_textureLoaded) {
        m_texture = LoadTextureFromImage(m_image);
        m_textureLoaded = true;
    }
    
    return m_texture;
}

void GameImage::unloadTexture() {
    if (m_textureLoaded)
        UnloadTexture(m_texture);
}

};

class RayLibMesh {
public:
    
    RayLibMesh(const TD::Model &model,
               const TD::GamePalette &palette,
               const TD::Scene &scene)
        : m_model({0})
        , m_mesh({0})
        , m_loaded(false)
    {
        if (model.polys().size() == 0)
            return;
        
        for (auto &poly : model.polys()) {
            auto color = scene.mapColor(poly.color1(), poly.color0(), palette);
            
            switch (poly.type()) {
                case 0:
                case 1:
                    if (poly.idx0() >= model.points().size())
                        continue;

                    addDisc(model.points()[poly.idx0()], 3, 3, color);
                    break;
                
                case 2:
                case 3:
                    if ((poly.idx0() >= model.points().size()) ||
                        (poly.idx1() >= model.points().size()))
                    {
                        continue;
                    }
                
                    addLine(color, model.points()[poly.idx0()], model.points()[poly.idx1()]);
                    break;
                    
                case 4:
                case 5:
                    if ((poly.idx0() >= model.points().size()) ||
                        (poly.idx1() >= model.points().size()) ||
                        (poly.idx2() >= model.points().size()))
                    {
                        continue;
                    }

                    addTriangle(model, color, poly.idx0(), poly.idx1(), poly.idx2());
                    break;
                    
                case 6:
                case 7:
                    if ((poly.idx0() >= model.points().size()) ||
                        (poly.idx1() >= model.points().size()) ||
                        (poly.idx2() >= model.points().size()) ||
                        (poly.idx3() >= model.points().size()))
                    {
                        continue;
                    }
                    
                    addTriangle(model, color, poly.idx1(), poly.idx0(), poly.idx2());
                    addTriangle(model, color, poly.idx2(), poly.idx0(), poly.idx3());
                    break;
            }
        }
        
        m_mesh.vertices = &m_vertices[0];
        m_mesh.indices = &m_indices[0];
        m_mesh.colors = &m_colors[0];
        m_mesh.vboId = &m_vboId[0];
    }
    
    void load() {
        if (m_loaded)
            return;
        
        if (m_mesh.triangleCount == 0)
            return;
        
        rlLoadMesh(&m_mesh, false);
        m_model = LoadModelFromMesh(m_mesh);
        m_loaded = true;
    }
    
    const Model &_model() {
        if (!m_loaded)
            load();
        
        return m_model;
    }
    
private:
    unsigned short addVertex(const TD::Point &point, TD::Color color) {
        int idx = static_cast<int>(m_vertices.size());
        
        m_vertices.push_back(point.x  / 4096.);
        m_vertices.push_back(point.z  / 4096.);
        m_vertices.push_back(-point.y / 4096.);
        
        m_colors.push_back(color.r);
        m_colors.push_back(color.g);
        m_colors.push_back(color.b);
        m_colors.push_back(color.a);
        
        m_mesh.vertexCount = idx / 3 + 1;
        return idx / 3;
    }
    
    unsigned short addVertex(
        float x, float y, float z,
        const TD::Color &color, const TD::Point &t = {}
    ) {
        int idx = static_cast<int>(m_vertices.size());
        
        m_vertices.push_back((x  + t.x) / 4096.);
        m_vertices.push_back((z  + t.z) / 4096.);
        m_vertices.push_back((-y + t.y) / 4096.);
        
        m_colors.push_back(color.r);
        m_colors.push_back(color.g);
        m_colors.push_back(color.b);
        m_colors.push_back(color.a);
        
        m_mesh.vertexCount = idx / 3 + 1;
        return idx / 3;
    }

    void addTriangle(const TD::Model& model, const TD::Color& color, int idx1, int idx2, int idx3) {
        m_indices.push_back(addVertex(model.points()[idx1], color));
        m_indices.push_back(addVertex(model.points()[idx2], color));
        m_indices.push_back(addVertex(model.points()[idx3], color));
        m_mesh.triangleCount++;
    }

    void addTriangle(int idx1, int idx2, int idx3) {
        m_indices.push_back(idx1);
        m_indices.push_back(idx2);
        m_indices.push_back(idx3);
        m_mesh.triangleCount++;
    }

    void addQuad(int idx1, int idx2, int idx3, int idx4) {
        addTriangle(idx1, idx2, idx3);
        addTriangle(idx3, idx4, idx1);
    }
    
    void addLine(const TD::Color &color, const TD::Point a, const TD::Point b) {
        static const int width = 1;
        
        auto lbA = addVertex(TD::Point(a.x - width, a.y - width, a.z), color);
        auto rbA = addVertex(TD::Point(a.x + width, a.y - width, a.z), color);
        auto rtA = addVertex(TD::Point(a.x + width, a.y + width, a.z), color);
        auto ltA = addVertex(TD::Point(a.x - width, a.y + width, a.z), color);
        
        auto lbB = addVertex(TD::Point(b.x - width, b.y - width, b.z), color);
        auto rbB = addVertex(TD::Point(b.x + width, b.y - width, b.z), color);
        auto rtB = addVertex(TD::Point(b.x + width, b.y + width, b.z), color);
        auto ltB = addVertex(TD::Point(b.x - width, b.y + width, b.z), color);
        
        addQuad(lbA, rbA, rtA, ltA);
        addQuad(lbB, rbB, rtB, ltB);
        
        addQuad(lbA, rbA, rbB, lbB);
        addQuad(ltA, rtA, rtB, ltB);
        
        addQuad(lbA, ltA, ltB, ltB);
        addQuad(rbA, rtA, rtB, rbB);
    }

    void addDisc(TD::Point position, float height, float radius, TD::Color &c) {
        static int sectorCount = 10;
        
        auto &t = position;
        
        auto bottomCenter = addVertex(0, 0, -height * .5f, c, t);
        auto topCenter    = addVertex(0, 0, -height * .5f, c, t);
        
        for (int i = 0; i < sectorCount + 1; i++) {
            float step = 2 * M_PI / float(sectorCount);
            float angle = i * step;
            
            auto v1 = addVertex(radius * cosf(angle),        radius * sinf(angle),        -height * .5f, c, t);
            auto v2 = addVertex(radius * cosf(angle + step), radius * sinf(angle + step), -height * .5f, c, t);

            auto v3 = addVertex(radius * cosf(angle),        radius * sinf(angle),        +height * .5f, c, t);
            auto v4 = addVertex(radius * cosf(angle + step), radius * sinf(angle + step), +height * .5f, c, t);

            addTriangle(v1, v3, v2);
            addTriangle(v2, v3, v4);
            
            addTriangle(bottomCenter, v1, v2);
            addTriangle(topCenter, v3, v4);
        }
    }
    
    Mesh m_mesh;
    Model m_model;
    bool m_loaded;
    
    std::vector<float> m_vertices;
    std::vector<unsigned short> m_indices;
    std::vector<uint8_t> m_colors;
   
    unsigned int m_vboId[7] = { 0 };
};

Vector3 NormalizeTDWorldLocation(TD::Point tdPos) {
    return Vector3 {
        .x = (tdPos.x * 4) / 4096.f - .5f,
        .y = (tdPos.z / 8) / 4096.f,
        .z = ((4096 * 16) - (tdPos.y * 4)) / 4096.f -.5f
    };
};

#define min(a, b) ((a < b) ? (a) : (b))
#define max(a, b) ((a < b) ? (b) : (a))

void printa(TD::Model &model)
{
    if (model.polys().size() == 0)
        return;
    
    for (auto &poly : model.polys()) {
        switch (poly.type()) {

            case 2:
            case 3:


            case 4:
            case 5:
                break;
                
            case 6:
            case 7:
                break;

            default:
                printf("UNK POLY (type = %2x (%2x) - %2x %2x %2x %2x\n", // -- (%3d %3d %3d) (%3d %3d %3d) (%3d %3d %3d) (%3d %3d %3d)\n",
                       poly.type(), poly.role(),
                       poly.idx0(), poly.idx1(), poly.idx2(), poly.idx3()
//                       ,
//                       model.points()[poly.idx0()].x, model.points()[poly.idx0()].y, model.points()[poly.idx0()].z,
//                       model.points()[poly.idx1()].x, model.points()[poly.idx1()].y, model.points()[poly.idx1()].z,
//                       model.points()[poly.idx2()].x, model.points()[poly.idx2()].y, model.points()[poly.idx2()].z,
//                       model.points()[poly.idx3()].x, model.points()[poly.idx3()].y, model.points()[poly.idx3()].z
                       );
                
        }
    }
    printf("\n\n");
}

void print_sprite_data(TD::Model &model)
{
    for (auto &sprite : model.sprites()) {
        printf("%04x %04x %04x %04x -- (%04x %04x %04x)\n",
               sprite.a, sprite.b, sprite.c, sprite.d,
               sprite.b & ~0xfc00, sprite.c & ~0xfc00, sprite.d & ~0xfc00);
    }
    
    printf("\n");
}


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

int main___()
{
    auto res = TD::Resources(BasePath());
    auto &scene = res.m_scenes[0];

    barfs(scene);
    exit(0);
}

int main_______34534()
{
    SetTargetFPS(30);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    
    InitWindow(800, 600, "test");

    auto res = TD::Resources(BasePath());
    auto &scene = res.m_scenes[0];
        
    auto otwcolbin = res.file("OTWCOL.BIN");
    auto otwcol = TD::PaletteFromData(otwcolbin);
    auto otwPalette = TD::GamePalette();
    otwPalette.copy(otwcol, 0x10);
    
    std::vector<TD::Model>  all_tiles_models;
    std::vector<RayLibMesh> all_tiles;

    for (auto &tileTdModel : res.m_genericTiles) {
        all_tiles_models.emplace_back(TD::Model(tileTdModel));
        all_tiles.emplace_back(RayLibMesh(tileTdModel, otwPalette, scene));
        all_tiles.back().load();
    }
    
    for (auto &tileTdModel : scene.tiles) {
        all_tiles_models.emplace_back(TD::Model(tileTdModel));
        all_tiles.emplace_back(RayLibMesh(tileTdModel, otwPalette, scene));
        all_tiles.back().load();
    }
    
    assert(all_tiles.size() == all_tiles_models.size());

    Camera camera = { 0 };
    camera.position = (Vector3){ 0.0f, 5.0f, 10.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.type = CAMERA_PERSPECTIVE;

    SetCameraMode(camera, CAMERA_PERSPECTIVE);
    rlDisableBackfaceCulling();
    
    int i = 0;
    size_t meshNr = 0;
    int scale = 10;
    
    printa(all_tiles_models[meshNr]);
    
    while (!WindowShouldClose()) {
        
        if (IsKeyPressed(KEY_RIGHT)) {
            meshNr = max(0, min(meshNr + 1, all_tiles.size() - 1));
        }
        
        if (IsKeyPressed(KEY_LEFT))  {
            meshNr = max(0, min(meshNr - 1, all_tiles.size() - 1));
        }

        if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_LEFT)) {
            printf(" --- tile %zu ---\n", meshNr);
            print_sprite_data(all_tiles_models[meshNr]);
        }
        
//        if (IsKeyPressed(KEY_UP))  { ZZ-=1; }
//        if (IsKeyPressed(KEY_DOWN)) { ZZ+=1; }

        BeginDrawing();
        ClearBackground(DARKGRAY);
        BeginMode3D(camera);

        DrawGrid(15, 1.0f);

        rlPushMatrix();
        {
            rlScalef(scale, scale, scale);
            rlRotatef(i++, 0, 1, 0);
            
            for (auto &sprite : all_tiles_models[meshNr].sprites()) {
                Vector3 pos;
                pos.x =  ((int16_t) sprite.b) / 1024.;
                pos.y =  ((int16_t) sprite.d) / 4096.;
                pos.z = -((int16_t) sprite.c) / 1024.;
                
                DrawCylinder(pos, 0.001, 0.001, .2, 16, RED);
                DrawSphere( pos, 0.01, VIOLET);
            }
            
            DrawSphere({ .5, 0, 0 }, 0.05, GREEN);
            DrawSphere({ 0, 0, .5 }, 0.05, BLUE);
            
            DrawModel(all_tiles[meshNr]._model(),
                      { 0 },
                      1,
                      ::WHITE);
        }
        rlPopMatrix();

        EndMode3D();
        
        char sucaminchia[100];
        snprintf(sucaminchia, 100, "%03lu of %03lu", meshNr, all_tiles.size());
        
        DrawText("MODEL EXPLORER", 30, 30, 30, BLUE);
        DrawText(sucaminchia, 30, 60, 20, BLUE);

//        snprintf(sucaminchia, 100, "object %03lu - flags %04x", meshNr + 4, scene.m_objects[meshNr+4].flags());
//        DrawText(sucaminchia, 30, 80, 20, BLUE);
        
        EndDrawing();
    }
    
    return 0;
}


int main______()
{
    auto res = TD::Resources(BasePath());
    auto &scene = res.m_scenes[0];
    
    auto otwcolbin = res.file("OTWCOL.BIN");
    auto otwcol = TD::PaletteFromData(otwcolbin);
    auto otwPalette = TD::GamePalette();
    otwPalette.copy(otwcol, 0x10);
    
    std::vector<std::shared_ptr<TD::Model>> models;
    std::vector<std::shared_ptr<RayLibMesh>> meshes;
    
    int DIOCAN = 0;
    for (auto &object : scene.m_objects) {
        if (DIOCAN == 1) continue;
        if (DIOCAN == 2) continue;
        if (DIOCAN == 3) continue;
        
        auto id = object.modelId();
        auto model = std::make_shared<TD::Model>(TD::LoadModel(res.m_scenetto, id, object.isLOD()));
        auto mesh = std::make_shared<RayLibMesh>(RayLibMesh(*model, otwPalette, scene));
        models.push_back(model);
        meshes.emplace_back(mesh);
    }
    
    SetTargetFPS(30);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    
    InitWindow(800, 600, "test");

    Camera camera = { 0 };
        camera.position = (Vector3){ 0.0f, 5.0f, 10.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.type = CAMERA_PERSPECTIVE;

    SetCameraMode(camera, CAMERA_PERSPECTIVE);
    rlDisableBackfaceCulling();
    
    int i = 0;
    size_t meshNr = 86;
    int scale = 100;
    
    printa(*models[meshNr]);
    
    while (!WindowShouldClose()) {
        
        if (IsKeyPressed(KEY_RIGHT)) { meshNr++; }
        if (IsKeyPressed(KEY_LEFT))  { meshNr--; }

        if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_LEFT)) {
            printa(*models[meshNr]);
        }
        
        meshNr = min(max(meshNr, 0), meshes.size() - 1);
        
//        if (IsKeyPressed(KEY_UP))  { ZZ-=1; }
//        if (IsKeyPressed(KEY_DOWN)) { ZZ+=1; }

        BeginDrawing();
        ClearBackground(DARKGRAY);
        BeginMode3D(camera);

        DrawGrid(15, 1.0f);

        Vector3 scaleVector;
        scaleVector.x = scaleVector.y = scaleVector.z = scale;

        auto &model = meshes[meshNr]->_model();
        DrawModelEx(model,
                    { 0 },
                    { 0, 1, 0 },
                    i++,
                    scaleVector,
                    ::WHITE);

        EndMode3D();
        
        char sucaminchia[100];
        snprintf(sucaminchia, 100, "%03lu of %03lu", meshNr, meshes.size());
        
        DrawText("MODEL EXPLORER", 30, 30, 30, BLUE);
        DrawText(sucaminchia, 30, 60, 20, BLUE);

        snprintf(sucaminchia, 100, "object %03lu - flags %04x", meshNr + 4, scene.m_objects[meshNr+4].flags());
        DrawText(sucaminchia, 30, 80, 20, BLUE);
        
        EndDrawing();
    }
    
    return 0;
}

int main234234324234234234() {
//int main() {
    SetTargetFPS(30);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    
    InitWindow(800, 600, "test");
    
    Camera camera = { 0 };
    
    camera.position = (Vector3){
        ((0x2080 / 4096.f)) - .5f ,
        ((0x130 / 4096.f)  ),
        (((4096 * 16) - 0x4780) / 4096.f) - .5f};

    camera.target = camera.position;
    camera.target.z += 1;
    
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.type = CAMERA_PERSPECTIVE;

    auto res = TD::Resources(BasePath());
    
    auto otwcolbin = res.file("OTWCOL.BIN");
    auto otwcol = TD::PaletteFromData(otwcolbin);
    auto otwPalette = TD::GamePalette();
    otwPalette.copy(otwcol, 0x10);
    
    auto &scene = res.m_scenes[0];
    
    std::vector<RayLibMesh> genericTiles;
    std::vector<RayLibMesh> tileMeshes;
    std::vector<RayLibMesh> objectMeshes;
    std::vector<RayLibMesh> objectLodMeshes;
    std::vector<RayLibMesh> carMeshes;
    
    for (auto &tileTdModel : res.m_genericTiles) {
        genericTiles.emplace_back(RayLibMesh(tileTdModel, otwPalette, scene));
        genericTiles.back().load();
    }
    
    for (auto &tileTdModel : scene.tiles) {
        tileMeshes.emplace_back(RayLibMesh(tileTdModel, otwPalette, scene));
        tileMeshes.back().load();
    }

    for (auto &i : res.m_genericObjects) {
        objectMeshes.emplace_back(RayLibMesh(i, otwPalette, scene));
        objectMeshes.back().load();
    }

    for (auto &i : res.m_genericObjectsLod) {
        objectLodMeshes.emplace_back(RayLibMesh(i, otwPalette, scene));
        objectLodMeshes.back().load();
    }
    
    for (auto &i : res.m_carModels) {
        carMeshes.emplace_back(RayLibMesh(i, otwPalette, scene));
        carMeshes.back().load();
    }
    
    rlDisableBackfaceCulling();
    
    SetCameraMode(camera, CAMERA_FIRST_PERSON);
    
    while (!WindowShouldClose()) {
        UpdateCamera(&camera);
        camera.position.y = ((0x130 / 4096.f) );
        
        BeginDrawing();
        ClearBackground(DARKGRAY);
        BeginMode3D(camera);
        
        for (int y = 0; y < TD::Scene::YTileCount; y++) {
            for (int x = 0; x < TD::Scene::XTileCount; x++) {
                
                auto tileinfo = scene.getTileInfo(x, y);
                auto tileid = tileinfo.tileId();
                auto &model = tileid < 0x40 ? genericTiles[tileid] : tileMeshes[tileid - 0x40];
                
                Vector3 position;
                position.x = x;
                position.y = tileinfo.height() / 4096.;
                position.z = y;
                
                DrawModelEx(model._model(),
                            position,
                            { 0, 1, 0 },
                            -tileinfo.rot() * 90,
                            { 1, 1, 1 },
                            ::WHITE);
            }
        }
        
        for (auto &i : scene.m_objects) {
            RayLibMesh * m;
            
            if (i.modelId() == 0) {
                continue;
            }
            else if (i.modelId() == 1) {
                m = &carMeshes[0];
            }
            else if (i.modelId() == 2) {
                m = &carMeshes[2];
            }
            else if (i.modelId() == 3) {
                m = &carMeshes[1];
            }
            else if (i.isLOD()){
                m = &objectLodMeshes[i.modelId()];
            }
            else {
                m = &objectMeshes[i.modelId()];
            }
            
            DrawModelEx(m->_model(),
                        NormalizeTDWorldLocation(i.location()),
                        { 0, 1, 0 },
                        -(i.rotation()) * 90,
                        { 1, 1, 1 },
                        ::WHITE);
        };
        
        EndMode3D();
        EndDrawing();
    }

    return 0;
}

using namespace TD;

int main(void)
{
    auto res = Resources(BasePath());

    const int multiplicator = 3;
    
    const int screenWidth = TD3ScreenSizeWidth * multiplicator;
    const int screenHeight = TD3ScreenSizeHeight * multiplicator;
    
    auto t1 = res.file("SCENETT1.DAT");
    auto t = res.file("SCENETTT.BIN");
    auto p = res.file("SCENETTP.BIN");
    auto o = res.file("SCENETTO.BIN");
    
    auto selclr = res.file("SELCOLR.BIN");
    auto select = res.file("SELECT.LZ");

    auto gamePalette = GamePalette();

    auto selColrPalette = PaletteFromData(selclr);
    gamePalette.copy(selColrPalette, 0x10);
    
    auto selectImage = GameImage(select, 320, gamePalette);
    
    
    //    auto compass = res.file("COMPASS.LZ");
    //    auto compassImage = GameImage(compass, 152, gamePalette);

    auto detail1 = res.file("DETAIL1.LZ");
    auto detail2 = res.file("DETAIL2.LZ");
    
//    auto detail1Image = GameImage(detail1, 0x161, gamePalette);
//    auto detail2Image = GameImage(detail2, 0x15d, gamePalette);
    
    auto otwcolbin = res.file("OTWCOL.BIN");
    
    auto otwcol = PaletteFromData(otwcolbin);

    auto otwPalette = GamePalette();
    otwPalette.copy(otwcol, 0x10);
    
    auto &car = res.cars()[2];

    auto carsic = PaletteFromData(car.sicbin);
    auto carsicPalette = GamePalette();
    carsicPalette.copy(carsic, 0x40);

    auto carsicImage = GameImage(car.sic, 0x48, carsicPalette);
    auto carcol = PaletteFromData(car.col);
    auto carPalette = GamePalette();
    carPalette.copy(carcol, 0x10);
    
    
    auto topImage = GameImage(car.top, 320,   carPalette);
    auto bot1Image = GameImage(car.bot1, 320, carPalette);
    auto bot2Image = GameImage(car.bot2, 320, carPalette);
    auto lbotImage = GameImage(car.lbot, 168, carPalette);
    auto rbotImage = GameImage(car.rbot, 168, carPalette);
    auto etcImage = GameImage(car.etc, 56, carPalette);

    auto sccol = PaletteFromData(car.sc);
    auto scPalette = GamePalette();
    scPalette.copy(sccol, 0x10);
    
    auto fl1Image = GameImage(car.fl1, 208, scPalette);
    auto fl2Image = GameImage(car.fl2, 208, scPalette);
    auto bicImage = GameImage(car.bic, 112, scPalette);
    auto sidImage = GameImage(car.sid, 112, scPalette);
    auto icnImage = GameImage(car.icn, 208, scPalette);
    
    auto &scene = res.m_scenes[0];

    int tileid = 0;
    
    SetTargetFPS(30);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    
    InitWindow(screenWidth, screenHeight, "raylib [textures] example - texture from raw data");

  
    // Generate a checked texture by code
    int width = 960;
    int height = 480;

    // Dynamic memory allocation to store pixels data (Color type)
    ::Color *pixels = (::Color *)malloc(width*height*sizeof(::Color));

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            if (((x/32+y/32)/1)%2 == 0) pixels[y*width + x] = ::ORANGE;
            else pixels[y*width + x] = ::GOLD;
        }
    }

    // Load pixels data into an image structure and create texture
    Image checkedIm = {
        .data = pixels,             // We can assign pixels directly to data
        .width = width,
        .height = height,
        .format = UNCOMPRESSED_R8G8B8A8,
        .mipmaps = 1
    };
    
    Texture2D checked = LoadTextureFromImage(checkedIm);
    UnloadImage(checkedIm);
    
    selectImage.texture();
    bot1Image.texture();
    bot2Image.texture();
    
    Camera camera = { 0 };
    camera.position = (Vector3){ 0.0f, 10.0f, 10.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.type = CAMERA_PERSPECTIVE;

    SetCameraMode(camera, CAMERA_FIRST_PERSON);
    
    float XX = 0;
    float ZZ = 0;
    
    int rot = 0;
    
    while (!WindowShouldClose())
    {
        if (0)
        {
        if (IsKeyPressed(KEY_RIGHT)) { XX+=1; }
        if (IsKeyPressed(KEY_LEFT)) { XX-=1; }

        if (IsKeyPressed(KEY_UP))  { ZZ-=1; }
        if (IsKeyPressed(KEY_DOWN)) { ZZ+=1; }

        if (IsKeyPressed(KEY_A))  { rot = (rot - 1) % 4; }
        if (IsKeyPressed(KEY_D))  { rot = (rot + 1) % 4; }
        }
        
        BeginDrawing();
        {
            ClearBackground(::DARKGRAY);

//            DrawTexture(checked, screenWidth/2 - checked.width/2, screenHeight/2 - checked.height/2, Fade(WHITE, 0.5f));


            BeginMode3D(camera);
            {
                
//                DrawGrid(10, 1.0f);        // Draw a grid
            }
/*
            if (0)
            {
                DrawCube((Vector3){-4.0f, 0.0f, 2.0f}, 2.0f, 5.0f, 2.0f, RED);
                DrawCubeWires((Vector3){-4.0f, 0.0f, 2.0f}, 2.0f, 5.0f, 2.0f, GOLD);
                DrawCubeWires((Vector3){-4.0f, 0.0f, -2.0f}, 3.0f, 6.0f, 2.0f, MAROON);

                DrawSphere((Vector3){-1.0f, 0.0f, -2.0f}, 1.0f, GREEN);
                DrawSphereWires((Vector3){1.0f, 0.0f, 2.0f}, 2.0f, 16, 16, LIME);

                DrawCylinder((Vector3){4.0f, 0.0f, -2.0f}, 1.0f, 2.0f, 3.0f, 4, SKYBLUE);
                DrawCylinderWires((Vector3){4.0f, 0.0f, -2.0f}, 1.0f, 2.0f, 3.0f, 4, DARKBLUE);
                DrawCylinderWires((Vector3){4.5f, -1.0f, 2.0f}, 1.0f, 1.0f, 2.0f, 6, BROWN);

                DrawCylinder((Vector3){1.0f, 0.0f, -4.0f}, 0.0f, 1.5f, 3.0f, 8, GOLD);
                DrawCylinderWires((Vector3){1.0f, 0.0f, -4.0f}, 0.0f, 1.5f, 3.0f, 8, PINK);

//                DrawGrid(10, 1.0f);        // Draw a grid
                
            }
 */
            EndMode3D();


                DrawTexture(selectImage.texture(),  20,  20, ::WHITE);
                
                DrawTexture(topImage.texture(),    400,  20, ::WHITE);
                DrawTexture(bot1Image.texture(),   400,  60, ::WHITE);
                DrawTexture(bot2Image.texture(),   400, 120, ::WHITE);
                DrawTexture(lbotImage.texture(),   400, 180, ::WHITE);
                DrawTexture(rbotImage.texture(),   600, 180, ::WHITE);
                DrawTexture(etcImage.texture(),    400, 250, ::WHITE);
                
                DrawTexture(carsicImage.texture(),  20, 270, ::WHITE);
                
                DrawTexture(fl1Image.texture(),     20, 340, ::WHITE);
                DrawTexture(fl2Image.texture(),    260, 340, ::WHITE);
                
                DrawTexture(bicImage.texture(),    670, 340, ::WHITE);
                DrawTexture(sidImage.texture(),    550, 340, ::WHITE);
                DrawTexture(icnImage.texture(),    550, 490, ::WHITE);

            /*
            DrawTexture(detail1Image.texture(), 20, 300, WHITE);
            DrawTexture(detail2Image.texture(), 20, 350, WHITE);
            DrawTexture(compassImage.texture(), 400, 160, WHITE);
            */
            
            /*
            DrawText("CHECKED TEXTURE ", 84, 85, 30, BROWN);
            DrawText("GENERATED by CODE", 72, 148, 30, BROWN);
            DrawText("and RAW IMAGE LOADING", 46, 210, 30, BROWN);
             */
            
            DrawText("(c) Fudesumi sprite by Eiden Marsal", 310, screenHeight - 20, 10, ::BROWN);
        }
        
        EndDrawing();
    }

    UnloadTexture(checked);

    selectImage.unloadTexture();
    
    CloseWindow();

    return 0;
}

