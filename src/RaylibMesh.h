//
//  RaylibMesh.h
//  testdrive
//
//  Created by Antonio Malara on 21/01/2022.
//

#pragma once

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

        UploadMesh(&m_mesh, false);
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
