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
    
    // assert(all_tiles.size() == all_tiles_models.size());

    Camera camera = { 0 };
    camera.position = (Vector3){ 0.0f, 5.0f, 10.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

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


int mainModelExplorer()
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
    camera.projection = CAMERA_PERSPECTIVE;

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
