#ifndef SHOP_TOOLS_H
#define SHOP_TOOLS_H

#include "GUI.h"
#include "ZmqTools.h"

const std::string SENTINEL = "~@~";

constexpr uint PHOTO_W = 700;
constexpr uint PHOTO_H = 260;

constexpr uint PREV_W = 180;
constexpr uint PREV_H = 130;

constexpr uint CAT_PHOTO_W = 460;
constexpr uint CAT_PHOTO_H = 170;

constexpr uint WZ_X = WIN_W/16;
constexpr uint WZ_Y = WIN_H/10;
constexpr uint WZ_W = WIN_W-WIN_W/8;
constexpr uint WZ_H = WIN_H-WIN_H/5;

// Расстояния между кнопками (по горизонтали и по вертикали)
constexpr uint X_DIST = (13*WIN_W/16 - 3*WIN_W/16 - PREV_W*3) / 2;
constexpr uint Y_DIST = (17*WIN_H/20 - 3*WIN_H/20 - PREV_H*3) / 2;

// Структура для передачи аргументов в функцию обратного вызова
struct TArgs
{
    zmq::socket_t* s;
    TWindow* w;
    IWidget* widget;
    uint cid;   // category id (or guitar id)
    uint aid;   // attribute id

    std::vector<TInBox*>* pib;   // указатель на вектор указателей на TInBox*

    TArgs(zmq::socket_t* ss, TWindow* ww) : s{ss}, w{ww} {}
    TArgs(zmq::socket_t* ss, TWindow* ww, IWidget* iw) : s{ss}, w{ww}, widget{iw} {}
    TArgs(zmq::socket_t* ss, TWindow* ww, uint ccid) : s{ss}, w{ww}, cid{ccid} {}
    TArgs(zmq::socket_t* ss, TWindow* ww, uint ccid, uint aaid) : s{ss}, w{ww}, cid{ccid}, aid{aaid} {}
    TArgs(zmq::socket_t* ss, TWindow* ww, uint ccid, std::vector<TInBox*>* pp) : s{ss}, w{ww}, cid{ccid}, pib{pp} {}
};

// Для передачи в качестве аргумента обратного вызова для кнопок, связанных с 
// добавлением гитар в корзину
struct TAddToCartArgs     
{
    zmq::socket_t* s;
    TOutBox* pob;
    uint guitarId;
    uint userId;
    uint guitarCount;

    TAddToCartArgs(zmq::socket_t* ss, TOutBox* pp, const uint& gg, const uint& uu, const uint& gc)
        : s{ss}, pob{pp}, guitarId{gg}, userId{uu}, guitarCount{gc} {}
};

extern TGroup* workzoneGroup;

extern TScroll* scroll;

extern TAddToCartArgs* atca;

#endif