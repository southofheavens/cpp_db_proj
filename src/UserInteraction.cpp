#include "../include/UserInteraction.h"
#include <sstream>
#include <iomanip>
#include <openssl/sha.h> 
#include <cstdint>
#include <unicode/ustream.h>
#include <unicode/unistr.h>
#include <FL/fl_message.H>

std::vector<IWidget*> usWidgets;
std::vector<TImage*> usImages;

TUserInfo* userInfo = nullptr;

TButton* signUp = nullptr;
TButton* signIn = nullptr;
TButton* profile = nullptr;
TButton* cart = nullptr;
TButton* adminPersAcc = nullptr;

TScroll* userScroll = nullptr;

TButton* back = nullptr;

TArgs* uargs = nullptr;
TRegArgs* rargs = nullptr;

TGroup* userGroup = nullptr;

std::pair<zmq::socket_t*,IWidget*>* ordArgs = nullptr;

std::string HashPassword(const std::string& password)
{
    unsigned char hashed[SHA256_DIGEST_LENGTH]; // Массив для результата хеширования
    SHA256(reinterpret_cast<const unsigned char*>(password.c_str()), password.length(), hashed);

    std::stringstream ss;
    for (unsigned char byte : hashed) {
        ss << std::hex << std::setfill('0') << std::setw(2) << static_cast<unsigned int>(byte);
    }
    return ss.str();
}

bool CheckPassword(const std::string& p)
{
    if (p.length() == 0)
    {
        usWidgets.back()->SetLabel("Пароль не может быть пустым!");
        return false;
    }
    bool capital = false;
    for (size_t i = 0; i < userInfo->password.length(); ++i)
    {
        if ('A' <= userInfo->password[i] && userInfo->password[i] <= 'Z') 
        {
            capital = true;
            break;
        }
    }
    if (!capital) 
    {
        usWidgets.back()->SetLabel("Пароль должен содержать хотя бы одну прописную букву!");
        return false;
    }
    return true;
}

bool CheckName(const std::string& n)
{
    if (n.length() == 0)
    {
        usWidgets.back()->SetLabel("Имя не может быть пустым");
        return false;
    }
    bool okay = true;
    std::vector<char32_t> russianAlphabet = 
    {
        U'а', U'б', U'в', U'г', U'д', U'е', U'ё', U'ж', U'з', U'и', U'й', U'к',
        U'л', U'м', U'н', U'о', U'п', U'р', U'с', U'т', U'у', U'ф', U'х', U'ц',
        U'ч', U'ш', U'щ', U'ъ', U'ы', U'ь', U'э', U'ю', U'я',
        U'А', U'Б', U'В', U'Г', U'Д', U'Е', U'Ё', U'Ж', U'З', U'И', U'Й', U'К',
        U'Л', U'М', U'Н', U'О', U'П', U'Р', U'С', U'Т', U'У', U'Ф', U'Х', U'Ц',
        U'Ч', U'Ш', U'Щ', U'Ъ', U'Ы', U'Ь', U'Э', U'Ю', U'Я'
    };
    std::u32string name = Utf8ToUtf32(userInfo->name);
    for (size_t i = 0; i < name.length(); ++i)
    {
        char32_t let = name[i];

        if (!('a' <= let && let <= 'z'
            || 'A' <= let && let <= 'Z'
            || (std::find(russianAlphabet.begin(),russianAlphabet.end(),let) != russianAlphabet.end())
            || let == ' '))
        {
            okay = false;
            break;
        }
    }
    if (!okay)
    {
        usWidgets.back()->SetLabel("Имя может содержать только буквы русского/английского алфавита!");
        return false;
    }

    return true;
}

bool CheckPhonenumber(const std::string& n)
{
    if (n.length() != 12) {
        return false;
    }
    if (n[0] != '+' || n[1] != '7') {
        return false;
    }
    for (size_t i = 2; i < n.length(); ++i)
    {
        if (!('0' <= n[i] && n[i] <= '9')) {
            return false;
        }
    }

    return true;
}

std::u32string Utf8ToUtf32(const std::string& utf8Str) 
{
    icu::UnicodeString ucs(utf8Str.c_str(), utf8Str.length());
    std::u32string result;
    for (int32_t i = 0; i < ucs.length(); ++i) {
        result += ucs[i];
    }
    return result;
}

bool RegDataMatch(zmq::socket_t& s)
{
    std::string request = "CheckUserReg" + SENTINEL + userInfo->login + SENTINEL;
    request += userInfo->phoneNumber + SENTINEL + userInfo->email;
    std::string replyStr = ZmqRequest(s,request); 
    
    std::vector<uint> result(3,0); // 0 - логин, 1 - номер телефона, 2 - почта

    if (replyStr.find("Invalid") != std::string::npos)
    {
        std::cerr << replyStr << std::endl;
        exit(1);
    }

    size_t pos = 0;
    size_t action = 0;
    while ((pos = replyStr.find(SENTINEL)) != std::string::npos) 
    {
        std::string token = replyStr.substr(0, pos);
        if (token != "0") {
            result[action] = 1;
        }
        action++;
        replyStr.erase(0, pos + SENTINEL.length());
    }   

    if (result[0] != 0 || result[1] != 0 || result[2] != 0)
    {
        delete userInfo;
        userInfo = nullptr;
        if (result[0] != 0) {
            usWidgets.back()->SetLabel("Логин занят!");
        }
        else if (result[1] != 0) {
            usWidgets.back()->SetLabel("Номер телефона занят!");
        }
        else {
            usWidgets.back()->SetLabel("Электронная почта занята!");
        }
        return false;
    }
    return true;
}

