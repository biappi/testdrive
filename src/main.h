// #include "raylib.h"
// #include "rlgl.h"

#include <cstddef>
#include <cstdlib>
#include <string>
#include <vector>
#include <map>
#include <memory>

#include "Decoders.h"
#include "Models.h"

const int TD3ScreenSizeWidth  = 320;
const int TD3ScreenSizeHeight = 200;

std::string BasePath() {
    return "data/";
}

namespace TD {

struct Color {
    uint8_t r, g, b, a;
};

class GamePalette {
public:
    std::vector<Color> palette;

    GamePalette();
    void copy(std::vector<Color> palette, int at);
};

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
    CarLst lst;
    
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

struct TileInfo {
    
    int tileId() const {
        return std::to_integer<int>(tile);
    }
    
    int rot() const {
        return std::to_integer<int>(info) >> 6;
    }

    int height() const {
        return (std::to_integer<int>(info) & 0x3f) * 250;
    }

    std::byte tile;
    std::byte info;
};

class GameObject {
public:
    GameObject(uint16_t objectId,
               uint16_t x,
               uint16_t y,
               uint16_t z,
               uint16_t rotation)
        : m_objectId(objectId)
        , m_x(x), m_y(y), m_z(z)
        , m_rotation(rotation)
    {
    }
    
    uint16_t modelId()   const { return m_objectId & 0x003f; }
    uint16_t flags()     const { return m_objectId & 0xffc0; }
    bool     isLOD()     const { return m_objectId & 0x3000; }

    Point    location()  const { return Point(m_x, m_y, m_z); }
    int      rotation()  const { return m_rotation >> 6; }
        
private:
    uint16_t m_objectId;
    uint16_t m_x;
    uint16_t m_y;
    uint16_t m_z;
    uint16_t m_rotation;
};

class Scene {
   
private:
    static const int tta_dseg_start_offset = 0x9370;
    
public:
    static const int XTileCount = 32;
    static const int YTileCount = 16;
    
    int getSingleCourseDataBoh() {
        static int offset = 0x939d - tta_dseg_start_offset;
        return std::to_integer<uint8_t>(a_dat[offset]);
    }
    
    uint8_t getCourseDataLut2(int i) {
        static int offset = 0x93bf - tta_dseg_start_offset;
        return GetByte(a_dat, offset + i);
    }
    
    TileInfo getTileInfo(int l) {
        static int offset = 0x944f - tta_dseg_start_offset;
  
        return {
            .tile = a_dat[offset + l * 2    ],
            .info = a_dat[offset + l * 2 + 1],
        };
    }
    
    TileInfo getTileInfo(int tileX, int tileY) {
        return getTileInfo(tileY * XTileCount + tileX);
    }
    
    Color mapColor(uint8_t colorHi, uint8_t colorLo, const GamePalette &palette) const {
        static int doubleColorTable = 0xb297 - tta_dseg_start_offset;
        static int singleColorTable = 0xb497 - tta_dseg_start_offset;
        
        if (colorHi & 0x10) {
            colorHi = std::to_integer<uint8_t>(a_dat[singleColorTable + (colorHi & 0x0f)]);
            if (colorLo & 0x10) {
                colorLo = std::to_integer<uint8_t>(a_dat[singleColorTable + (colorLo & 0x0f)]);
            }
        }
        else if (colorLo & 0x10) {
            colorLo = std::to_integer<uint8_t>(a_dat[singleColorTable + (colorLo & 0x0f)]);
        }
        else {
            auto idx = (colorHi << 4) | colorLo;
            
            colorLo = std::to_integer<uint8_t>(a_dat[doubleColorTable + (idx * 2) + 0]);
            colorHi = std::to_integer<uint8_t>(a_dat[doubleColorTable + (idx * 2) + 1]);
        }
        
        auto pattern0rgb = palette.palette[colorLo];
        auto pattern1rgb = palette.palette[colorHi];
        
        Color color;
        color.r = (pattern0rgb.r + pattern1rgb.r) / 2;
        color.g = (pattern0rgb.g + pattern1rgb.g) / 2;
        color.b = (pattern0rgb.b + pattern1rgb.b) / 2;
        color.a = 0xff;
        return color;
    }

