//
//  Resources.h
//  testdrive
//
//  Created by Antonio Malara on 22/01/2022.
//

#include "Scene.h"

namespace TD {

class PlayDisk {
public:
    void load(FILE * file);
    std::vector<std::string> carNames();
    std::vector<std::string> trackNames();
    
private:
    char name[18];
    char cars[14][6];
    char tracks[8][8];
    uint16_t boh1[4];
    uint8_t boh2[4];
};

void PlayDisk::load(FILE *file) {
    fread(&name, 1, 18, file);
    fread(&cars, 14, 6, file);
    fread(&tracks, 8, 8, file);
    fread(&boh1, 2, 4, file);
    fread(&boh2, 1, 4, file);
    
    for (int i = 0; i < 14; i++) {
        for (int k = 0; k < 6; k++) {
            cars[i][k] = tolower(cars[i][k]);
        }
    }
    
    for (int i = 0; i < 8; i++) {
        for (int k = 0; k < 8; k++) {
            tracks[i][k] = tolower(tracks[i][k]);
        }
    }
}

std::vector<std::string> PlayDisk::carNames() {
    std::vector<std::string> names;
 
    for (int i = 0; i < 14; i++) {
        if (cars[i][0] == 'c') {
            names.push_back(cars[i]);
        }
    }
    
    return names;
}

std::vector<std::string> PlayDisk::trackNames() {
    std::vector<std::string> names;
 
    for (int i = 0; i < 8; i++) {
        if (tracks[i][0] == 's') {
            names.push_back(tracks[i]);
        }
    }
    
    return names;
}

class PackedFileDesc {
public:
    uint16_t hash1;
    uint16_t hash2;
    char dataFile;
    uint32_t start;
    uint32_t size;

    void load(FILE *file);
};

void PackedFileDesc::load(FILE *file) {
    char padding;
    
    fread(&hash1, 2, 1, file);
    fread(&hash2, 2, 1, file);
    fread(&dataFile, 1, 1, file);
    fread(&padding, 1, 1, file);
    fread(&start, 4, 1, file);
    fread(&size, 4, 1, file);
}

class CarLst {
public:
    void load(FILE *file);
    
    char name[19];
    uint16_t boh1[13];
    uint8_t boh2[66];
    uint8_t boh3[7];
    uint16_t boh4[5];
    uint8_t boh5[10];
    uint16_t boh6[20];
    uint8_t boh7[217];
    uint16_t boh8[35];
    PackedFileDesc files[15];
};

void CarLst::load(FILE * file) {
    fread(&name, 1, 19, file);
    fread(&boh1, 2, 13, file);
    fread(&boh2, 1, 66, file);
    fread(&boh3, 1, 7, file);
    fread(&boh4, 2, 5, file);
    fread(&boh5, 1, 10, file);
    fread(&boh6, 2, 20, file);
    fread(&boh7, 1, 217, file);
    fread(&boh8, 2, 35, file);
    
    for (int i = 0; i < 15; i++) {
        files[i].load(file);
    }
}

struct Car {
    std::vector<std::byte> sicbin;
    std::vector<std::byte> sic;
   
    std::vector<std::byte> col;
    std::vector<std::byte> top;
    std::vector<std::byte> bot1;
    std::vector<std::byte> bot2;
    std::vector<std::byte> lbot;
    std::vector<std::byte> rbot;
    std::vector<std::byte> etc;
    
    std::vector<std::byte> sc;
    std::vector<std::byte> fl1;
    std::vector<std::byte> fl2;
    std::vector<std::byte> bic;
    std::vector<std::byte> sid;
    std::vector<std::byte> icn;
};

class SceneLst {
public:
    void load(FILE *file);
    
    char name[0x13];
    char boh1;
    uint16_t boh2[0x1d];
    char boh3[0x0a];
    char boh4[0x1b0];
    char boh5[0x2c8];
    PackedFileDesc files[29];
};
 
void SceneLst::load(FILE *file) {
    fread(&name, 0x13, 1, file);
    fread(&boh1, 1, 1, file);
    fread(&boh2, 2, 0x1d, file);
    fread(&boh3, 1, 0x0a, file);
    fread(&boh4, 1, 0x1b0, file);
    fread(&boh5, 1, 0x2c8, file);
    
    for (int i = 0; i < 15; i++) {
        files[i].load(file);
    }
}

class Resources {
public:
    Resources(std::string basePath);