bool RegDataVerif(zmq::socket_t& s)
{
    // Проверка, что введённых логина, номера телефоны и почты нет в БД
    bool rdm = RegDataMatch(s);
    if (!rdm) {
        return false;
    }

    // Логин
    if (userInfo->login.length() == 0) 
    {
        usWidgets.back()->SetLabel("Логин не может быть пустым!");
        return false;
    }

    // Пароль
    bool chPass = CheckPassword(userInfo->password); 
    if (!chPass) {
        return false;
    }

    // Имя
    bool chName = CheckName(userInfo->name);
    if (!chName) {
        return false;
    }
    
    // Номер телефона
    bool cp = CheckPhonenumber(userInfo->phoneNumber);
    if (!cp)
    {
        usWidgets.back()->SetLabel("Введите номер телефона в формате '+7xxxxxxxxxx'!");
        return false;
    }

    // Почта
    if (userInfo->email.length() == 0 || userInfo->email.find('@') == std::string::npos) 
    {
        usWidgets.back()->SetLabel("Введите корректный адрес электронной почты!");
        return false;
    }
   
    return true;
}

void InitUserInteraction(zmq::socket_t& s, TWindow* win)
{  
    userGroup = new TGroup({WIN_W/16,WIN_H/10},WIN_W-WIN_W/8,WIN_H-WIN_H/5);

    back = new TButton({WIN_W/12,WIN_H/8},WIN_W/20,WIN_H/20,"@<-",[](Fl_Widget*,void*)
    {
        [](){ workzoneGroup->Show(); userGroup->Hide(); }();
    });
    userGroup->Attach(*back);

    win->Attach(*userGroup);
    userGroup->Hide();

    uargs = new TArgs(&s,win);

    signUp = new TButton({WIN_W-2*WIN_W/10,0},WIN_W/10,WIN_H/15,"Регистрация");
    signUp->SetCallback([](Fl_Widget*,void* a)
    {
        TArgs* args = reinterpret_cast<TArgs*>(a);
        SignUpPage(*args->s, args->w);
    }, reinterpret_cast<void*>(uargs));

    signIn = new TButton({WIN_W-WIN_W/10,0},WIN_W/10,WIN_H/15,"Вход");
    signIn->SetCallback([](Fl_Widget*,void* a)
    {
        TArgs* args = reinterpret_cast<TArgs*>(a);
        SignInPage(*args->s, args->w);
    }, reinterpret_cast<void*>(uargs));

    profile = new TButton({WIN_W-WIN_W/10,0},WIN_W/10,WIN_H/15,"Профиль");
    profile->SetCallback([](Fl_Widget*,void* a)
    {
        TArgs* args = reinterpret_cast<TArgs*>(a);
        ProfilePage(*args->s, args->w);
    }, reinterpret_cast<void*>(uargs));

    cart = new TButton({WIN_W-2*WIN_W/10,0},WIN_W/10,WIN_H/15,"Корзина");
    cart->SetCallback([](Fl_Widget*,void* a)
    {
        TArgs* args = reinterpret_cast<TArgs*>(a);
        CartPage(*args->s, args->w);
    }, reinterpret_cast<void*>(uargs));

    adminPersAcc = new TButton({WIN_W-2*WIN_W/10,0},WIN_W/10,WIN_H/15,"ЛК Админа");
    adminPersAcc->SetCallback([](Fl_Widget*,void* a)
    {
        TArgs* args = reinterpret_cast<TArgs*>(a);
        AdminPersAccPage(*args->s, args->w);
    }, reinterpret_cast<void*>(uargs));

    win->Attach(*signUp);
    win->Attach(*signIn);
    win->Attach(*profile);
    win->Attach(*cart);
    win->Attach(*adminPersAcc);
    
    profile->Hide();
    cart->Hide();
    adminPersAcc->Hide();
}

