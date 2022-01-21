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

class ModelExplorer {
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

class TilesExplorer {
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

#pragma mark - Main Functions

int mainTestBarfs()
{
    auto res = TD::Resources(BasePath());
    auto &scene = res.m_scenes[0];

    barfs(scene);
    exit(0);
}

enum Screens {
    SCREEN_MODEL_EXPLORER,
    SCREEN_TILES_EXPLORER,
};

int mainExplorers()
{
    auto resources = TD::Resources(BasePath());
    auto game = SceneAssets(resources);
    auto modelExplorer = ModelExplorer(game);
    auto tilesExplorer = TilesExplorer(game);

    SetTargetFPS(30);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    
    InitWindow(800, 600, "test");

    rlDisableBackfaceCulling();
    modelExplorer.setup();
    tilesExplorer.setup();

    auto currentScreen = SCREEN_MODEL_EXPLORER;

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_ONE))
            currentScreen = SCREEN_MODEL_EXPLORER;

        if (IsKeyPressed(KEY_TWO))
            currentScreen = SCREEN_TILES_EXPLORER;

        switch (currentScreen) {
            case SCREEN_MODEL_EXPLORER:
                modelExplorer.loop();
                break;

            case SCREEN_TILES_EXPLORER:
                tilesExplorer.loop();
                break;
        }
    }
    
    return 0;
}

int mainTestCamera() {
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
    camera.projection = CAMERA_PERSPECTIVE;

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

    bool enableCamera = true;

    while (!WindowShouldClose()) {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (enableCamera) {
                enableCamera = false;
                ShowCursor();
                EnableCursor();
            }
            else {
                enableCamera = true;
                HideCursor();
                DisableCursor();
            }
        }

        if (enableCamera) {
            UpdateCamera(&camera);
            camera.position.y = ((0x130 / 4096.f) );
        }

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

namespace TD {

struct CarImages {
    CarImages(const Car &car, const GamePalette &carPalette)
        : top (car.top,  320, carPalette)
        , bot1(car.bot1, 320, carPalette)
        , bot2(car.bot2, 320, carPalette)
        , lbot(car.lbot, 168, carPalette)
        , rbot(car.rbot, 168, carPalette)
        , etc (car.etc,   56, carPalette)
    { }

    GameImage top;
    GameImage bot1;
    GameImage bot2;
    GameImage lbot;
    GameImage rbot;
    GameImage etc;
};

struct CarSCImages {
    CarSCImages(const Car &car, const GamePalette &scPalette)
        : fl1(car.fl1, 208, scPalette)
        , fl2(car.fl2, 208, scPalette)
        , bic(car.bic, 112, scPalette)
        , sid(car.sid, 112, scPalette)
        , icn(car.icn, 208, scPalette)
    { }

    GameImage fl1;
    GameImage fl2;
    GameImage bic;
    GameImage sid;
    GameImage icn;
};

};

using namespace TD;

int mainTestBitmaps(void)
{
    auto res = Resources(BasePath());

    const int multiplicator = 3;
    
    const int screenWidth = TD3ScreenSizeWidth * multiplicator;
    const int screenHeight = TD3ScreenSizeHeight * multiplicator;

    auto selclr = res.file("SELCOLR.BIN");
    auto select = res.file("SELECT.LZ");

    auto otwcolbin = res.file("OTWCOL.BIN");

    auto gamePalette = GamePaletteFromData(selclr, 0x10);
    auto selectImage = GameImage(select, 320, gamePalette);

    auto compass = res.file("COMPASS.LZ");
    auto compassImage = GameImage(compass, 152, gamePalette);

    auto detail1 = res.file("DETAIL1.LZ");
    auto detail2 = res.file("DETAIL2.LZ");
    
    auto detail1Image = GameImage(detail1, 0x161, gamePalette);
    auto detail2Image = GameImage(detail2, 0x15d, gamePalette);

    auto &car = res.cars()[2];

    auto carsicPalette = GamePaletteFromData(car.sicbin, 0x40);
    auto carPalette = GamePaletteFromData(car.col, 0x10);
    auto scPalette = GamePaletteFromData(car.sc, 0x10);

    auto carsicImage = GameImage(car.sic, 0x48, carsicPalette);

    auto carImages = CarImages(car, carPalette);
    auto scImages = CarSCImages(car, scPalette);

    SetTargetFPS(30);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    
    InitWindow(screenWidth, screenHeight, "raylib [textures] example - texture from raw data");
  
    while (!WindowShouldClose())
    {
        BeginDrawing();
        {
            ClearBackground(::DARKGRAY);

            DrawTexture(selectImage.texture(),  20,  20, ::WHITE);
                
            DrawTexture(carImages.top.texture(),    400,  20, ::WHITE);
            DrawTexture(carImages.bot1.texture(),   400,  60, ::WHITE);
            DrawTexture(carImages.bot2.texture(),   400, 120, ::WHITE);
            DrawTexture(carImages.lbot.texture(),   400, 180, ::WHITE);
            DrawTexture(carImages.rbot.texture(),   600, 180, ::WHITE);
            DrawTexture(carImages.etc.texture(),    400, 250, ::WHITE);
                
            DrawTexture(carsicImage.texture(),  20, 270, ::WHITE);
                
            DrawTexture(scImages.fl1.texture(),     20, 340, ::WHITE);
            DrawTexture(scImages.fl2.texture(),    260, 340, ::WHITE);

            DrawTexture(scImages.bic.texture(),    670, 340, ::WHITE);
            DrawTexture(scImages.sid.texture(),    550, 340, ::WHITE);
            DrawTexture(scImages.icn.texture(),    550, 490, ::WHITE);

//            DrawTexture(detail1Image.texture(), 20, 300, ::WHITE);
//            DrawTexture(detail2Image.texture(), 20, 350, ::WHITE);
            DrawTexture(compassImage.texture(), 0, 0, ::WHITE);
        }
        
        EndDrawing();
    }

    CloseWindow();

    return 0;
}

#pragma mark - Main

int main() {
//    mainTestCamera();

//    mainTestBarfs();
    mainTestBitmaps();
//    mainModelExplorer();
//    mainTilesExplorer();
}
