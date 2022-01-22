//
//  main.m
//  ray
//
//  Created by Antonio Malara on 11/10/2020.
//

#include <raylib.h>
#include <rlgl.h>

#include <cmath>
#include <utility>

#include "main.h"
#include "barfs.h"
#include "RaylibMesh.h"
#include "GameImage.h"
#include "Explorer.h"
#include "Images.h"

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

Vector3 NormalizeTDWorldLocation(TD::Point tdPos) {
    return Vector3 {
        .x = (tdPos.x * 4) / 4096.f - .5f,
        .y = (tdPos.z / 8) / 4096.f,
        .z = ((4096 * 16) - (tdPos.y * 4)) / 4096.f -.5f
    };
};

void printa(const TD::Model &model)
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

void print_sprite_data(const TD::Model &model)
{
    for (auto &sprite : model.sprites()) {
        printf("%04x %04x %04x %04x -- (%04x %04x %04x)\n",
               sprite.a, sprite.b, sprite.c, sprite.d,
               sprite.b & ~0xfc00, sprite.c & ~0xfc00, sprite.d & ~0xfc00);
    }
    
    printf("\n");
}

class SceneAssets {
public:
    SceneAssets(TD::Resources &resources)
        : m_resources(resources)
    {
        loadOTW();
        loadGenericTiles();
        loadSceneTiles();
        loadObjectModels();
    }

    TD::Scene& scene() {
        return m_resources.m_scenes[0];
    }

    const std::vector<TD::Model>& tileModels() {
        return m_tile_models;
    }

    std::vector<RayLibMesh>& tileMeshes() {
        return m_tile_meshes;
    }

    const std::vector<TD::Model>& objectModels() {
        return m_object_models;
    }

    std::vector<RayLibMesh>& objectMeshes() {
        return m_object_meshes;
    }

private:
    void loadOTW() {
        auto otwcolbin = m_resources.file("OTWCOL.BIN");
        auto otwcol = TD::PaletteFromData(otwcolbin);
        m_otwPalette.copy(otwcol, 0x10);
    }

    void loadGenericTiles() {
        for (auto &tileTdModel : m_resources.m_genericTiles) {
            m_tile_models.emplace_back(TD::Model(tileTdModel));
            m_tile_meshes.emplace_back(RayLibMesh(tileTdModel, m_otwPalette, scene()));
        }
    }

    void loadSceneTiles() {
        for (auto &tileTdModel : scene().tiles) {
            m_tile_models.emplace_back(TD::Model(tileTdModel));
            m_tile_meshes.emplace_back(RayLibMesh(tileTdModel, m_otwPalette, scene()));
        }
    }

    void loadObjectModels() {
        int DIOCAN = 0;
        for (auto &object : scene().m_objects) {
            if (DIOCAN == 1) continue;
            if (DIOCAN == 2) continue;
            if (DIOCAN == 3) continue;

            auto id = object.modelId();
            auto tdModel = TD::LoadModel(m_resources.m_scenetto, id, object.isLOD());

            m_object_models.emplace_back(TD::Model(tdModel));
            m_object_meshes.emplace_back(RayLibMesh(tdModel, m_otwPalette, scene()));
        }
    }

    TD::Resources& m_resources;
    TD::GamePalette m_otwPalette { };
    std::vector<RayLibMesh> m_generic_tiles;

    std::vector<TD::Model>  m_tile_models;
    std::vector<RayLibMesh> m_tile_meshes;

    std::vector<TD::Model>  m_object_models;
    std::vector<RayLibMesh> m_object_meshes;
};

struct ObjectAssets {
    ObjectAssets(TD::Resources& res,
                 TD::GamePalette& otwPalette,
                 TD::Scene& scene)
    {
        for (auto &tileTdModel : res.m_genericTiles)
            genericTiles.emplace_back(RayLibMesh(tileTdModel, otwPalette, scene));

        for (auto &tileTdModel : scene.tiles)
            tileMeshes.emplace_back(RayLibMesh(tileTdModel, otwPalette, scene));

        for (auto &i : res.m_genericObjects)
            objectMeshes.emplace_back(RayLibMesh(i, otwPalette, scene));

        for (auto &i : res.m_genericObjectsLod)
            objectLodMeshes.emplace_back(RayLibMesh(i, otwPalette, scene));

        for (auto &i : res.m_carModels)
            carMeshes.emplace_back(RayLibMesh(i, otwPalette, scene));
    }

    std::vector<RayLibMesh> genericTiles;
    std::vector<RayLibMesh> tileMeshes;
    std::vector<RayLibMesh> objectMeshes;
    std::vector<RayLibMesh> objectLodMeshes;
    std::vector<RayLibMesh> carMeshes;
};