void SignUpPage(zmq::socket_t& s, TWindow* win) 
{
    FreeAll2();

    win->Redraw();
    userGroup->Show();
    workzoneGroup->Hide();

    // Размещаем поле для ввода логина
    usWidgets.push_back(new TInBox({WZ_X+(WZ_W-WZ_W/5)/2,WZ_Y+4*WZ_H/20},WZ_W/5,WZ_H/20,"Логин: "));
    userGroup->Attach(*usWidgets.back());

    // Размещаем поле для ввода пароля
    usWidgets.push_back(new TInBox({WZ_X+(WZ_W-WZ_W/5)/2,WZ_Y+6*WZ_H/20},WZ_W/5,WZ_H/20,"Пароль: "));
    userGroup->Attach(*usWidgets.back());

    // Размещаем поле для ввода имени
    usWidgets.push_back(new TInBox({WZ_X+(WZ_W-WZ_W/5)/2,WZ_Y+10*WZ_H/20},WZ_W/5,WZ_H/20,"Имя: "));
    userGroup->Attach(*usWidgets.back());

    // Размещаем поле для ввода номера телефона
    usWidgets.push_back(new TInBox({WZ_X+(WZ_W-WZ_W/5)/2,WZ_Y+12*WZ_H/20},WZ_W/5,WZ_H/20,"Номер телефона: "));
    userGroup->Attach(*usWidgets.back());

    // Размещаем поле для ввода адреса электронной почты
    usWidgets.push_back(new TInBox({WZ_X+(WZ_W-WZ_W/5)/2,WZ_Y+14*WZ_H/20},WZ_W/5,WZ_H/20,"Адрес электронной почты: "));
    userGroup->Attach(*usWidgets.back());

    // Размещаем надпись "Регистрация"
    usWidgets.push_back(new TBox({WZ_X+(WZ_W-WZ_W/5)/2,WZ_Y+WZ_H/40},WZ_W/5,WZ_H/20,"Регистрация"));
    userGroup->Attach(*usWidgets.back());
    //reinterpret_cast<TBox*>(usWidgets.back())->SetColor(fl_rgb_color(123,104,238));
    reinterpret_cast<TBox*>(usWidgets.back())->SetSize(30);
    reinterpret_cast<TBox*>(usWidgets.back())->SetFont(FL_BOLD);

    // Размещаем надпись "Данные для авторизации"
    usWidgets.push_back(new TBox({WZ_X+(WZ_W-WZ_W/5)/2,WZ_Y+2*WZ_H/20},WZ_W/5,WZ_H/20,"Данные для авторизации"));
    userGroup->Attach(*usWidgets.back());
    reinterpret_cast<TBox*>(usWidgets.back())->SetColor(fl_rgb_color(178,34,34));
    reinterpret_cast<TBox*>(usWidgets.back())->SetSize(20);
    reinterpret_cast<TBox*>(usWidgets.back())->SetFont(FL_BOLD_ITALIC);

    // Размещаем надпись "Персональная информация"
    usWidgets.push_back(new TBox({WZ_X+(WZ_W-WZ_W/5)/2,WZ_Y+8*WZ_H/20},WZ_W/5,WZ_H/20,"Персональная информация"));
    userGroup->Attach(*usWidgets.back());
    reinterpret_cast<TBox*>(usWidgets.back())->SetColor(fl_rgb_color(178,34,34));
    reinterpret_cast<TBox*>(usWidgets.back())->SetSize(20);
    reinterpret_cast<TBox*>(usWidgets.back())->SetFont(FL_BOLD_ITALIC);

    // Размещаем кнопку "Зарегистрироваться"
    usWidgets.push_back(new TButton({WZ_X+(WZ_W-WZ_W/5)/2,WZ_Y+16*WZ_H/20},WZ_W/5,WZ_H/15,"Зарегистрироваться"));
    userGroup->Attach(*usWidgets.back());

    rargs = new TRegArgs(reinterpret_cast<TInBox*>(usWidgets[0]),
        reinterpret_cast<TInBox*>(usWidgets[1]), reinterpret_cast<TInBox*>(usWidgets[2]), 
        reinterpret_cast<TInBox*>(usWidgets[3]), reinterpret_cast<TInBox*>(usWidgets[4]));

    reinterpret_cast<TButton*>(usWidgets.back())->SetCallback([](Fl_Widget*,void* a)
    {
        Registration(reinterpret_cast<TRegArgs*>(a));
    }, reinterpret_cast<void*>(rargs));

    // Размещаем текст с возможным предупреждением
    usWidgets.push_back(new TBox({WZ_X,WZ_Y+9*WZ_H/10},WZ_W,WZ_H/15));
    userGroup->Attach(*usWidgets.back());
    reinterpret_cast<TBox*>(usWidgets.back())->SetColor(fl_rgb_color(178,34,34));
    reinterpret_cast<TBox*>(usWidgets.back())->SetSize(25);
}

