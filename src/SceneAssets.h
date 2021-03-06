//
//  SceneAssets.h
//  testdrive
//
//  Created by Antonio Malara on 22/01/2022.
//

#pragma once

#import "Resources.h"
#import "RaylibMesh.h"

struct SceneAssets {
    SceneAssets(TD::Resources& res,
                TD::GamePalette& otwPalette,
                TD::Scene& scene)
    {
        for (auto &tileTdModel : res.m_genericTiles) {
            genericTiles.emplace_back(RayLibMesh(tileTdModel, otwPalette, scene));

            tileExplorerMeshes.emplace_back(RayLibMesh(tileTdModel, otwPalette, scene));
            tileExplorerModels.emplace_back(TD::Model(tileTdModel));
        }

        for (auto &tileTdModel : scene.tiles) {
            tileMeshes.emplace_back(RayLibMesh(tileTdModel, otwPalette, scene));

            tileExplorerMeshes.emplace_back(RayLibMesh(tileTdModel, otwPalette, scene));
            tileExplorerModels.emplace_back(TD::Model(tileTdModel));
        }

        for (auto &i : res.m_genericObjects) {
            objectMeshes.emplace_back(RayLibMesh(i, otwPalette, scene));

            modelExplorerMeshes.emplace_back(RayLibMesh(i, otwPalette, scene));
            modelExplorerModels.emplace_back(TD::Model(i));
        }

        for (auto &i : res.m_genericObjectsLod)
            objectLodMeshes.emplace_back(RayLibMesh(i, otwPalette, scene));

        for (auto &i : res.m_carModels) {
            carMeshes.emplace_back(RayLibMesh(i, otwPalette, scene));
        }
    }

    RayLibMesh* meshForModelId(int modelId, bool isLOD) {
        if (modelId == 0) {
            return nullptr;
        }
        else if (modelId == 1) {
            return &(carMeshes[0]);
        }
        else if (modelId == 2) {
            return &(carMeshes[2]);
        }
        else if (modelId == 3) {
            return &(carMeshes[1]);
        }
        else if (isLOD){
            return &(objectLodMeshes[modelId]);
        }
        else {
            return &(objectMeshes[modelId]);
        }
    }

    std::vector<RayLibMesh> genericTiles;
    std::vector<RayLibMesh> tileMeshes;
    std::vector<RayLibMesh> objectMeshes;
    std::vector<RayLibMesh> objectLodMeshes;
    std::vector<RayLibMesh> carMeshes;

    std::vector<TD::Model>  tileExplorerModels;
    std::vector<RayLibMesh> tileExplorerMeshes;

    std::vector<TD::Model>  modelExplorerModels;
    std::vector<RayLibMesh> modelExplorerMeshes;
};

