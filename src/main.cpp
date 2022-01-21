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


class DONTKNOWTHENAME {
public:
    DONTKNOWTHENAME(std::string path)
        : m_resources(path)
    {
        loadOTW();
        loadGenericTiles();
        loadSceneTiles();
        loadObjectModels();
    }

    TD::Scene& scene() {
        return m_resources.m_scenes[0];
    }

    /*
    void loadMeshes() {
        for (auto &mesh : m_tile_meshes) {
            mesh.load();
        }
    }
     */

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

    TD::Resources m_resources;
    TD::GamePalette m_otwPalette { };
    std::vector<RayLibMesh> m_generic_tiles;

    std::vector<TD::Model>  m_tile_models;
    std::vector<RayLibMesh> m_tile_meshes;

    std::vector<TD::Model>  m_object_models;
    std::vector<RayLibMesh> m_object_meshes;
};

class ModelExplorer {
public:
    ModelExplorer(DONTKNOWTHENAME &game)
        : m_game(game)
        , m_explorer("MODELS EXPLORER", game.objectMeshes().size())
    {
        m_explorer.setScale(100);
        m_explorer.setCurrent(86);
    }

    void setup() {
        SetCameraMode(m_explorer.camera(), CAMERA_PERSPECTIVE);
        printa(m_game.objectModels()[m_explorer.current()]);
    }

    void loop() {
        auto meshNr = m_explorer.current();

        m_explorer.checkInput();

        if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_LEFT)) {
            printa(m_game.objectModels()[meshNr]);
        }

        m_explorer.beginDrawingObject();

        DrawModelEx(m_game.objectMeshes()[meshNr]._model(),
                    { 0 },
                    { 0, 1, 0 },
                    0,
                    { 1, 1, 1 },
                    ::WHITE);

        m_explorer.endDrawingObject();

        char sucaminchia[100];
        snprintf(sucaminchia, 100, "object %03lu - flags %04x", meshNr + 4, m_game.scene().m_objects[meshNr+4].flags());
        DrawText(sucaminchia, 30, 80, 20, BLUE);

        EndDrawing();
    }

private:
    DONTKNOWTHENAME &m_game;
    Explorer m_explorer;
};

class TilesExplorer {
public:
    TilesExplorer(DONTKNOWTHENAME &game)
        : m_game(game)
        , m_explorer("TILES EXPLORER", game.tileMeshes().size())
    { }

    void setup() {
        SetCameraMode(m_explorer.camera(), CAMERA_PERSPECTIVE);
        printa(m_game.tileModels()[0]);
    }

    void loop() {
        auto meshNr = m_explorer.current();

        m_explorer.checkInput();

        if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_LEFT)) {
            printf(" --- tile %zu ---\n", meshNr);
            print_sprite_data(m_game.tileModels()[meshNr]);
        }

        BeginDrawing();

        m_explorer.beginDrawingObject();
        for (auto &sprite : m_game.tileModels()[meshNr].sprites()) {
            Vector3 pos;
            pos.x =  ((int16_t) sprite.b) / 1024.;
            pos.y =  ((int16_t) sprite.d) / 4096.;
            pos.z = -((int16_t) sprite.c) / 1024.;

            DrawCylinder(pos, 0.001, 0.001, .2, 16, RED);
            DrawSphere( pos, 0.01, VIOLET);
        }

        DrawSphere({ .5, 0, 0 }, 0.05, GREEN);
        DrawSphere({ 0, 0, .5 }, 0.05, BLUE);

        DrawModel(m_game.tileMeshes()[meshNr]._model(),
                  { 0 },
                  1,
                  ::WHITE);

        m_explorer.endDrawingObject();

        EndDrawing();
    }

private:
    DONTKNOWTHENAME &m_game;
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

int mainTilesExplorer()
{
    auto game = DONTKNOWTHENAME(BasePath());
    auto tilesExplorer = TilesExplorer(game);

    SetTargetFPS(30);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    InitWindow(800, 600, "test");

    rlDisableBackfaceCulling();
    tilesExplorer.setup();

    while (!WindowShouldClose()) {
        tilesExplorer.loop();
    }

    return 0;
}

int mainModelExplorer()
{
    auto game = DONTKNOWTHENAME(BasePath());
    auto modelExplorer = ModelExplorer(game);

    SetTargetFPS(30);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    
    InitWindow(800, 600, "test");

    rlDisableBackfaceCulling();
    modelExplorer.setup();

    while (!WindowShouldClose()) {
        modelExplorer.loop();
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

using namespace TD;

int mainTestBitmaps(void)
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

    auto compass = res.file("COMPASS.LZ");
    auto compassImage = GameImage(compass, 152, gamePalette);

    auto detail1 = res.file("DETAIL1.LZ");
    auto detail2 = res.file("DETAIL2.LZ");
    
    auto detail1Image = GameImage(detail1, 0x161, gamePalette);
    auto detail2Image = GameImage(detail2, 0x15d, gamePalette);
    
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
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
    };
    
    Texture2D checked = LoadTextureFromImage(checkedIm);
    UnloadImage(checkedIm);
    
    selectImage.texture();
    bot1Image.texture();
    bot2Image.texture();

    while (!WindowShouldClose())
    {
        BeginDrawing();
        {
            ClearBackground(::DARKGRAY);

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

//            DrawTexture(detail1Image.texture(), 20, 300, ::WHITE);
//            DrawTexture(detail2Image.texture(), 20, 350, ::WHITE);
            DrawTexture(compassImage.texture(), 0, 0, ::WHITE);
        }
        
        EndDrawing();
    }

    UnloadTexture(checked);

    selectImage.unloadTexture();
    
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