void Registration(TRegArgs* ra)
{
    userInfo = new TUserInfo(ra->login->GetV(), ra->password->GetV(), ra->name->GetV(),
        ra->phonenumber->GetV(), ra->mail->GetV()); 
    
    // проверка на непустые значения
    bool okay = RegDataVerif(*ra->socketAndWin->s);
    if (!okay) 
    {
        delete userInfo;
        userInfo = nullptr;
        return;
    }

    std::string hashPass = HashPassword(userInfo->password);
    std::string request = "AddUser" + SENTINEL + userInfo->login + SENTINEL;
    request += hashPass + SENTINEL + userInfo->name + SENTINEL;
    request += userInfo->phoneNumber + SENTINEL + userInfo->email;
    std::string replyStr = ZmqRequest(*ra->socketAndWin->s,request); 

    if (replyStr.find("Invalid") != std::string::npos)
    {
        std::cerr << replyStr << std::endl;
        exit(1);
    }

    delete userInfo;
    userInfo = nullptr;
    
    for (size_t i = 0; i < usWidgets.size(); ++i) {
        usWidgets[i]->Hide();
    }
    usWidgets.push_back(new TBox({WIN_W/2-WZ_W/10,WIN_H/2-WZ_H/15},WZ_W/5,WZ_H/15, "Вы зарегистрированы!"));
    userGroup->Attach(*usWidgets.back());
    reinterpret_cast<TBox*>(usWidgets.back())->SetColor(fl_rgb_color(50,205,50));
    reinterpret_cast<TBox*>(usWidgets.back())->SetFont(FL_TIMES_BOLD_ITALIC);
    reinterpret_cast<TBox*>(usWidgets.back())->SetSize(40);

    // проверяем что с логином и почтой всё нормально
    // если да, то ништяк
    // если нет, то чистим память из под userInfo и выдаём на экране ошибку, что такой пользователь зарегистрирован
    // тут нужно очистить память из под ra и из под user, если с ним что-то не так

    // и мне кажется что userInfo надо очистить, ведь после регистрации вход у нас в аккаунт не выполняется
    // а если userInfo != nullptr, то мы видим кнопки профиль и корзина, а не вход и регистрация
}

void SignInPage(zmq::socket_t& s, TWindow* win)
{
    FreeAll2();
    win->Redraw();
    userGroup->Show();
    workzoneGroup->Hide();

    // Размещаем поле для ввода логина
    usWidgets.push_back(new TInBox({WZ_X+(WZ_W-WZ_W/5)/2,WZ_Y+3*WZ_H/20},WZ_W/5,WZ_H/20,"Логин: "));
    userGroup->Attach(*usWidgets.back());

    // Размещаем поле для ввода пароля
    usWidgets.push_back(new TInBox({WZ_X+(WZ_W-WZ_W/5)/2,WZ_Y+5*WZ_H/20},WZ_W/5,WZ_H/20,"Пароль: "));
    userGroup->Attach(*usWidgets.back());

    // Размещаем надпись "Вход"
    usWidgets.push_back(new TBox({WZ_X+(WZ_W-WZ_W/5)/2,WZ_Y+WZ_H/40},WZ_W/5,WZ_H/20,"Вход"));
    userGroup->Attach(*usWidgets.back());
    //reinterpret_cast<TBox*>(usWidgets.back())->SetColor(fl_rgb_color(123,104,238));
    reinterpret_cast<TBox*>(usWidgets.back())->SetSize(30);
    reinterpret_cast<TBox*>(usWidgets.back())->SetFont(FL_BOLD);

    // Размещаем кнопку "Войти"
    usWidgets.push_back(new TButton({WZ_X+(WZ_W-WZ_W/5)/2,WZ_Y+7*WZ_H/20},WZ_W/5,WZ_H/15,"Войти"));
    userGroup->Attach(*usWidgets.back());

    rargs = new TRegArgs(reinterpret_cast<TInBox*>(usWidgets[0]),   // ? корректно ли освобождается память
        reinterpret_cast<TInBox*>(usWidgets[1]));

    reinterpret_cast<TButton*>(usWidgets.back())->SetCallback([](Fl_Widget*,void* a)
    {
        Login(reinterpret_cast<TRegArgs*>(a));
    }, reinterpret_cast<void*>(rargs));

    // Размещаем текст с возможным предупреждением
    usWidgets.push_back(new TBox({7*WIN_W/10,5*WIN_H/8},WZ_W/5,WZ_H/3));
    userGroup->Attach(*usWidgets.back());
    reinterpret_cast<TBox*>(usWidgets.back())->SetColor(fl_rgb_color(178,34,34));
}