    void loadObjectData(const std::vector<std::byte> &a_dat)
    {
        const auto objectIdOffset    = 0xa257 - tta_dseg_start_offset;
        const auto xOffset           = 0xa397 - tta_dseg_start_offset;
        const auto yOffset           = 0xa4d7 - tta_dseg_start_offset;
        const auto zOffset           = 0xa617 - tta_dseg_start_offset;
        const auto orientationOffset = 0xa757 - tta_dseg_start_offset;

        /*
        auto normalObjectsCount = GetWord(a_dat, 0xa251 - 0x9370);
        auto activeObjectsCount = GetWord(a_dat, 0xa253 - 0x9370);
        auto lodObjectsCount    = GetWord(a_dat, 0xa255 - 0x9370);
        */

        for (int i = 0; i < 0xa0; i++)
        {
            m_objects.emplace_back(
                GetWord(a_dat, objectIdOffset + i * 2),
                GetWord(a_dat, xOffset + i * 2),
                GetWord(a_dat, yOffset + i * 2),
                GetWord(a_dat, zOffset + i * 2),
                GetWord(a_dat, orientationOffset + i * 2)
            );
        }
    }
    
public:
    SceneLst lst;
    
    std::vector<std::byte> a_dat;
    std::vector<std::byte> one_dat;
    std::vector<std::byte> t_bin;
    std::vector<std::byte> o_bin;
    std::vector<std::byte> p_bin;
    
    std::vector<Model> tiles;
    
    std::vector<GameObject> m_objects;
};

class Resources {
public:
    Resources(std::string basePath);
    ~Resources();
    
    std::vector<std::byte> file(const std::string &name);
    std::vector<std::byte> fileForCar(const std::string &name, const std::string &lowerCarName, const CarLst &carLst);
    std::vector<std::byte> fileForScene(const std::string &name, const std::string &lowerCarName, const SceneLst &sceneLst);

    const std::vector<Car>& cars() { return carsArray; }
    
private:
    std::string basePath;

    FILE *exeFile;
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
    exeFile = fopen(exeFilePath.c_str(), "r");
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
    
    auto playdiskPath = basePath + "/" + "playdisk.dat";
    auto playdiskFile = fopen(playdiskPath.c_str(), "r");
    playdisk.load(playdiskFile);
    fclose(playdiskFile);
    