class Screen {
public:
    virtual void setup() = 0;
    virtual void loop() = 0;
};

class ModelExplorer: public Screen {
public:
    ModelExplorer(SceneAssets &assets)
        : m_assets(assets)
        , m_explorer("MODELS EXPLORER", assets.objectMeshes().size())
    {
        m_explorer.setScale(100);
        m_explorer.setCurrent(86);
    }

    void setup() {
        SetCameraMode(m_explorer.camera(), CAMERA_PERSPECTIVE);
        printa(m_assets.objectModels()[m_explorer.current()]);
    }

    void loop() {
        auto meshNr = m_explorer.current();

        m_explorer.checkInput();

        if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_LEFT)) {
            printa(m_assets.objectModels()[meshNr]);
        }

        m_explorer.beginDrawingObject();

        DrawModelEx(m_assets.objectMeshes()[meshNr]._model(),
                    { 0 },
                    { 0, 1, 0 },
                    0,
                    { 1, 1, 1 },
                    ::WHITE);

        m_explorer.endDrawingObject();

        char sucaminchia[100];
        snprintf(sucaminchia, 100, "object %03lu - flags %04x", meshNr + 4, m_assets.scene().m_objects[meshNr+4].flags());
        DrawText(sucaminchia, 30, 80, 20, BLUE);

        EndDrawing();
    }

private:
    SceneAssets &m_assets;
    Explorer m_explorer;
};

class TilesExplorer: public Screen {
public:
    TilesExplorer(SceneAssets &assets)
        : m_assets(assets)
        , m_explorer("TILES EXPLORER", assets.tileMeshes().size())
    { }

    void setup() {
        SetCameraMode(m_explorer.camera(), CAMERA_PERSPECTIVE);
        printa(m_assets.tileModels()[0]);
    }

    void loop() {
        auto meshNr = m_explorer.current();

        m_explorer.checkInput();

        if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_LEFT)) {
            printf(" --- tile %zu ---\n", meshNr);
            print_sprite_data(m_assets.tileModels()[meshNr]);
        }

        BeginDrawing();

        m_explorer.beginDrawingObject();
        for (auto &sprite : m_assets.tileModels()[meshNr].sprites()) {
            Vector3 pos;
            pos.x =  ((int16_t) sprite.b) / 1024.;
            pos.y =  ((int16_t) sprite.d) / 4096.;
            pos.z = -((int16_t) sprite.c) / 1024.;

            DrawCylinder(pos, 0.001, 0.001, .2, 16, RED);
            DrawSphere( pos, 0.01, VIOLET);
        }

        DrawSphere({ .5, 0, 0 }, 0.05, GREEN);
        DrawSphere({ 0, 0, .5 }, 0.05, BLUE);

        DrawModel(m_assets.tileMeshes()[meshNr]._model(),
                  { 0 },
                  1,
                  ::WHITE);

        m_explorer.endDrawingObject();

        EndDrawing();
    }

private:
    SceneAssets &m_assets;
    Explorer m_explorer;
};

class BitmapTest: public Screen {
public:
    BitmapTest(TD::Resources &resources)
        : m_menuImages(resources)
    {
        for (auto& car : resources.cars()) {
            m_carImages.emplace_back(car);
        }
    }

    void setup() {
    }

    void loop() {
        BeginDrawing();

        ClearBackground(::DARKGRAY);

        DrawTexture(m_menuImages.select.texture(),       20,  20, ::WHITE);
        DrawTexture(m_menuImages.detail1.texture(),      20, 300, ::WHITE);
        DrawTexture(m_menuImages.detail2.texture(),      20, 350, ::WHITE);
        DrawTexture(m_menuImages.compass.texture(),         0, 0, ::WHITE);

        auto& carImages = m_carImages[2];

        DrawTexture(carImages.sic.texture(),     20, 270, ::WHITE);
        DrawTexture(carImages.top.texture(),    400,  20, ::WHITE);
        DrawTexture(carImages.bot1.texture(),   400,  60, ::WHITE);
        DrawTexture(carImages.bot2.texture(),   400, 120, ::WHITE);
        DrawTexture(carImages.lbot.texture(),   400, 180, ::WHITE);
        DrawTexture(carImages.rbot.texture(),   600, 180, ::WHITE);
        DrawTexture(carImages.etc.texture(),    400, 250, ::WHITE);

        DrawTexture(carImages.fl1.texture(),      20, 340, ::WHITE);
        DrawTexture(carImages.fl2.texture(),     260, 340, ::WHITE);

        DrawTexture(carImages.bic.texture(),     670, 340, ::WHITE);
        DrawTexture(carImages.sid.texture(),     550, 340, ::WHITE);
        DrawTexture(carImages.icn.texture(),     550, 490, ::WHITE);

        EndDrawing();
    }

private:
    TD::MenuImages m_menuImages;
    std::vector<TD::CarImages> m_carImages;
};