void Login(TRegArgs* ra)
{
    std::string login = ra->login->GetV();
    std::string pass = ra->password->GetV();

    // проверка на непустые значения

    std::string hashPass = HashPassword(pass);
    const std::string sentinel = "~@~";
    std::string request = "CheckUserLog" + sentinel + login + sentinel + hashPass;
    std::string replyStr = ZmqRequest(*ra->socketAndWin->s,request); 
    
    bool find;    // false - пользователь не найден, true - пользователь найден

    if (replyStr.find("Invalid") == std::string::npos)
    {
        // std::string delimiter = "@";
        size_t pos = replyStr.find(SENTINEL);
        std::string token = replyStr.substr(0, pos);

        if (token == "0") {
            find = false;
        }
        else {
            find = true;
        }
    }
    else
    {
        std::cerr << replyStr << std::endl;
        exit(1);
    }  

    if (!find) {
        usWidgets.back()->SetLabel("Пользователь не найден");
    }
    else
    {
        for (size_t i = 0; i < usWidgets.size(); ++i) {
            usWidgets[i]->Hide();
        }
        usWidgets.push_back(new TBox({WIN_W/2-WZ_W/10,WIN_H/2-WZ_H/15},WZ_W/5,WZ_H/15, "Вход выполнен!"));
        userGroup->Attach(*usWidgets.back());
        reinterpret_cast<TBox*>(usWidgets.back())->SetColor(fl_rgb_color(178,34,34));
        reinterpret_cast<TBox*>(usWidgets.back())->SetFont(FL_TIMES_BOLD_ITALIC);
        reinterpret_cast<TBox*>(usWidgets.back())->SetSize(40);

        // запрос для того чтобы найти данные пользователя и выделить память
        // для userInfo
        std::string request = "GetUserInfo" + sentinel + login + sentinel + hashPass;
        std::string replyStr = ZmqRequest(*ra->socketAndWin->s,request); 

        if (replyStr.find("Invalid") == std::string::npos)
        {
            size_t pos = 0;

            std::vector<std::string> result;
            while ((pos = replyStr.find(SENTINEL)) != std::string::npos) 
            {
                std::string token = replyStr.substr(0, pos);
                result.push_back(token);
                replyStr.erase(0, pos + SENTINEL.length());
            }

            userInfo = new TUserInfo(result[0],result[1],result[2],result[3],result[4],std::stoi(result[5]));
        }
        else
        {
            std::cerr << replyStr << std::endl;
            exit(1);
        }

        signUp->Hide();
        signIn->Hide();
        profile->Show();
        if (userInfo->type != TType::admin) {
            cart->Show();
        }
        if (userInfo->type == TType::admin) {
            adminPersAcc->Show();
        }

        // Отправляем запрос на сервер с информацией о том, что вход выполнен
        request = "Login"+SENTINEL+(userInfo->type == TType::admin ? "admin" : "user");
        replyStr = ZmqRequest(*ra->socketAndWin->s,request);
    }
}

void ProfilePage(zmq::socket_t& s, TWindow* win)
{
    FreeAll2();
    win->Redraw();
    userGroup->Show();
    workzoneGroup->Hide();

    // Размещаем надпись "Профиль"
    usWidgets.push_back(new TBox({WZ_X+(WZ_W-WZ_W/5)/2,WZ_Y+WZ_H/40},WZ_W/5,WZ_H/20,"Профиль"));
    userGroup->Attach(*usWidgets.back());
    //reinterpret_cast<TBox*>(usWidgets.back())->SetColor(fl_rgb_color(123,104,238));
    reinterpret_cast<TBox*>(usWidgets.back())->SetSize(30);
    reinterpret_cast<TBox*>(usWidgets.back())->SetFont(FL_BOLD);

    // Размещаем логин
    usWidgets.push_back(new TOutBox({WZ_X+(WZ_W-WZ_W/5)/2,WZ_Y+5*WZ_H/40},WZ_W/5,WZ_H/20,"Логин: "));
    userGroup->Attach(*usWidgets.back());
    reinterpret_cast<TOutBox*>(usWidgets.back())->Put(userInfo->login);

    // Размещаем имя
    usWidgets.push_back(new TOutBox({WZ_X+(WZ_W-WZ_W/5)/2,WZ_Y+9*WZ_H/40},WZ_W/5,WZ_H/20,"Имя: "));
    userGroup->Attach(*usWidgets.back());
    reinterpret_cast<TOutBox*>(usWidgets.back())->Put(userInfo->name);

    // Размещаем номер телефона
    usWidgets.push_back(new TOutBox({WZ_X+(WZ_W-WZ_W/5)/2,WZ_Y+13*WZ_H/40},WZ_W/5,WZ_H/20,"Номер телефона: "));
    userGroup->Attach(*usWidgets.back());
    reinterpret_cast<TOutBox*>(usWidgets.back())->Put(userInfo->phoneNumber);

    // Размещаем почту
    usWidgets.push_back(new TOutBox({WZ_X+(WZ_W-WZ_W/5)/2,WZ_Y+17*WZ_H/40},WZ_W/5,WZ_H/20,"Адрес электронной почты: "));
    userGroup->Attach(*usWidgets.back());
    reinterpret_cast<TOutBox*>(usWidgets.back())->Put(userInfo->email);

    // Размещаем кнопку "Выйти"
    usWidgets.push_back(new TButton({WZ_X+(WZ_W-WZ_W/5)/2,WZ_Y+21*WZ_H/40},WZ_W/5,WZ_H/20,"Выйти из профиля"));
    userGroup->Attach(*usWidgets.back());
    usWidgets.back()->SetCallback([](Fl_Widget*,void* a)
    {
        signUp->Show();
        signIn->Show();
        profile->Hide();
        cart->Hide();
        adminPersAcc->Hide();
        delete userInfo;
        userInfo = nullptr;

        // Отправляем запрос на сервер с информацией о том, что пользователь вышел из профиля
        std::string replyStr = ZmqRequest(*reinterpret_cast<TArgs*>(a)->s,"Logout"); 

        if (replyStr.find("Invalid") != std::string::npos)
        {
            std::cerr << replyStr << std::endl;
            exit(1);
        }
         
        SignInPage(*reinterpret_cast<TArgs*>(a)->s,reinterpret_cast<TArgs*>(a)->w);
    }, reinterpret_cast<void*>(uargs));
}

