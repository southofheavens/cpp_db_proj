#ifndef SHOP_PAGES_H
#define SHOP_PAGES_H

#include "GUI.h"
#include "ShopTools.h"

void InitShopPages(zmq::socket_t&, TWindow*);

void CategoriesPage(zmq::socket_t&, TWindow*);
void CertainCatPage(zmq::socket_t&, TWindow*, uint);
void GuitarPage(zmq::socket_t&, TWindow*, uint, uint); 
void SearchPage(zmq::socket_t&, TWindow*, IWidget*);

void AddGuitar(zmq::socket_t&, TWindow*, uint);
void AddGuitarCb(Fl_Widget*, void*);
void DeleteGuitar(zmq::socket_t&, TWindow*, uint);

void FreeAll();

#endif