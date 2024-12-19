#ifndef USER_TOOLS_H
#define USER_TOOLS_H

#include <string>
#include "GUI.h"

enum class TType {admin, user};

// Структура для хранения информации о пользователе
struct TUserInfo
{
    TType type;
    std::string login;
    std::string password;
    std::string name;
    std::string phoneNumber;
    std::string email;
    uint userId;

    TUserInfo(const std::string& l, const std::string& pa, const std::string& n,
        const std::string& ph, const std::string& e) : login{l}, password{pa},
        name{n}, phoneNumber{ph}, email{e} {}

    TUserInfo(const std::string& l, const std::string& pa, const std::string& n,
        const std::string& ph, const std::string& e, const uint& u) : login{l}, password{pa},
        name{n}, phoneNumber{ph}, email{e}, userId{u}
    {
        if (login == "admin") {
            type = TType::admin;
        }
        else {
            type = TType::user;
        }
    }
};

extern TUserInfo* userInfo;

extern TGroup* userGroup;

extern TButton* signUp;
extern TButton* signIn;
extern TButton* profile;
extern TButton* cart;
extern TButton* adminPersAcc; // доступна админу

extern TScroll* userScroll;

extern TButton* back;

extern TArgs* uargs; // Для передачи аргументов в кнопки

// Структура для передачи аргументов при регистрации/входе
struct TRegArgs
{   
    TInBox* login;
    TInBox* password;
    TInBox* name;
    TInBox* phonenumber;
    TInBox* mail;
    TArgs* socketAndWin;

    TRegArgs(TInBox* l, TInBox* p) : login{l}, password{p}, socketAndWin{uargs} {}
    TRegArgs(TInBox* l, TInBox* pa, TInBox* n, TInBox* ph, TInBox* m) 
        : login{l}, password{pa}, name{n}, phonenumber{ph}, mail{m}, socketAndWin{uargs} {}
};

extern TRegArgs* rargs;

// Для передачи в качестве аргумента callback-функции для кнопки "Создать заказ"
extern std::pair<zmq::socket_t*,IWidget*>* ordArgs;

#endif