void CartPage(zmq::socket_t& s, TWindow* win)
{
    FreeAll2();
    win->Redraw();
    userGroup->Show();
    workzoneGroup->Hide();
    
    std::string replyStr = ZmqRequest(s,"GetCartItems"+SENTINEL+std::to_string(userInfo->userId)); 
    if (replyStr.find("Invalid") == std::string::npos)
    {
        std::vector<std::string> guitarIds;
        std::vector<std::string> quantities;

        size_t pos = 0;

        while ((pos = replyStr.find(SENTINEL)) != std::string::npos) 
        {
            std::string token = replyStr.substr(0, pos);
            guitarIds.push_back(token);
            replyStr.erase(0, pos + SENTINEL.length());

            pos = replyStr.find(SENTINEL);
            token = replyStr.substr(0, pos);
            quantities.push_back(token);
            replyStr.erase(0, pos + SENTINEL.length());
        }

        userScroll = new TScroll({WZ_X+2,WIN_H/8+WIN_H/10},WZ_W-4,WZ_H-WIN_H/8-WIN_H/10);
        userGroup->Attach(*userScroll);

        for (size_t i = 0; i < guitarIds.size(); ++i)
        {
            replyStr = ZmqRequest(s,"GetGuitarInfo"+SENTINEL+guitarIds[i]); 

            constexpr uint xPos = WIN_W/12+WIN_W/30;
            constexpr uint yPos = WIN_H/8+WIN_H/10;
            if (replyStr.find("Invalid") == std::string::npos)
            {
                std::string pathToPreview;
                uint guitarAttrId;
                std::string name;
                uint price;

                int action = 0;
                while ((pos = replyStr.find(SENTINEL)) != std::string::npos) 
                {
                    std::string token = replyStr.substr(0, pos);
                    switch (action)
                    {
                        case 0:
                            pathToPreview = token;
                            break;
                        case 1:
                            guitarAttrId = std::stoi(token);
                            break;
                        case 2:
                            name = token;
                            break;
                        case 3:
                            price = std::stoi(token);
                            break;
                    }
                    action = (action + 1) % 4;
                    replyStr.erase(0, pos + SENTINEL.length());
                }

                usImages.push_back(new TImage({xPos, yPos+3*PREV_H/2*i},pathToPreview));
                userScroll->Attach(*usImages.back());

                usWidgets.push_back(new TOutBox({xPos + 11*PREV_W/10,yPos+3*PREV_H/2*i},PREV_W,PREV_H/5));
                reinterpret_cast<TOutBox*>(usWidgets.back())->Put(name);
                userScroll->Attach(*usWidgets.back());

                usWidgets.push_back(new TBox({xPos + 11*PREV_W/10,yPos+PREV_H/20+3*PREV_H/2*i},
                    PREV_W,PREV_W,"Итого: " + std::to_string(std::stoi(quantities[i])*price) + " руб."));
                userScroll->Attach(*usWidgets.back());

                usWidgets.push_back(new TBox({xPos + 11*PREV_W/10,yPos+PREV_H/5+3*PREV_H/2*i},
                    PREV_W,PREV_W,"Количество: " + quantities[i]));
                userScroll->Attach(*usWidgets.back());
            }
            else
            {
                std::cerr << replyStr << std::endl;
                exit(1);
            }
        }
        
        usWidgets.push_back(new TInBox({WZ_X+WZ_W/3,WZ_Y+19*WZ_H/20}, 2*PREV_W, PREV_H/5,"Введите адрес доставки: "));
        userGroup->Attach(*usWidgets.back());

        ordArgs = new std::pair<zmq::socket_t*,IWidget*>(&s,usWidgets.back());
        usWidgets.push_back(new TButton({WZ_X+8*WZ_W/10,WZ_Y+9*WZ_H/10},WZ_W/5,WZ_H/10,"Оформить заказ",
        [](Fl_Widget*, void* a)
        {
            std::pair<zmq::socket_t*,IWidget*>* ordArgs = reinterpret_cast<std::pair<zmq::socket_t*,IWidget*>*>(a);
            PlaceAnOrder(ordArgs);
        },reinterpret_cast<void*>(ordArgs)));
        userGroup->Attach(*usWidgets.back());
    }
    else
    {
        std::cerr << replyStr << std::endl;
        exit(1);
    }  
}

