#ifndef USER_INTERACTION_H
#define USER_INTERACTION_H

#include <string>
#include "ShopTools.h"
#include "UserTools.h"

std::string HashPassword(const std::string&);
bool CheckPassword(const std::string&);
bool CheckName(const std::string&);
bool CheckPhonenumber(const std::string&);
std::u32string Utf8ToUtf32(const std::string&);
bool RegDataMatch(zmq::socket_t&);
bool RegDataVerif(zmq::socket_t&);

void InitUserInteraction(zmq::socket_t&, TWindow*);

void SignUpPage(zmq::socket_t&, TWindow*);
void Registration(TRegArgs*);

void SignInPage(zmq::socket_t&, TWindow*);
void Login(TRegArgs*);

void ProfilePage(zmq::socket_t&, TWindow*);
void CartPage(zmq::socket_t&, TWindow*);
void PlaceAnOrder(std::pair<zmq::socket_t*,IWidget*>*);
void AdminPersAccPage(zmq::socket_t&, TWindow*);

void FreeAll2();
void FreeResources();

#endif