class CameraTest: public Screen {
public:
    CameraTest(TD::Resources& res, TD::Scene& scene)
        : m_scene(scene)
        , m_otwPalette(res.file("OTWCOL.BIN"), 0x10)
        , m_assets(res, m_otwPalette, m_scene)
    {
    }

    void resetCamera() {
        auto position = (Vector3) {
            ((0x2080 / 4096.f)) - .5f ,
            ((0x130 / 4096.f)  ),
            (((4096 * 16) - 0x4780) / 4096.f) - .5f
        };

        m_camera = {
            .position = position,
            .target = {
                position.x,
                position.y,
                position.z + 1.f,
            },
            .up = { 0.0f, 1.0f, 0.0f },
            .fovy = 45.0f,
            .projection = CAMERA_PERSPECTIVE,
        };

        SetCameraMode(m_camera, CAMERA_FIRST_PERSON);
    }

    void setup() {
        resetCamera();
    }

    void loop() {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (m_enableCamera) {
                m_enableCamera = false;
                ShowCursor();
                EnableCursor();
            }
            else {
                m_enableCamera = true;
                HideCursor();
                DisableCursor();
            }
        }

        if (m_enableCamera) {
            UpdateCamera(&m_camera);
            m_camera.position.y = ((0x130 / 4096.f) );
        }

        BeginDrawing();
        ClearBackground(DARKGRAY);
        BeginMode3D(m_camera);

        for (int y = 0; y < TD::Scene::YTileCount; y++) {
            for (int x = 0; x < TD::Scene::XTileCount; x++) {

                auto tileinfo = m_scene.getTileInfo(x, y);
                auto tileid = tileinfo.tileId();

                auto &model = tileid < 0x40
                    ? m_assets.genericTiles[tileid]
                    : m_assets.tileMeshes[tileid - 0x40];

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

        for (auto &i : m_scene.m_objects) {
            RayLibMesh * m;

            if (i.modelId() == 0) {
                continue;
            }
            else if (i.modelId() == 1) {
                m = &(m_assets.carMeshes[0]);
            }
            else if (i.modelId() == 2) {
                m = &(m_assets.carMeshes[2]);
            }
            else if (i.modelId() == 3) {
                m = &(m_assets.carMeshes[1]);
            }
            else if (i.isLOD()){
                m = &(m_assets.objectLodMeshes[i.modelId()]);
            }
            else {
                m = &(m_assets.objectMeshes[i.modelId()]);
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

private:
    TD::Scene &m_scene;
    TD::GamePalette m_otwPalette;
    ObjectAssets m_assets;
    Camera m_camera;
    bool m_enableCamera = true;
};

int mainTestBarfs()
{
    auto res = TD::Resources(BasePath());
    auto &scene = res.m_scenes[0];

    barfs(scene);
    exit(0);
}

int main()
{
//    mainTestBarfs();
    const int multiplicator = 3;

    const int screenWidth = TD3ScreenSizeWidth * multiplicator;
    const int screenHeight = TD3ScreenSizeHeight * multiplicator;

    auto resources = TD::Resources(BasePath());
    auto assets = SceneAssets(resources);

    auto cameraTest = CameraTest(resources, resources.m_scenes[0]);
    auto bitmapTest = BitmapTest(resources);
    auto modelExplorer = ModelExplorer(assets);
    auto tilesExplorer = TilesExplorer(assets);

    SetTargetFPS(30);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    
    InitWindow(screenWidth, screenHeight, "test");

    rlDisableBackfaceCulling();

    Screen* currentScreen = &cameraTest;
    currentScreen->setup();

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_ONE)) {
            currentScreen = &modelExplorer;
            currentScreen->setup();
        }

        if (IsKeyPressed(KEY_TWO)) {
            currentScreen = &tilesExplorer;
            currentScreen->setup();
        }

        if (IsKeyPressed(KEY_THREE)) {
            currentScreen = &bitmapTest;
            currentScreen->setup();
        }

        if (IsKeyPressed(KEY_FOUR)) {
            currentScreen = &cameraTest;
            currentScreen->setup();
        }

        currentScreen->loop();
    }
    
    return 0;
}
