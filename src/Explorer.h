//
//  Explorer.h
//  testdrive
//
//  Created by Antonio Malara on 21/01/2022.
//

#pragma once

#define min(a, b) ((a < b) ? (a) : (b))
#define max(a, b) ((a < b) ? (b) : (a))

class Explorer {
public:
    Explorer(std::string title, size_t size)
        : m_title(title)
        , m_size(size)
    { }

    const Camera& camera() const {
        return m_camera;
    }

    void checkInput() {
        if (IsKeyPressed(KEY_RIGHT)) {
            m_current = max(0, min(m_current + 1, m_size - 1));
        }

        if (IsKeyPressed(KEY_LEFT))  {
            m_current = max(0, min(m_current - 1, m_size - 1));
        }
    }

    size_t current() const {
        return m_current;
    }

    void setCurrent(size_t value) {
        m_current = value;
    }

    int scale() const {
        return m_scale;
    }

    void setScale(int value) {
        m_scale = value;
    }

    void drawTitle() {
        char sucaminchia[100];
        snprintf(sucaminchia, 100, "%03lu of %03lu", m_current, m_size);

        DrawText(m_title.c_str(), 30, 30, 30, BLUE);
        DrawText(sucaminchia, 30, 60, 20, BLUE);
    }

    void beginDrawingObject() {
        ClearBackground(DARKGRAY);
        BeginMode3D(m_camera);

        DrawGrid(15, 1.0f);
        rlPushMatrix();

        rlScalef(m_scale, m_scale, m_scale);
        rlRotatef(m_rotation++, 0, 1, 0);
    }

    void endDrawingObject() {
        rlPopMatrix();
        EndMode3D();

        drawTitle();
    }

private:
    std::string m_title;
    size_t m_size;

    size_t m_current = 0;
    int m_rotation = 0;
    int m_scale = 10;

    Camera m_camera = {
        .position = (Vector3){ 0.0f, 5.0f, 10.0f },
        .target = (Vector3){ 0.0f, 0.0f, 0.0f },
        .up = (Vector3){ 0.0f, 1.0f, 0.0f },
        .fovy = 45.0f,
        .projection = CAMERA_PERSPECTIVE,
    };

};