void PlaceAnOrder(std::pair<zmq::socket_t*,IWidget*>* ordArgs)
{
    std::string replyStr = ZmqRequest(*ordArgs->first,"GetCartItems"+SENTINEL+std::to_string(userInfo->userId)); 
    if (replyStr.find("Invalid") != std::string::npos)
    {
        std::cerr << replyStr << '\n';
        exit(1);
    }

    std::vector<std::string> guitarIds;
    std::vector<std::string> quantities;    // количества гитар (в корзине)
    size_t pos = 0;

    while ((pos = replyStr.find(SENTINEL)) != std::string::npos) 
    {
        std::string token = replyStr.substr(0, pos);
        guitarIds.push_back(token);
        replyStr.erase(0, pos + SENTINEL.length());

        pos = replyStr.find(SENTINEL);
        token = replyStr.substr(0, pos);
        quantities.push_back(token);
        replyStr.erase(0, pos + SENTINEL.length());
    }

    std::vector<uint> inStocks(guitarIds.size());   // количества  гитар (всего)

    for (size_t i = 0; i < inStocks.size(); ++i)
    {
        std::string replyStr = ZmqRequest(*ordArgs->first,"GetGuitarInStock"+SENTINEL+guitarIds[i]);
        if (replyStr.find("Invalid") != std::string::npos)
        {
            std::cerr << replyStr << '\n';
            exit(1);
        }

        inStocks[i] = std::stoi(replyStr);
    }

    std::string request = "CreateOrder"+SENTINEL+std::to_string(userInfo->userId)+SENTINEL;
    request += reinterpret_cast<TInBox*>(ordArgs->second)->GetV();
    std::string orderId = ZmqRequest(*ordArgs->first,request);

    if (orderId.find("Invalid") != std::string::npos)
    {
        std::cerr << replyStr << '\n';
        exit(1);
    } 
    pos = orderId.find(SENTINEL);
    orderId = orderId.substr(0,pos);

    for (size_t i = 0; i < guitarIds.size(); ++i)
    {
        std::string request = "AddItemToOrder"+SENTINEL+orderId+SENTINEL+std::to_string(userInfo->userId);
        request += SENTINEL+guitarIds[i]+SENTINEL+quantities[i];

        std::string replyStr = ZmqRequest(*ordArgs->first,request); 
        if (replyStr.find("Invalid") != std::string::npos)
        {
            std::cerr << replyStr << '\n';
            exit(1);
        } 
    }

    // for (size_t i = 0; i < quantities.size(); ++i)
    // {
    //     // Обновляем количество гитар (in_stock) в базе данных
    //     std::string request = "UpdateGuitarInStock"+SENTINEL+guitarIds[i];
    //     request += SENTINEL+std::to_string(inStocks[i]-std::stoi(quantities[i]));
    //     std::string replyStr = ZmqRequest(*ordArgs->first,request); 
    //     if (replyStr.find("Invalid") != std::string::npos)
    //     {
    //         std::cerr << replyStr << '\n';
    //         exit(1);
    //     } 

    //     //  Удаляем гитару из корзины
    //     request = "DeleteCartItem"+SENTINEL+guitarIds[i]+SENTINEL;
    //     request += std::to_string(userInfo->userId);
    //     replyStr = ZmqRequest(*ordArgs->first,request); 
    //     if (replyStr.find("Invalid") != std::string::npos)
    //     {
    //         std::cerr << replyStr << '\n';
    //         exit(1);
    //     } 
    // }

    for (size_t i = 0; i < usWidgets.size(); ++i) {
        usWidgets[i]->Hide();
    }
    for (size_t i = 0; i < usImages.size(); ++i) {
        usImages[i]->Hide();
    }

    usWidgets.push_back(new TBox({WZ_X + WZ_W/3,WZ_Y+WZ_H/3},WZ_W/3,WZ_H/5,"Заказ успешно создан!"));
    reinterpret_cast<TBox*>(usWidgets.back())->SetColor(fl_rgb_color(50,205,50));
    reinterpret_cast<TBox*>(usWidgets.back())->SetFont(FL_TIMES_BOLD_ITALIC);
    reinterpret_cast<TBox*>(usWidgets.back())->SetSize(40);
    userGroup->Attach(*usWidgets.back());
}