    const std::vector<std::byte> file(const std::string &name) const;
    const std::vector<std::byte> fileForCar(const std::string &name, const std::string &lowerCarName, const CarLst &carLst);
    const std::vector<std::byte> fileForScene(const std::string &name, const std::string &lowerCarName, const SceneLst &sceneLst);

    const std::vector<Car>& cars() { return carsArray; }
    
private:
    std::string basePath;

    PlayDisk playdisk;
    std::vector<PackedFileDesc> files;
    
public:
    std::vector<Car> carsArray;
    std::vector<Scene> m_scenes;
    std::vector<Model> m_genericTiles;
    std::vector<Model> m_genericObjects;
    std::vector<Model> m_genericObjectsLod;
    std::vector<Model> m_carModels;
    
    std::vector<std::byte> m_scenetto;
};

Resources::Resources(const std::string basePath)
    : basePath(basePath)
{
    auto exeFilePath = basePath + "/" + "td3.exe";
    auto exeFile = fopen(exeFilePath.c_str(), "r");
    fseek(exeFile, 0x20ae2, SEEK_SET);
    
    while (true) {
        PackedFileDesc desc;
        desc.load(exeFile);
        
        if (desc.hash1) {
            files.push_back(desc);
        }
        else {
            break;
        }
    }
    fclose(exeFile);

    auto playdiskPath = basePath + "/" + "playdisk.dat";
    auto playdiskFile = fopen(playdiskPath.c_str(), "r");
    playdisk.load(playdiskFile);
    fclose(playdiskFile);
    
    for (auto carName : playdisk.carNames()) {
        auto path = basePath + "/" + carName + ".lst";
        auto file = fopen(path.c_str(), "r");

        CarLst lst;
        lst.load(file);
        fclose(file);

        Car car;
        car.col = fileForCar("COL.BIN", carName, lst);
        
        car.sicbin = fileForCar("SIC.BIN", carName, lst);
        car.sic = fileForCar(".SIC", carName, lst);
        
        car.top = fileForCar(".TOP", carName, lst);
        car.bot1 = fileForCar("1.BOT", carName, lst);
        car.bot2 = fileForCar("2.BOT", carName, lst);
        car.lbot = fileForCar("L.BOT", carName, lst);
        car.rbot = fileForCar("R.BOT", carName, lst);
        car.etc = fileForCar(".ETC", carName, lst);
        
        car.sc = fileForCar("SC.BIN", carName, lst);
        car.fl1 = fileForCar("FL1.LZ", carName, lst);
        car.fl2 = fileForCar("FL2.LZ", carName, lst);
        car.bic = fileForCar(".BIC", carName, lst);
        car.sid = fileForCar(".SID", carName, lst);
        car.icn = fileForCar(".ICN", carName, lst);

        carsArray.push_back(car);
        
        auto pobFilename = basePath + "/" + carName + ".pob";
        auto pobFile = fopen(pobFilename.c_str(), "r");
        fseek(pobFile, 0, SEEK_END);

        auto len = ftell(pobFile);
        fseek(pobFile, 0, SEEK_SET);
        
        std::vector<std::byte> pobData(len);
        fread(&pobData[0], len, 1, pobFile);
        fclose(pobFile);

        m_carModels.push_back(Model(pobData, 0, true));
    }
    
    for (auto sceneName : playdisk.trackNames()) {
        auto path = basePath + "/" + sceneName + ".lst";
        auto file = fopen(path.c_str(), "r");

        SceneLst lst;
        lst.load(file);
        fclose(file);

        Scene scene;
        // a -> subcourse + 'A'
        scene.a_dat = fileForScene("A.DAT", sceneName, lst);
        scene.one_dat = fileForScene("1.DAT", sceneName, lst);
        
        scene.t_bin = fileForScene("T.BIN", sceneName, lst);
        
        //not used
        scene.o_bin = fileForScene("O.BIN", sceneName, lst);
        scene.p_bin = fileForScene("P.BIN", sceneName, lst);
        
        scene.tiles = LoadModels(scene.t_bin, 64);
        
        scene.loadObjectData(scene.a_dat);

        m_scenes.push_back(scene);
    }
    
    auto genericTilesFile = file("SCENETTT.BIN");
    m_genericTiles = LoadModels(genericTilesFile, 64);
    
    m_scenetto = file("SCENETTO.BIN");
    m_genericObjects = LoadModels(m_scenetto, 64);
    m_genericObjectsLod = LoadModels(m_scenetto, 64, true);
}

uint16_t Hash2(const std::string &name) {
    uint16_t h = 0;

    for (long i = name.length() - 1; i >= 0; i--) {
        h = (0x101 * h) + name[i];
    }
    
    return h;
}

uint16_t Hash1(const std::string &name) {
    uint16_t h = 0;
    
    for (long i = 0; i < name.length() - 1; i++) {
        h += name[i] * i;
    }
    
    return h;
}

std::optional<PackedFileDesc> FindFileDesc(const std::string &name, const std::vector<PackedFileDesc> descs) {
    auto h1 = Hash1(name);
    auto h2 = Hash2(name);

    for (auto desc : descs) {
        if ((desc.hash1 == h1) && (desc.hash2 == h2)) {
            return desc;
        }
    }
    
    return {};
}

const std::vector<std::byte> Resources::file(const std::string &name) const {
    const std::map<char, std::string> char_to_files = {
        {'a', "dataa.dat"},
        {'b', "datab.dat"},
        {'c', "datac.dat"},
    };
    
    if (const auto desc = FindFileDesc(name, files)) {
        std::vector<std::byte> ret(desc->size);
        
        auto fileName = char_to_files.at(desc->dataFile);
        auto path = basePath + "/" + fileName;
        
//        printf("%s %s %c %x %x\n", (name).c_str(), path.c_str(), desc->dataFile,desc->start, desc->size);

        auto file = fopen(path.c_str(), "r");
        fseek(file, desc->start, SEEK_SET);
        fread(&ret[0], 1, desc->size, file);
        fclose(file);
        
        return ret;
    }

    return {};
}

const std::vector<std::byte> Resources::fileForCar(const std::string &name, const std::string &lowerCarName, const CarLst &carLst) {
    std::vector<PackedFileDesc> files;
    
    for (int i = 0; i < sizeof(carLst.files) / sizeof(PackedFileDesc); i++) {
        files.push_back(carLst.files[i]);
    }
    
    std::string carNameUpper;
    for (auto c : lowerCarName) {
        carNameUpper.push_back(toupper(c));
    }
    
    if (const auto desc = FindFileDesc(carNameUpper + name, files)) {
        std::vector<std::byte> ret(desc->size);
        
        auto path = basePath + "/" + lowerCarName + ".dat";
        
        auto file = fopen(path.c_str(), "r");
        fseek(file, desc->start, SEEK_SET);
        fread(&ret[0], 1, desc->size, file);
        fclose(file);
        
        return ret;
    }

    return {};
}

const std::vector<std::byte> Resources::fileForScene(const std::string &name, const std::string &lowerSceneName, const SceneLst &sceneLst) {
    std::vector<PackedFileDesc> files;
    
    for (int i = 0; i < sizeof(sceneLst.files) / sizeof(PackedFileDesc); i++) {
        files.push_back(sceneLst.files[i]);
    }
    
    std::string upperSceneName;
    for (auto c : lowerSceneName) {
        upperSceneName.push_back(toupper(c));
    }
    
    if (const auto desc = FindFileDesc(upperSceneName + name, files)) {
        std::vector<std::byte> ret(desc->size);
        
        auto path = basePath + "/" + lowerSceneName + ".dat";
        
//       printf("%s %s %c %x %x\n", (upperSceneName + name).c_str(), path.c_str(), desc->dataFile,desc->start, desc->size);
        
        auto file = fopen(path.c_str(), "r");
        fseek(file, desc->start, SEEK_SET);
        fread(&ret[0], 1, desc->size, file);
        fclose(file);
        
        return ret;
    }
    
    return {};
}


}
