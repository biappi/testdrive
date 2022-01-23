//
//  Explorer.h
//  testdrive
//
//  Created by Antonio Malara on 21/01/2022.
//

#pragma once

#define min(a, b) ((a < b) ? (a) : (b))
#define max(a, b) ((a < b) ? (b) : (a))

class Spinner {
public:
    Spinner(int size, int decreaseKey, int increaseKey)
        : m_size(size)
        , m_decreaseKey(decreaseKey)
        , m_increaseKey(increaseKey)
    { }

    void checkInput() {
        if (IsKeyPressed(m_increaseKey))
            m_current = min(m_current + 1, m_size - 1);

        if (IsKeyPressed(m_decreaseKey))
            m_current = max(m_current - 1, 0);
    }

    int current() const {
        return m_current;
    }

    int size() const {
        return m_size;
    }

private:
    const int m_decreaseKey;
    const int m_increaseKey;

    int m_size;
    int m_current;
};

class Explorer {
public:
    Explorer(std::string title, int size)
        : m_title(title)
        , m_spinner(size, KEY_LEFT, KEY_RIGHT)
    { }

    const Camera& camera() const {
        return m_camera;
    }

    void checkInput() {
        m_spinner.checkInput();
    }

    int current() const {
        return m_spinner.current();
    }

    int scale() const {
        return m_scale;
    }

    void setScale(int value) {
        m_scale = value;
    }

    void drawTitle() {
        char sucaminchia[100];
        snprintf(sucaminchia, 100, "%03d of %03d", m_spinner.current(), m_spinner.size());

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
    Spinner m_spinner;

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

