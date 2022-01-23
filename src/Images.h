//
//  Images.h
//  testdrive
//
//  Created by Antonio Malara on 21/01/2022.
//

#pragma once

#include "GameImage.h"
#include "Resources.h"

namespace TD {

struct MenuImages {
    MenuImages(Resources &res)
        : palette(res.file("SELCOLR.BIN"), 0x10)
        , select (res.file("SELECT.LZ"),  320, palette)
        , compass(res.file("COMPASS.LZ"), 152, palette)
        , detail1(res.file("DETAIL1.LZ"), 184, palette)
        , detail2(res.file("DETAIL2.LZ"), 184, palette)
    { }

    const GamePalette palette;
    
    GameImage select;
    GameImage compass;
    GameImage detail1;
    GameImage detail2;
};

struct CarImages {
    CarImages(const Car &car)
        : carsicPalette(car.sicbin, 0x40)
        , carPalette(car.col, 0x10)
        , scPalette(car.sc, 0x10)
        , top (car.top,  320, carPalette)
        , bot1(car.bot1, 320, carPalette)
        , bot2(car.bot2, 320, carPalette)
        , lbot(car.lbot, 168, carPalette)
        , rbot(car.rbot, 168, carPalette)
        , etc (car.etc,   56, carPalette)
        , sic (car.sic, 0x48, carsicPalette)
        , fl1 (car.fl1,  208, scPalette)
        , fl2 (car.fl2,  208, scPalette)
        , bic (car.bic,  112, scPalette)
        , sid (car.sid,  112, scPalette)
        , icn (car.icn,  208, scPalette)
    { }

    const GamePalette carsicPalette;
    const GamePalette carPalette;
    const GamePalette scPalette;

    GameImage top;
    GameImage bot1;
    GameImage bot2;
    GameImage lbot;
    GameImage rbot;
    GameImage etc;
    GameImage sic;
    GameImage fl1;
    GameImage fl2;
    GameImage bic;
    GameImage sid;
    GameImage icn;
};

};