void AdminPersAccPage(zmq::socket_t& s, TWindow* win)
{
    FreeAll2();

    win->Redraw();
    userGroup->Show();
    workzoneGroup->Hide();

    constexpr uint SCROLLBAR_WIDTH = 16;
    userScroll = new TScroll({WZ_X,WZ_Y+WZ_H/5},WZ_W-2*SCROLLBAR_WIDTH,WZ_H-WZ_H/5);
    userGroup->Attach(*userScroll);

    std::string replyStr = ZmqRequest(s,"GetMostPopularCategory");
    // Проверяем, содержит ли ответ ошибку
    if (replyStr.find("Invalid") != std::string::npos)
    {
        std::cerr << replyStr << std::endl;
        exit(1);
    }

    replyStr = replyStr.substr(0,replyStr.find(SENTINEL));
    usWidgets.push_back(new TBox{{WZ_X+WZ_W/6,WZ_Y+WZ_H/30},WZ_W/5,WZ_H/10,
        "Самая популярная категория: " + replyStr});
    reinterpret_cast<TBox*>(usWidgets.back())->SetSize(20);
    reinterpret_cast<TBox*>(usWidgets.back())->SetFont(FL_TIMES_ITALIC);
    userGroup->Attach(*usWidgets.back());

    replyStr = ZmqRequest(s,"GetAverageOrdersTotalAmount");
    if (replyStr.find("Invalid") != std::string::npos)
    {
        std::cerr << replyStr << std::endl;
        exit(1);
    }

    replyStr = std::to_string(std::stoi(replyStr.substr(0,replyStr.find(SENTINEL))));
    usWidgets.push_back(new TBox{{WZ_X+65*WZ_W/100,WZ_Y+WZ_H/30},WZ_W/5,WZ_H/10,
        "Средняя сумма заказов: " + replyStr});
    reinterpret_cast<TBox*>(usWidgets.back())->SetSize(20);
    reinterpret_cast<TBox*>(usWidgets.back())->SetFont(FL_TIMES_ITALIC);
    userGroup->Attach(*usWidgets.back());

    // Размещаем надпись "Все заказы"
    usWidgets.push_back(new TBox{{WZ_X+WZ_W/2-WZ_W/10,WZ_Y+WZ_H/10},WZ_W/5,WZ_H/10,
        "Все заказы"});
    reinterpret_cast<TBox*>(usWidgets.back())->SetSize(25);
    reinterpret_cast<TBox*>(usWidgets.back())->SetFont(FL_TIMES_ITALIC);
    userGroup->Attach(*usWidgets.back());

    replyStr = ZmqRequest(s,"GetOrders");
    if (replyStr.find("Invalid") != std::string::npos)
    {
        std::cerr << replyStr << std::endl;
        exit(1);
    }

    std::vector<std::string> orderIds;
    std::vector<std::string> userIds;
    std::vector<std::string> orderDates;
    std::vector<std::string> addresses;
    std::vector<std::string> statuses;

    size_t pos = 0;
    int action = 0;
    while ((pos = replyStr.find(SENTINEL)) != std::string::npos) 
    {
        std::string token = replyStr.substr(0, pos);
        switch (action)
        {
            case 0:
                orderIds.push_back(token);
                break;
            case 1:
                userIds.push_back(token);
                break;
            case 2:
                orderDates.push_back(token);
                break;
            case 3:
                addresses.push_back(token);
                break;
            case 4:
                statuses.push_back(token);
                break;
        }
        action = (action + 1) % 5;
        replyStr.erase(0, pos + SENTINEL.length());
    }

    constexpr uint OUTPUT_WIDTH = WZ_W/2;
    constexpr uint OUTPUT_HEIGHT = OUTPUT_WIDTH/2;
    constexpr uint X_POS = WZ_X + (WZ_W-OUTPUT_WIDTH)/2;
    constexpr uint Y_POS = WZ_Y+WZ_H/5;

    for (size_t i = 0; i < orderIds.size(); ++i)
    {
        std::string putValue = "order_id: " + orderIds[i] + '\n' + "user_id: " + userIds[i] + '\n';
        putValue += "order_date: " + orderDates[i] + '\n' + "address: " + addresses[i] + '\n';
        putValue += "status: " + statuses[i] + '\n' + '\n' + "Товары:\n";

        std::string replyStr = ZmqRequest(s,"GetCurrOrderItems"+SENTINEL+orderIds[i]);

        // Проверяем, содержит ли ответ ошибку
        if (replyStr.find("Invalid") != std::string::npos)
        {
            std::cerr << replyStr << std::endl;
            exit(1);
        }

        std::vector<std::string> orderItemsIds;
        std::vector<std::string> guitarIds;
        std::vector<std::string> quantities;

        size_t pos = 0;
        int action = 0;
        while ((pos = replyStr.find(SENTINEL)) != std::string::npos) 
        {
            std::string token = replyStr.substr(0, pos);
            switch (action)
            {
                case 0:
                    orderItemsIds.push_back(token);
                    break;
                case 1:
                    guitarIds.push_back(token);
                    break;
                case 2:
                    quantities.push_back(token);
                    break;
            }
            action = (action + 1) % 3;
            replyStr.erase(0, pos + SENTINEL.length());
        }

        for (size_t i = 0; i < orderItemsIds.size(); ++i)
        {
            putValue += "order_item_id: " + orderItemsIds[i] + '\n' + "guitar_id" + guitarIds[i] + '\n';
            putValue += "quantity: " + quantities[i] + '\n' + '\n';
        }

        usWidgets.push_back(new TMultilineOutput({X_POS, Y_POS+i*5*OUTPUT_HEIGHT/4},
            OUTPUT_WIDTH,OUTPUT_HEIGHT));
        userScroll->Attach(*usWidgets.back());
        reinterpret_cast<TMultilineOutput*>(usWidgets.back())->Put(putValue);
        reinterpret_cast<TMultilineOutput*>(usWidgets.back())->Wrap(true);
    }
}

void FreeAll2()
{
    for (size_t i = 0; i < usWidgets.size(); ++i) {
        delete usWidgets[i];
    }
    usWidgets.clear();

    for (size_t i = 0; i < usImages.size(); ++i) {
        delete usImages[i];
    }
    usImages.clear();

    if (rargs) 
    {
        delete rargs;
        rargs = nullptr;
    }

    if (ordArgs) 
    {
        delete ordArgs;
        ordArgs = nullptr;
    }

    if (userScroll)
    {
        delete userScroll;
        userScroll = nullptr;
    }
}

void FreeResources()
{
    delete signUp;
    delete signIn;
    delete profile;
    delete cart;
    delete back;
    delete workzoneGroup;
    delete userGroup;
    delete uargs;
    delete adminPersAcc;

    if (userInfo) {
        delete userInfo;
    }
}