    for (auto carName : playdisk.carNames()) {
        auto path = basePath + "/" + carName + ".lst";
        auto file = fopen(path.c_str(), "r");
        
        Car car;
        car.lst.load(file);
        fclose(file);
        
        car.col = this->fileForCar("COL.BIN", carName, car.lst);
        
        car.sicbin = this->fileForCar("SIC.BIN", carName, car.lst);
        car.sic = this->fileForCar(".SIC", carName, car.lst);
        
        car.top = this->fileForCar(".TOP", carName, car.lst);
        car.bot1 = this->fileForCar("1.BOT", carName, car.lst);
        car.bot2 = this->fileForCar("2.BOT", carName, car.lst);
        car.lbot = this->fileForCar("L.BOT", carName, car.lst);
        car.rbot = this->fileForCar("R.BOT", carName, car.lst);
        car.etc = this->fileForCar(".ETC", carName, car.lst);
        
        car.sc = this->fileForCar("SC.BIN", carName, car.lst);
        car.fl1 = this->fileForCar("FL1.LZ", carName, car.lst);
        car.fl2 = this->fileForCar("FL2.LZ", carName, car.lst);
        car.bic = this->fileForCar(".BIC", carName, car.lst);
        car.sid = this->fileForCar(".SID", carName, car.lst);
        car.icn = this->fileForCar(".ICN", carName, car.lst);

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

        Scene scene;
        scene.lst.load(file);
        fclose(file);
        
        // a -> subcourse + 'A'
        scene.a_dat = fileForScene("A.DAT", sceneName, scene.lst);
        scene.one_dat = fileForScene("1.DAT", sceneName, scene.lst);
        
        scene.t_bin = fileForScene("T.BIN", sceneName, scene.lst);
        
        //not used
        scene.o_bin = fileForScene("O.BIN", sceneName, scene.lst);
        scene.p_bin = fileForScene("P.BIN", sceneName, scene.lst);
        
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

Resources::~Resources() {
    fclose(exeFile);
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

std::vector<std::byte> Resources::file(const std::string &name) {
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

std::vector<std::byte> Resources::fileForCar(const std::string &name, const std::string &lowerCarName, const CarLst &carLst) {
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

std::vector<std::byte> Resources::fileForScene(const std::string &name, const std::string &lowerSceneName, const SceneLst &sceneLst) {
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

static std::vector<Color> defaultPalette = {
    { 0x00, 0x00, 0x00, 0xff },
    { 0x00, 0x00, 0x28, 0xff },
    { 0x00, 0x28, 0x00, 0xff },
    { 0x00, 0x28, 0x28, 0xff },
    { 0x28, 0x00, 0x00, 0xff },
    { 0x28, 0x00, 0x28, 0xff },
    { 0x28, 0x14, 0x00, 0xff },
    { 0x28, 0x28, 0x28, 0xff },
    { 0x14, 0x14, 0x14, 0xff },
    { 0x14, 0x14, 0x3c, 0xff },
    { 0x14, 0x3c, 0x14, 0xff },
    { 0x14, 0x3c, 0x3c, 0xff },
    { 0x3c, 0x14, 0x14, 0xff },
    { 0x3c, 0x14, 0x3c, 0xff },
    { 0x3c, 0x3c, 0x14, 0xff },
    { 0x3c, 0x3c, 0x3c, 0xff },
};

std::vector<Color> DefaultPalette() {
    static bool doubled = false;

    static std::vector<Color> defPalette = {
        { 0x00, 0x00, 0x00, 0xff },
        { 0x00, 0x00, 0x28, 0xff },
        { 0x00, 0x28, 0x00, 0xff },
        { 0x00, 0x28, 0x28, 0xff },
        { 0x28, 0x00, 0x00, 0xff },
        { 0x28, 0x00, 0x28, 0xff },
        { 0x28, 0x14, 0x00, 0xff },
        { 0x28, 0x28, 0x28, 0xff },
        { 0x14, 0x14, 0x14, 0xff },
        { 0x14, 0x14, 0x3c, 0xff },
        { 0x14, 0x3c, 0x14, 0xff },
        { 0x14, 0x3c, 0x3c, 0xff },
        { 0x3c, 0x14, 0x14, 0xff },
        { 0x3c, 0x14, 0x3c, 0xff },
        { 0x3c, 0x3c, 0x14, 0xff },
        { 0x3c, 0x3c, 0x3c, 0xff },
    };

    if (!doubled) {
        for (int i = 0; i < defPalette.size(); i++) {
            defPalette[i].r <<= 2;
            defPalette[i].g <<= 2;
            defPalette[i].b <<= 2;
        }
        doubled = true;
    }
    
    return defPalette;
}

GamePalette::GamePalette()
    : palette(0x100)
{
    copy(DefaultPalette(), 0);
}

void GamePalette::copy(std::vector<Color> src, int at) {
    for (int i = 0; i < src.size(); i++) {
        palette[at + i] = src[i];
    }
}

std::vector<Color> PaletteFromData(std::vector<std::byte> data) {
    auto count = data.size() / 3;
    std::vector<Color> palette(count);
    
    for (int i = 0; i < count; i++) {
        palette[i].r = std::to_integer<uint8_t>(data[(i * 3) + 0]) << 2;
        palette[i].g = std::to_integer<uint8_t>(data[(i * 3) + 1]) << 2;
        palette[i].b = std::to_integer<uint8_t>(data[(i * 3) + 2]) << 2;
        palette[i].a = 255;
    }

    return palette;
}

}
