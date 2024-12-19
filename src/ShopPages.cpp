#include "../include/ShopPages.h"
#include "../include/UserTools.h"

std::vector<IWidget*> wzWidgets;
std::vector<TImage*> wzImages;
std::vector<TArgs*> args;

TGroup* workzoneGroup = nullptr; 

TScroll* scroll = nullptr;

TAddToCartArgs* atca = nullptr;

void InitShopPages(zmq::socket_t& s, TWindow* w)
{
    workzoneGroup = new TGroup({WIN_W/16,WIN_H/10},WIN_W-WIN_W/8,WIN_H-WIN_H/5);
    w->Attach(*workzoneGroup);

    CategoriesPage(s, w);
}

void CategoriesPage(zmq::socket_t& s, TWindow* w)
{
    FreeAll();
    w->Redraw();

    wzWidgets.push_back(new TInBox({WZ_X+WZ_W/3,WZ_Y+WZ_H/10},WZ_W/3,WZ_H/20,"Поиск: "));
    workzoneGroup->Attach(*wzWidgets.back());

    args.push_back(new TArgs(&s,w,wzWidgets.back()));

    wzWidgets.push_back(new TButton({WZ_X+WZ_W/3+WZ_W/10,WZ_Y+WZ_H/6},WZ_W/8,WZ_H/18,"Найти"));
    wzWidgets.back()->SetCallback([](Fl_Widget*,void* a)
    {
        TArgs* args = reinterpret_cast<TArgs*>(a);
        SearchPage(*args->s,args->w,args->widget);
    }, reinterpret_cast<void*>(args.back()));
    workzoneGroup->Attach(*wzWidgets.back());

    std::string replyStr = ZmqRequest(s,"GetCategoryPreviews");

    // Проверяем, содержит ли ответ ошибку
    if (replyStr.find("Invalid") != std::string::npos)
    {
        std::cerr << replyStr << std::endl;
        exit(1);
    }

    std::vector<std::string> paths;
    std::vector<int> categoryIds;
    std::vector<std::string> categoryNames;

    size_t pos = 0;

    int action = 0; // 0 - paths, 1 - categoryIds, 2 - categoryNames
    // Разбивает полученный ответ на строки, разделенные сентинелом
    while ((pos = replyStr.find(SENTINEL)) != std::string::npos) 
    {
        std::string token = replyStr.substr(0, pos);
        switch (action)
        {
            case 0:
                paths.push_back(token);
                break;
            case 1:
                categoryIds.push_back(stoi(token));
                break;
            case 2:
                categoryNames.push_back(token);
                break;
        }
        action = (action + 1) % 3;
        replyStr.erase(0, pos + SENTINEL.length());
    }

    for (size_t i = 0; i < categoryIds.size(); ++i)
    {
        wzWidgets.push_back(new TButton({3*WIN_W/16 + (PREV_W+X_DIST) * i ,2*WIN_H/5},PREV_W,PREV_H));
        args.push_back(new TArgs(&s,w,categoryIds[i]));                   
        wzWidgets.back()->SetCallback([](Fl_Widget*,void* a)     
        {
            TArgs* args = reinterpret_cast<TArgs*>(a);
            CertainCatPage(*args->s,args->w,args->cid); 
        }, reinterpret_cast<void*>(args.back()));
        workzoneGroup->Attach(*wzWidgets.back());
    }

    for (size_t i = 0; i < paths.size(); ++i)
    {
        wzImages.push_back(new TImage({3*WIN_W/16 + (PREV_W+X_DIST) * i, 2*WIN_H/5},paths[i]));
        workzoneGroup->Attach(*wzImages[i]);
    }

    for (size_t i = 0; i < categoryNames.size(); ++i)
    {
        wzWidgets.push_back(new TMultilineOutput(
            {3*WIN_W/16 + (PREV_W+X_DIST) * i, 2*WIN_H/5 + PREV_H},
            PREV_W, PREV_H/5));
        uint lastIndex = wzWidgets.size() - 1;
        if (categoryNames[i] == "acoustic") {
            reinterpret_cast<TMultilineOutput*>(wzWidgets[lastIndex])->Put("Акустические гитары");
        }
        else if (categoryNames[i] == "bass") {
            reinterpret_cast<TMultilineOutput*>(wzWidgets[lastIndex])->Put("Бас-гитары");
        }
        else {
            reinterpret_cast<TMultilineOutput*>(wzWidgets[lastIndex])->Put("Электрогитары");
        }
        workzoneGroup->Attach(*wzWidgets[lastIndex]);
    }
}

void CertainCatPage(zmq::socket_t& s, TWindow* w, uint cid)
{    
    FreeAll();
    w->Redraw();

    std::string replyStr = ZmqRequest(s,"GetGuitars"+SENTINEL+std::to_string(cid));

    // Проверяем, содержит ли ответ ошибку
    if (replyStr.find("Invalid") != std::string::npos)
    {
        std::cerr << replyStr << std::endl;
        exit(1);
    }

    std::vector<std::string> pathsToPrev;
    std::vector<int> ids;
    std::vector<std::string> names;
    std::vector<int> prices;
    std::vector<std::string> pathsToPhoto;

    size_t pos = 0;
    int action = 0;
    while ((pos = replyStr.find(SENTINEL)) != std::string::npos) 
    {
        std::string token = replyStr.substr(0, pos);
        switch (action)
        {
            case 0:
                pathsToPrev.push_back(token);
                break;
            case 1:
                ids.push_back(stoi(token));
                break;
            case 2:
                names.push_back(token);
                break;
            case 3:
                prices.push_back(stoi(token));
                break;
            case 4:
                pathsToPhoto.push_back(token);
                break;
        }
        action = (action + 1) % 5;
        replyStr.erase(0, pos + SENTINEL.length());
    }
    
    constexpr uint widthOfSlider = 16;

    scroll = new TScroll({WIN_W/16,WIN_H/10},WIN_W-WIN_W/8,WIN_H-WIN_H/5);
    workzoneGroup->Attach(*scroll);

    args.push_back(new TArgs(&s,w));
    wzWidgets.push_back(new TButton({WIN_W/12,WIN_H/8},WIN_W/20,WIN_H/20,"@<-",[](Fl_Widget*,void* a)
    {
        TArgs* arg = reinterpret_cast<TArgs*>(a);
        CategoriesPage(*arg->s,arg->w);
    }, reinterpret_cast<void*>(args.back())));

    scroll->Attach(*wzWidgets.back());

    for (size_t i = 0, xPos = 3*WIN_W/16,
        yPos = CAT_PHOTO_H + PREV_H/5 + 3*WIN_H/25, iteration = 0; i < pathsToPrev.size(); ++i)
    {
        args.push_back(new TArgs(&s,w,cid,ids[i]));
        wzWidgets.push_back(new TButton({xPos, yPos}, PREV_W, PREV_H, "", [](Fl_Widget*,void* a)
        {
            TArgs* arg = reinterpret_cast<TArgs*>(a);
            GuitarPage(*arg->s, arg->w, arg->cid, arg->aid);
        }, reinterpret_cast<void*>(args.back())));

        scroll->Attach(*wzWidgets.back());

        if (iteration % 2 == 0 && iteration != 0)
        {
            xPos -= (PREV_W + X_DIST) * 2;
            yPos += (PREV_H + Y_DIST);
            iteration = 0;
        }
        else 
        {
            xPos += (PREV_W + X_DIST);
            ++iteration;
        }

        // Размещаем кнопку "Добавить гитару"
        if (userInfo && userInfo->type == TType::admin && i == names.size() - 1)
        {
            if (iteration == 1 || iteration == 2) {
                yPos += (PREV_H + Y_DIST);
            }
            xPos = 3*WIN_W/16 + (PREV_W + X_DIST);
            wzWidgets.push_back(new TButton({xPos, yPos}, PREV_W, PREV_H/2, "Добавить гитару"));
            scroll->Attach(*wzWidgets.back());
            args.push_back(new TArgs(&s,w,cid));
            wzWidgets.back()->SetCallback([](Fl_Widget*,void* a)
            {
                TArgs* arg = reinterpret_cast<TArgs*>(a);
                AddGuitar(*arg->s,arg->w,arg->cid);
            }, reinterpret_cast<void*>(args.back()));
        }
    }

    for (size_t i = 0, xPos = 3*WIN_W/16,
        yPos = CAT_PHOTO_H + PREV_H/5 + 3*WIN_H/25, iteration = 0; i < pathsToPrev.size(); ++i)
    {
        wzImages.push_back(new TImage({xPos, yPos}, pathsToPrev[i]));
        scroll->Attach(*wzImages[i]);

        if (iteration % 2 == 0 && iteration != 0)
        {
            xPos -= (PREV_W + X_DIST) * 2;
            yPos += (PREV_H + Y_DIST);
            iteration = 0;
        }
        else 
        {
            xPos += (PREV_W + X_DIST);
            ++iteration;
        }
    }

    for (size_t i = 0, xPos = 3*WIN_W/16,
        yPos = CAT_PHOTO_H + 6*PREV_H/5 + 3*WIN_H/25, iteration = 0; i < names.size(); ++i)
    {
        wzWidgets.push_back(new TMultilineOutput({xPos, yPos}, PREV_W, PREV_H/3));
        std::string val = "Цена: " + std::to_string(prices[i]) + "\n" + names[i];
        reinterpret_cast<TMultilineOutput*>(wzWidgets.back())->Put(val);
        scroll->Attach(*wzWidgets.back());

        if (iteration % 2 == 0 && iteration != 0)
        {
            xPos -= (PREV_W + X_DIST) * 2;
            yPos += (PREV_H + Y_DIST);
            iteration = 0;
        }
        else
        {
            xPos += (PREV_W + X_DIST);
            ++iteration;
        }
    }
                                                    // расстояние между началом первой фотографии
    constexpr uint phDist = 3*PREV_W+2*X_DIST;      // до конца последней фотографии
    constexpr uint xPos = 3*WIN_W/16 + (phDist - CAT_PHOTO_W)/2;
    constexpr uint yPos = 3*WIN_H/25;

    // Фотография
    wzImages.push_back(new TImage({xPos, yPos}, pathsToPhoto[0]));
    scroll->Attach(*wzImages[wzImages.size()-1]);
}

void GuitarPage(zmq::socket_t& s, TWindow* w, uint cid, uint aid)
{
    FreeAll();
    w->Redraw();

    std::string replyStr = ZmqRequest(s,"GetCurrGuitar"+SENTINEL+std::to_string(aid));

    // Проверяем, содержит ли ответ ошибку
    if (replyStr.find("Invalid") != std::string::npos)
    {
        std::cerr << replyStr << '\n';
        exit(1);
    }

    struct Types
    {
        std::string s;
        uint u;
        bool b;

        Types(const std::string& ss) : s{ss} {}
        Types(const uint& uu) : u{uu} {}
        Types(const bool& bb) : b{bb} {}
    };
    std::vector<Types> elems;

    // str description - 0, uint numOfStrings - 1, str design - 2, str scale - 3, str frame - 4,
    // str neck - 5, str fingerboard - 6, uint fingerboardRadius - 7, str color - 8, uint nutWidth - 9,
    // str pegs - 10, str fingerboardSensor - 11, str bridgeSensor - 12, str electronics - 13,
    // str strings - 14, bool caseIncluded - 15, str countryOfOrigin - 16, uint inStock - 17,
    // str pathToPhoto - 18, int guitarId - 19, int guitarAttrId - 20

    size_t pos = 0;
    // std::string delimiter = "@";
    int action = 0; 

    // Разбивает полученный ответ на строки, разделенные символом '@'
    while ((pos = replyStr.find(SENTINEL)) != std::string::npos) 
    {
        std::string token = replyStr.substr(0, pos);
        switch (action)
        {
            case 0: case 2: case 3: case 4: case 5: case 6: 
            case 8: case 10: case 11: case 12: case 13: 
            case 14: case 16: case 18:
                elems.push_back(Types(token));
                break;

            case 1: case 7: case 9: case 17: case 19:
                elems.push_back(Types(static_cast<uint>(stoi(token))));
                break;
            
            case 15:
                if (token == "f") {
                    elems.push_back(Types(false));
                }
                else {
                    elems.push_back(Types(true));
                }
                break;
        }
        action = (action + 1) % 20;
        replyStr.erase(0, pos + SENTINEL.length());
    }

    scroll = new TScroll({WIN_W/16,WIN_H/10},WIN_W-WIN_W/8,WIN_H-WIN_H/5);
    workzoneGroup->Attach(*scroll);

    // Размещаем кнопку <-
    args.push_back(new TArgs(&s,w,cid));
    wzWidgets.push_back(new TButton({WIN_W/12,WIN_H/8},WIN_W/20,WIN_H/20,"@<-",[](Fl_Widget*,void* a)
    {
        TArgs* arg = reinterpret_cast<TArgs*>(a);
        CertainCatPage(*arg->s,arg->w,arg->cid);
    }, reinterpret_cast<void*>(args.back())));
    scroll->Attach(*wzWidgets[0]);

    constexpr uint xPos = WIN_W/16 + (WIN_W-WIN_W/8 - PHOTO_W)/2;
    constexpr uint yPos = 3*WIN_H/25;

    // Размещаем фото         
    wzImages.push_back(new TImage
    (
        {xPos, yPos},
        elems[18].s
    ));
    scroll->Attach(*wzImages[0]);
    
    // Размещаем описание
    wzWidgets.push_back(new TMultilineOutput({xPos, yPos+11*PHOTO_H/10}, 
        PHOTO_W, 3*PHOTO_H/4));
    reinterpret_cast<TMultilineOutput*>(wzWidgets.back())->Put(elems[0].s);
    reinterpret_cast<TMultilineOutput*>(wzWidgets.back())->Wrap(true);
    scroll->Attach(*wzWidgets.back());

    // Размещаем количество струн
    wzWidgets.push_back(new TBox({xPos, yPos+39*PHOTO_H/20}, 
        PHOTO_W/5, PHOTO_H/9,"Количество струн"));
    scroll->Attach(*wzWidgets.back());
    wzWidgets.push_back(new TMultilineOutput({xPos+PHOTO_W/5, yPos+39*PHOTO_H/20}, 
        PHOTO_W/5, PHOTO_H/9));
    scroll->Attach(*wzWidgets.back());
    reinterpret_cast<TMultilineOutput*>(wzWidgets.back())->Put(std::to_string(elems[1].u));

    // Размещаем конструкцию
    wzWidgets.push_back(new TBox({xPos, yPos+43*PHOTO_H/20}, 
        PHOTO_W/5, PHOTO_H/9,"Конструкция"));
    scroll->Attach(*wzWidgets.back());
    wzWidgets.push_back(new TMultilineOutput({xPos+PHOTO_W/5, yPos+43*PHOTO_H/20}, 
        PHOTO_W/5, PHOTO_H/9));
    scroll->Attach(*wzWidgets.back());
    reinterpret_cast<TMultilineOutput*>(wzWidgets.back())->Put(elems[2].s);

    // Размещаем мензуру
    wzWidgets.push_back(new TBox({xPos, yPos+47*PHOTO_H/20}, 
        PHOTO_W/5, PHOTO_H/9,"Мензура"));
    scroll->Attach(*wzWidgets.back());
    wzWidgets.push_back(new TMultilineOutput({xPos+PHOTO_W/5, yPos+47*PHOTO_H/20}, 
        PHOTO_W/5, PHOTO_H/9));
    scroll->Attach(*wzWidgets.back());
    reinterpret_cast<TMultilineOutput*>(wzWidgets.back())->Put(elems[3].s);

    // Размещаем корпус
    wzWidgets.push_back(new TBox({xPos, yPos+51*PHOTO_H/20}, 
        PHOTO_W/5, PHOTO_H/9,"Корпус"));
    scroll->Attach(*wzWidgets.back());
    wzWidgets.push_back(new TMultilineOutput({xPos+PHOTO_W/5, yPos+51*PHOTO_H/20}, 
        PHOTO_W/5, PHOTO_H/9));
    scroll->Attach(*wzWidgets.back());
    reinterpret_cast<TMultilineOutput*>(wzWidgets.back())->Put(elems[4].s);

    // Размещаем гриф
    wzWidgets.push_back(new TBox({xPos, yPos+55*PHOTO_H/20}, 
        PHOTO_W/5, PHOTO_H/9,"Гриф"));
    scroll->Attach(*wzWidgets.back());
    wzWidgets.push_back(new TMultilineOutput({xPos+PHOTO_W/5, yPos+55*PHOTO_H/20}, 
        PHOTO_W/5, PHOTO_H/9));
    scroll->Attach(*wzWidgets.back());
    reinterpret_cast<TMultilineOutput*>(wzWidgets.back())->Put(elems[5].s);

    // Размещаем накладку грифа
    wzWidgets.push_back(new TBox({xPos, yPos+59*PHOTO_H/20}, 
        PHOTO_W/5, PHOTO_H/9,"Накладка грифа"));
    scroll->Attach(*wzWidgets.back());
    wzWidgets.push_back(new TMultilineOutput({xPos+PHOTO_W/5, yPos+59*PHOTO_H/20}, 
        PHOTO_W/5, PHOTO_H/9));
    scroll->Attach(*wzWidgets.back());
    reinterpret_cast<TMultilineOutput*>(wzWidgets.back())->Put(elems[6].s);

    // Размещаем радиус накладки
    wzWidgets.push_back(new TBox({xPos, yPos+63*PHOTO_H/20}, 
        PHOTO_W/5, PHOTO_H/9,"Радиус накладки"));
    scroll->Attach(*wzWidgets.back());
    wzWidgets.push_back(new TMultilineOutput({xPos+PHOTO_W/5, yPos+63*PHOTO_H/20}, 
        PHOTO_W/5, PHOTO_H/9));
    scroll->Attach(*wzWidgets.back());
    reinterpret_cast<TMultilineOutput*>(wzWidgets.back())->Put(std::to_string(elems[7].u) + " мм");

    // Размещаем цвет
    wzWidgets.push_back(new TBox({xPos, yPos+67*PHOTO_H/20}, 
        PHOTO_W/5, PHOTO_H/9,"Цвет"));
    scroll->Attach(*wzWidgets.back());
    wzWidgets.push_back(new TMultilineOutput({xPos+PHOTO_W/5, yPos+67*PHOTO_H/20}, 
        PHOTO_W/5, PHOTO_H/9));
    scroll->Attach(*wzWidgets.back());
    reinterpret_cast<TMultilineOutput*>(wzWidgets.back())->Put(elems[8].s);

    // Размещаем ширину верхнего порожка
    wzWidgets.push_back(new TBox({xPos + 2.9*PHOTO_W/5, yPos+39*PHOTO_H/20},
        PHOTO_W/5, PHOTO_H/9,"Ширина верх. порожка"));
    scroll->Attach(*wzWidgets.back());
    wzWidgets.push_back(new TMultilineOutput({xPos+4*PHOTO_W/5, yPos+39*PHOTO_H/20}, 
        PHOTO_W/5, PHOTO_H/9));
    scroll->Attach(*wzWidgets.back());
    reinterpret_cast<TMultilineOutput*>(wzWidgets.back())->Put(std::to_string(elems[9].u) + " мм");

    // Размещаем колки
    wzWidgets.push_back(new TBox({xPos + 2.9*PHOTO_W/5, yPos+43*PHOTO_H/20}, 
        PHOTO_W/5, PHOTO_H/9,"Колки"));
    scroll->Attach(*wzWidgets.back());
    wzWidgets.push_back(new TMultilineOutput({xPos+4*PHOTO_W/5, yPos+43*PHOTO_H/20}, 
        PHOTO_W/5, PHOTO_H/9));
    scroll->Attach(*wzWidgets.back());
    reinterpret_cast<TMultilineOutput*>(wzWidgets.back())->Put(elems[10].s);

    // Размещаем датчик у грифа
    wzWidgets.push_back(new TBox({xPos + 2.9*PHOTO_W/5, yPos+47*PHOTO_H/20}, 
        PHOTO_W/5, PHOTO_H/9,"Датчик у грифа"));
    scroll->Attach(*wzWidgets.back());
    wzWidgets.push_back(new TMultilineOutput({xPos+4*PHOTO_W/5, yPos+47*PHOTO_H/20}, 
        PHOTO_W/5, PHOTO_H/9));
    scroll->Attach(*wzWidgets.back());
    reinterpret_cast<TMultilineOutput*>(wzWidgets.back())->Put(elems[11].s);

    // Размещаем датчик у бриджа
    wzWidgets.push_back(new TBox({xPos + 2.9*PHOTO_W/5, yPos+51*PHOTO_H/20}, 
        PHOTO_W/5, PHOTO_H/9,"Датчик у бриджа"));
    scroll->Attach(*wzWidgets.back());
    wzWidgets.push_back(new TMultilineOutput({xPos+4*PHOTO_W/5, yPos+51*PHOTO_H/20}, 
        PHOTO_W/5, PHOTO_H/9));
    scroll->Attach(*wzWidgets.back());
    reinterpret_cast<TMultilineOutput*>(wzWidgets.back())->Put(elems[12].s);

    // Размещаем электронику
    wzWidgets.push_back(new TBox({xPos + 2.9*PHOTO_W/5, yPos+55*PHOTO_H/20}, 
        PHOTO_W/5, PHOTO_H/9,"Электроника"));
    scroll->Attach(*wzWidgets.back());
    wzWidgets.push_back(new TMultilineOutput({xPos+4*PHOTO_W/5, yPos+55*PHOTO_H/20}, 
        PHOTO_W/5, PHOTO_H/9));
    scroll->Attach(*wzWidgets.back());
    reinterpret_cast<TMultilineOutput*>(wzWidgets.back())->Put(elems[13].s);

    // Размещаем струны
    wzWidgets.push_back(new TBox({xPos + 2.9*PHOTO_W/5, yPos+59*PHOTO_H/20}, 
        PHOTO_W/5, PHOTO_H/9,"Струны"));
    scroll->Attach(*wzWidgets.back());
    wzWidgets.push_back(new TMultilineOutput({xPos+4*PHOTO_W/5, yPos+59*PHOTO_H/20}, 
        PHOTO_W/5, PHOTO_H/9));
    scroll->Attach(*wzWidgets.back());
    reinterpret_cast<TMultilineOutput*>(wzWidgets.back())->Put(elems[14].s);

    // Размещаем кейс в комплекте
    wzWidgets.push_back(new TBox({xPos + 2.9*PHOTO_W/5, yPos+63*PHOTO_H/20}, 
        PHOTO_W/5, PHOTO_H/9,"Кейс в комплекте"));
    scroll->Attach(*wzWidgets.back());
    wzWidgets.push_back(new TMultilineOutput({xPos+4*PHOTO_W/5, yPos+63*PHOTO_H/20}, 
        PHOTO_W/5, PHOTO_H/9));
    scroll->Attach(*wzWidgets.back());
    if (elems[15].b) {
        reinterpret_cast<TMultilineOutput*>(wzWidgets.back())->Put("Да");
    }
    else {
        reinterpret_cast<TMultilineOutput*>(wzWidgets.back())->Put("Нет");
    }

    // Размещаем страну производства
    wzWidgets.push_back(new TBox({xPos + 2.9*PHOTO_W/5, yPos+67*PHOTO_H/20}, 
        PHOTO_W/5, PHOTO_H/9,"Страна производства"));
    scroll->Attach(*wzWidgets.back());
    wzWidgets.push_back(new TMultilineOutput({xPos+4*PHOTO_W/5, yPos+67*PHOTO_H/20}, 
        PHOTO_W/5, PHOTO_H/9));
    scroll->Attach(*wzWidgets.back());
    reinterpret_cast<TMultilineOutput*>(wzWidgets.back())->Put(elems[16].s);

    // Размещаем количество 
    if (elems[17].u > 0)
    {
        wzWidgets.push_back(new TBox({xPos + 1.45*PHOTO_W/5, yPos+71*PHOTO_H/20}, 
            PHOTO_W/5, PHOTO_H/9,"В наличии: "));
        scroll->Attach(*wzWidgets.back());
        wzWidgets.push_back(new TMultilineOutput({xPos+PHOTO_W/2, yPos+71*PHOTO_H/20}, 
            PHOTO_W/5, PHOTO_H/9));
        scroll->Attach(*wzWidgets.back());
        reinterpret_cast<TMultilineOutput*>(wzWidgets.back())->Put(std::to_string(elems[17].u));

        if (userInfo && userInfo->type == TType::user)   // Если выполнен вход
        {  
            // "Окошечко" с количеством добавляемых в корзину гитар
            wzWidgets.push_back(new TOutBox({WZ_X+WZ_W-250+PHOTO_H/9,yPos+71*PHOTO_H/20},
                2*PHOTO_H/9,PHOTO_H/9));
            scroll->Attach(*wzWidgets.back());

            std::string request = "GetCartItemQuantity" + SENTINEL;
            request += std::to_string(elems[19].u) + SENTINEL + std::to_string(userInfo->userId); 
            std::string replyStr = ZmqRequest(s,request);
                    
            size_t pos = (replyStr.find(SENTINEL) != std::string::npos);
            std::string quantity = replyStr.substr(0, pos);

            if (quantity.length() != 0) {
                reinterpret_cast<TOutBox*>(wzWidgets.back())->Put(quantity);
            }
            else {
                reinterpret_cast<TOutBox*>(wzWidgets.back())->Put("0");
            }

            atca = new TAddToCartArgs(&s, reinterpret_cast<TOutBox*>(wzWidgets.back()),
                elems[19].u, userInfo->userId, elems[17].u);

            wzWidgets.push_back(new TButton({WZ_X+WZ_W-250,yPos+71*PHOTO_H/20},
                PHOTO_H/9,PHOTO_H/9,"-"));
            wzWidgets.back()->SetCallback([](Fl_Widget*, void* a)
            {
                TAddToCartArgs* atca = reinterpret_cast<TAddToCartArgs*>(a);

                std::string request = "GetCartItemQuantity" + SENTINEL;
                request += std::to_string(atca->guitarId) + SENTINEL + std::to_string(atca->userId); 
                std::string replyStr = ZmqRequest(*atca->s,request);
                size_t pos = (replyStr.find(SENTINEL) != std::string::npos);
                std::string quantity = replyStr.substr(0, pos);
                if (quantity.length() != 0)
                {
                    uint q = std::stoi(quantity);
                    q--;
                    atca->pob->Put(std::to_string(q));
                    // запрос на обновление количества товара
                    if (q == 0) 
                    {
                        std::string request = "DeleteCartItem" + SENTINEL;
                        request += std::to_string(atca->guitarId) + SENTINEL + std::to_string(atca->userId); 
                        std::string replyStr = ZmqRequest(*atca->s,request);
                    }
                    else
                    {
                        std::string request = "UpdateCartItemQuantity" + SENTINEL;
                        request += std::to_string(atca->guitarId) + SENTINEL + std::to_string(atca->userId);
                        request += SENTINEL + "minus";
                        std::string replyStr = ZmqRequest(*atca->s,request);
                    }
                }
            }, reinterpret_cast<void*>(atca));
            scroll->Attach(*wzWidgets.back());

            wzWidgets.push_back(new TButton({WZ_X+WZ_W-250+3*PHOTO_H/9,yPos+71*PHOTO_H/20},
                PHOTO_H/9,PHOTO_H/9,"+"));
            wzWidgets.back()->SetCallback([](Fl_Widget*, void* a)
            {
                TAddToCartArgs* atca = reinterpret_cast<TAddToCartArgs*>(a);
                        
                std::string request = "GetCartItemQuantity" + SENTINEL;
                request += std::to_string(atca->guitarId) + SENTINEL + std::to_string(atca->userId); 
                std::string replyStr = ZmqRequest(*atca->s,request);
                size_t pos = (replyStr.find(SENTINEL) != std::string::npos);
                std::string quantity = replyStr.substr(0, pos);

                if (quantity.length() == 0)
                {
                    std::string request = "CreateCartItem" + SENTINEL;
                    request += std::to_string(atca->guitarId) + SENTINEL + std::to_string(atca->userId);
                    std::string replyStr = ZmqRequest(*atca->s,request);

                    atca->pob->Put("1");
                }
                else
                {
                    if (std::stoi(quantity)+1 <= atca->guitarCount)
                    {
                        std::string request = "UpdateCartItemQuantity" + SENTINEL;
                        request += std::to_string(atca->guitarId) + SENTINEL + std::to_string(atca->userId);
                        request += SENTINEL + "plus";
                        std::string replyStr = ZmqRequest(*atca->s,request);

                        atca->pob->Put(std::to_string(std::stoi(quantity)+1));
                    }
                }
            }, reinterpret_cast<void*>(atca));
            scroll->Attach(*wzWidgets.back());
        }
    }
    else
    {
        wzWidgets.push_back(new TBox({xPos + 2*PHOTO_W/5, yPos+71*PHOTO_H/20}, 
            PHOTO_W/5, PHOTO_H/9,"Нет в наличии"));
        scroll->Attach(*wzWidgets.back());
    }

    // Размещаем кнопку "Удалить гитару"
    if (userInfo && userInfo->type == TType::admin)
    {
        wzWidgets.push_back(new TButton({xPos + 2*PHOTO_W/5, yPos+75*PHOTO_H/20},PHOTO_W/5,PHOTO_H/9,"Удалить гитару"));
        scroll->Attach(*wzWidgets.back());
        args.push_back(new TArgs(&s,w,elems[19].u));
        wzWidgets.back()->SetCallback([](Fl_Widget*,void* a)
        {
            TArgs* ar = reinterpret_cast<TArgs*>(a);
            DeleteGuitar(*ar->s,ar->w,ar->cid);
        }, reinterpret_cast<void*>(args.back()));
    }
}

void SearchPage(zmq::socket_t& s, TWindow* w, IWidget* iw)
{
    std::string request = "GetGuitarsByName"+SENTINEL+reinterpret_cast<TInBox*>(iw)->GetV();
    std::string replyStr = ZmqRequest(s,request);

    FreeAll();
    w->Redraw();

    // Проверяем, содержит ли ответ ошибку
    if (replyStr.find("Invalid") != std::string::npos)
    {
        std::cerr << replyStr << '\n';
        exit(1);
    }

    std::vector<std::string> pathsToPrev;
    std::vector<int> ids; // attributeIds
    std::vector<std::string> names;
    std::vector<int> prices;
    std::vector<uint> gids; // guitar_ids

    size_t pos = 0;

    int action = 0; 
    // Разбивает полученный ответ на строки
    while ((pos = replyStr.find(SENTINEL)) != std::string::npos) 
    {
        std::string token = replyStr.substr(0, pos);
        switch (action)
        {
            case 0:
                pathsToPrev.push_back(token);
                break;
            case 1:
                ids.push_back(stoi(token));
                break;
            case 2:
                names.push_back(token);
                break;
            case 3:
                prices.push_back(stoi(token));
                break;
            case 4:
                gids.push_back(stoi(token));
                break;
        }
        action = (action + 1) % 5;
        replyStr.erase(0, pos + SENTINEL.length());
    }

    std::vector<uint> categories(pathsToPrev.size());
    for (size_t i = 0; i < categories.size(); ++i)
    {
        std::string request = "GetGuitarCategory"+SENTINEL+std::to_string(gids[i]);
        std::string replyStr = ZmqRequest(s,request);

        // Проверяем, содержит ли ответ ошибку
        if (replyStr.find("Invalid") != std::string::npos)
        {
            std::cerr << replyStr << '\n';
            exit(1);
        }
        categories[i] = std::stoi(replyStr);
    }

    constexpr uint widthOfSlider = 16;

    scroll = new TScroll({WIN_W/16,WIN_H/10},WIN_W-WIN_W/8,WIN_H-WIN_H/5);
    workzoneGroup->Attach(*scroll);

    args.push_back(new TArgs(&s,w));
    wzWidgets.push_back(new TButton({WIN_W/12,WIN_H/8},WIN_W/20,WIN_H/20,"@<-",[](Fl_Widget*,void* a)
    {
        TArgs* arg = reinterpret_cast<TArgs*>(a);
        CategoriesPage(*arg->s,arg->w);
    }, reinterpret_cast<void*>(args.back())));

    scroll->Attach(*wzWidgets.back());

    // -------------------------------------------------------------------------

    // -------------------------------------------------------------------------

    for (size_t i = 0, xPos = 3*WIN_W/16,
            yPos = PREV_H/5 + 3*WIN_H/25, iteration = 0; i < pathsToPrev.size(); ++i)
    {
        args.push_back(new TArgs(&s,w,categories[i],ids[i]));
        wzWidgets.push_back(new TButton({xPos, yPos}, PREV_W, PREV_H, "", [](Fl_Widget*,void* a)
        {
            TArgs* arg = reinterpret_cast<TArgs*>(a);
            GuitarPage(*arg->s, arg->w, arg->cid, arg->aid);
        }, reinterpret_cast<void*>(args.back())));

        scroll->Attach(*wzWidgets.back());

        if (iteration % 2 == 0 && iteration != 0)
        {
            xPos -= (PREV_W + X_DIST) * 2;
            yPos += (PREV_H + Y_DIST);
            iteration = 0;
        }
        else 
        {
            xPos += (PREV_W + X_DIST);
            ++iteration;
        }
    }

    for (size_t i = 0, xPos = 3*WIN_W/16,
        yPos = PREV_H/5 + 3*WIN_H/25, iteration = 0; i < pathsToPrev.size(); ++i)
    {
        wzImages.push_back(new TImage({xPos, yPos}, pathsToPrev[i]));
        scroll->Attach(*wzImages[i]);

        if (iteration % 2 == 0 && iteration != 0)
        {
            xPos -= (PREV_W + X_DIST) * 2;
            yPos += (PREV_H + Y_DIST);
            iteration = 0;
        }
        else 
        {
            xPos += (PREV_W + X_DIST);
            ++iteration;
        }
    }

    for (size_t i = 0, xPos = 3*WIN_W/16,
        yPos = 6*PREV_H/5 + 3*WIN_H/25, iteration = 0; i < names.size(); ++i)
    {
        wzWidgets.push_back(new TMultilineOutput({xPos, yPos}, PREV_W, PREV_H/3));
        std::string val = "Цена: " + std::to_string(prices[i]) + "\n" + names[i];
        reinterpret_cast<TMultilineOutput*>(wzWidgets.back())->Put(val);
        scroll->Attach(*wzWidgets.back());

        if (iteration % 2 == 0 && iteration != 0)
        {
            xPos -= (PREV_W + X_DIST) * 2;
            yPos += (PREV_H + Y_DIST);
            iteration = 0;
        }
        else
        {
            xPos += (PREV_W + X_DIST);
            ++iteration;
        }
    }
}

void AddGuitar(zmq::socket_t& s, TWindow* w, uint cid)
{
    FreeAll();
    w->Redraw();

    constexpr uint SCROLLBAR_WIDTH = 16;
    scroll = new TScroll({WIN_W/16,WIN_H/10},
        WIN_W-WIN_W/8+SCROLLBAR_WIDTH,WIN_H-WIN_H/5);
    workzoneGroup->Attach(*scroll);

    // Размещаем кнопку <-
    args.push_back(new TArgs(&s,w,cid));
    wzWidgets.push_back(new TButton({WIN_W/12,WIN_H/8},WIN_W/20,WIN_H/20,"@<-",[](Fl_Widget*,void* a)
    {
        TArgs* arg = reinterpret_cast<TArgs*>(a);
        CertainCatPage(*arg->s,arg->w,arg->cid);
    }, reinterpret_cast<void*>(args.back())));
    scroll->Attach(*wzWidgets[0]);

    std::vector<TInBox*>* inBoxes = new std::vector<TInBox*>;

    // Размещаем надпись "Добавить гитару"
    wzWidgets.push_back(new TBox({WZ_X+(WZ_W-WZ_W/5)/2,WZ_Y+WZ_H/40},WZ_W/5,WZ_H/20,"Добавить гитару"));
    scroll->Attach(*wzWidgets.back());
    //reinterpret_cast<TBox*>(usWidgets.back())->SetColor(fl_rgb_color(123,104,238));
    reinterpret_cast<TBox*>(wzWidgets.back())->SetSize(30);
    reinterpret_cast<TBox*>(wzWidgets.back())->SetFont(FL_BOLD);

    constexpr uint xPos = WZ_X + WZ_W/5;
    constexpr uint yPos = WZ_Y + WZ_H/8;

    // Размещаем название - 0
    wzWidgets.push_back(new TInBox({xPos, yPos}, 
        PHOTO_W/5, PHOTO_H/9,"Название"));
    (*inBoxes).push_back(reinterpret_cast<TInBox*>(wzWidgets.back()));
    scroll->Attach(*wzWidgets.back());

    // Размещаем цену - 1
    wzWidgets.push_back(new TInBox({xPos, yPos+WZ_H/10}, 
        PHOTO_W/5, PHOTO_H/9,"Цена"));
    (*inBoxes).push_back(reinterpret_cast<TInBox*>(wzWidgets.back()));
    scroll->Attach(*wzWidgets.back());

    // Размещаем мензуру - 2
    wzWidgets.push_back(new TInBox({xPos, yPos+2*WZ_H/10}, 
        PHOTO_W/5, PHOTO_H/9,"Мензура"));
    (*inBoxes).push_back(reinterpret_cast<TInBox*>(wzWidgets.back()));
    scroll->Attach(*wzWidgets.back());

    // Размещаем корпус - 3
    wzWidgets.push_back(new TInBox({xPos, yPos+3*WZ_H/10}, 
        PHOTO_W/5, PHOTO_H/9,"Корпус"));
    (*inBoxes).push_back(reinterpret_cast<TInBox*>(wzWidgets.back()));
    scroll->Attach(*wzWidgets.back());

    // Размещаем гриф - 4
    wzWidgets.push_back(new TInBox({xPos, yPos+4*WZ_H/10}, 
        PHOTO_W/5, PHOTO_H/9,"Гриф"));
    (*inBoxes).push_back(reinterpret_cast<TInBox*>(wzWidgets.back()));
    scroll->Attach(*wzWidgets.back());

    // Размещаем накладку грифа - 5
    wzWidgets.push_back(new TInBox({xPos, yPos+5*WZ_H/10}, 
        PHOTO_W/5, PHOTO_H/9,"Накладка грифа"));
    (*inBoxes).push_back(reinterpret_cast<TInBox*>(wzWidgets.back()));
    scroll->Attach(*wzWidgets.back());

    // Размещаем радиус накладки - 6
    wzWidgets.push_back(new TInBox({xPos, yPos+6*WZ_H/10}, 
        PHOTO_W/5, PHOTO_H/9,"Радиус накладки"));
    (*inBoxes).push_back(reinterpret_cast<TInBox*>(wzWidgets.back()));
    scroll->Attach(*wzWidgets.back());

    // Размещаем цвет - 7
    wzWidgets.push_back(new TInBox({xPos, yPos+7*WZ_H/10}, 
        PHOTO_W/5, PHOTO_H/9,"Цвет"));
    (*inBoxes).push_back(reinterpret_cast<TInBox*>(wzWidgets.back()));
    scroll->Attach(*wzWidgets.back());

    // Размещаем путь к фото - 8
    wzWidgets.push_back(new TInBox({xPos, yPos+8*WZ_H/10}, 
        PHOTO_W/5, PHOTO_H/9,"Путь к фото"));
    (*inBoxes).push_back(reinterpret_cast<TInBox*>(wzWidgets.back()));
    scroll->Attach(*wzWidgets.back());

    // Размещаем путь к превью - 9
    wzWidgets.push_back(new TInBox({xPos, yPos+9*WZ_H/10}, 
        PHOTO_W/5, PHOTO_H/9,"Путь к превью"));
    (*inBoxes).push_back(reinterpret_cast<TInBox*>(wzWidgets.back()));
    scroll->Attach(*wzWidgets.back());

    // Размещаем артикул - 10
    wzWidgets.push_back(new TInBox({xPos, yPos+WZ_H}, 
        PHOTO_W/5, PHOTO_H/9,"Артикул"));
    (*inBoxes).push_back(reinterpret_cast<TInBox*>(wzWidgets.back()));
    scroll->Attach(*wzWidgets.back());

    // Размещаем ширину верхнего порожка - 11
    wzWidgets.push_back(new TInBox({xPos+WZ_W/2, yPos}, 
        PHOTO_W/5, PHOTO_H/9,"Ширина верхнего порожка"));
    (*inBoxes).push_back(reinterpret_cast<TInBox*>(wzWidgets.back()));
    scroll->Attach(*wzWidgets.back());

    // Размещаем колки - 12
    wzWidgets.push_back(new TInBox({xPos+WZ_W/2, yPos+WZ_H/10}, 
        PHOTO_W/5, PHOTO_H/9,"Колки"));
    (*inBoxes).push_back(reinterpret_cast<TInBox*>(wzWidgets.back()));
    scroll->Attach(*wzWidgets.back());
        
    // Размещаем датчик у грифа - 13
    wzWidgets.push_back(new TInBox({xPos+WZ_W/2, yPos+2*WZ_H/10}, 
        PHOTO_W/5, PHOTO_H/9,"Датчик у грифа"));
    (*inBoxes).push_back(reinterpret_cast<TInBox*>(wzWidgets.back()));
    scroll->Attach(*wzWidgets.back());

    // Размещаем датчик у бриджа - 14
    wzWidgets.push_back(new TInBox({xPos+WZ_W/2, yPos+3*WZ_H/10}, 
        PHOTO_W/5, PHOTO_H/9,"Датчик у бриджа"));
    (*inBoxes).push_back(reinterpret_cast<TInBox*>(wzWidgets.back()));
    scroll->Attach(*wzWidgets.back());

    // Размещаем электронику - 15
    wzWidgets.push_back(new TInBox({xPos+WZ_W/2, yPos+4*WZ_H/10}, 
        PHOTO_W/5, PHOTO_H/9,"Электроника (Активная/Пассивная)"));
    (*inBoxes).push_back(reinterpret_cast<TInBox*>(wzWidgets.back()));
    scroll->Attach(*wzWidgets.back());

    // Размещаем струны - 16
    wzWidgets.push_back(new TInBox({xPos+WZ_W/2, yPos+5*WZ_H/10}, 
        PHOTO_W/5, PHOTO_H/9,"Струны"));
    (*inBoxes).push_back(reinterpret_cast<TInBox*>(wzWidgets.back()));
    scroll->Attach(*wzWidgets.back());

    // Размещаем кейс в комплекте - 17
    wzWidgets.push_back(new TInBox({xPos+WZ_W/2, yPos+6*WZ_H/10}, 
        PHOTO_W/5, PHOTO_H/9,"Кейс в комплекте (Да/Нет)"));
    (*inBoxes).push_back(reinterpret_cast<TInBox*>(wzWidgets.back()));
    scroll->Attach(*wzWidgets.back());

    // Размещаем страну производства - 18
    wzWidgets.push_back(new TInBox({xPos+WZ_W/2, yPos+7*WZ_H/10}, 
        PHOTO_W/5, PHOTO_H/9,"Страна производства"));
    (*inBoxes).push_back(reinterpret_cast<TInBox*>(wzWidgets.back()));
    scroll->Attach(*wzWidgets.back());

    // Размещаем количество - 19
    wzWidgets.push_back(new TInBox({xPos+WZ_W/2, yPos+8*WZ_H/10}, 
        PHOTO_W/5, PHOTO_H/9,"Количество"));
    (*inBoxes).push_back(reinterpret_cast<TInBox*>(wzWidgets.back()));
    scroll->Attach(*wzWidgets.back());

    // Размещаем количество струн - 20
    wzWidgets.push_back(new TInBox({xPos+WZ_W/2, yPos+9*WZ_H/10}, 
        PHOTO_W/5, PHOTO_H/9,"Количество струн"));
    (*inBoxes).push_back(reinterpret_cast<TInBox*>(wzWidgets.back()));
    scroll->Attach(*wzWidgets.back());

    // Размещаем конструкцию - 21
    wzWidgets.push_back(new TInBox({xPos+WZ_W/2, yPos+WZ_H}, 
        PHOTO_W/5, PHOTO_H/9,"Конструкция"));
    (*inBoxes).push_back(reinterpret_cast<TInBox*>(wzWidgets.back()));
    scroll->Attach(*wzWidgets.back());

    // Размещаем описание - 22
    wzWidgets.push_back(new TInBox({xPos+1*WZ_W/8, yPos+11*WZ_H/10}, 
        PHOTO_W/2, PHOTO_H/3,"Описание"));
    (*inBoxes).push_back(reinterpret_cast<TInBox*>(wzWidgets.back()));
    scroll->Attach(*wzWidgets.back());

    // Размещаем кнопку "Добавить"
    args.push_back(new TArgs(&s,w,cid,inBoxes));
    wzWidgets.push_back(new TButton({WZ_X+7*WZ_W/8,WZ_Y},WZ_W/8,WZ_H/10,"Добавить"));
    scroll->Attach(*wzWidgets.back());   
    wzWidgets.back()->SetCallback(AddGuitarCb,reinterpret_cast<void*>(args.back()));
}

void AddGuitarCb(Fl_Widget*, void* a)
{
    TArgs* targs = reinterpret_cast<TArgs*>(a);

    std::vector<TInBox*> boxes = *(targs->pib);
    std::string request = "AddGuitarAttributes" + SENTINEL + boxes[22]->GetV();
    request += SENTINEL + boxes[20]->GetV() + SENTINEL + boxes[21]->GetV() + SENTINEL;
    request += boxes[2]->GetV() + SENTINEL + boxes[3]->GetV() + SENTINEL;
    request += boxes[4]->GetV() + SENTINEL + boxes[5]->GetV() + SENTINEL;
    request += boxes[6]->GetV() + SENTINEL + boxes[7]->GetV() + SENTINEL;
    request += boxes[11]->GetV() + SENTINEL + boxes[12]->GetV() + SENTINEL;
    request += boxes[13]->GetV() + SENTINEL + boxes[14]->GetV() + SENTINEL;
    request += boxes[15]->GetV() + SENTINEL + boxes[16]->GetV() + SENTINEL;
    request += boxes[17]->GetV() + SENTINEL + boxes[18]->GetV() + SENTINEL;
    request += boxes[19]->GetV() + SENTINEL + boxes[8]->GetV();

    std::string addGuitAttr = ZmqRequest(*targs->s,request);
    // Находим guitar_attr_id по path_to_photo
    std::string getGuitAttrId = ZmqRequest(*targs->s,"GetGuitarAttrId"+SENTINEL+boxes[8]->GetV());
    
    request = "AddGuitar" + SENTINEL + std::to_string(targs->cid) + SENTINEL;
    request += boxes[10]->GetV() + SENTINEL + boxes[0]->GetV() + SENTINEL;
    request += boxes[1]->GetV() + SENTINEL + boxes[9]->GetV() + SENTINEL;
    request += getGuitAttrId;

    std::string addGuitar = ZmqRequest(*targs->s,request);

    scroll->Hide();

    // Размещаем кнопку <-
    wzWidgets.push_back(new TButton({WIN_W/12,WIN_H/8},WIN_W/20,WIN_H/20,"@<-",[](Fl_Widget*,void* a)
    {
        TArgs* arg = reinterpret_cast<TArgs*>(a);
        CategoriesPage(*arg->s,arg->w);
    }, reinterpret_cast<void*>(targs)));
    workzoneGroup->Attach(*wzWidgets.back());

    // Размещаем надпись "Гитара успешно добавлена"
    wzWidgets.push_back(new TBox({WZ_X+(WZ_W-WZ_W/5)/2,WZ_Y+WZ_H/2},WZ_W/5,WZ_H/20,"Гитара успешно добавлена"));
    workzoneGroup->Attach(*wzWidgets.back());
    reinterpret_cast<TBox*>(wzWidgets.back())->SetColor(fl_rgb_color(50,205,50));
    reinterpret_cast<TBox*>(wzWidgets.back())->SetSize(30);
    reinterpret_cast<TBox*>(wzWidgets.back())->SetFont(FL_BOLD);
}

void DeleteGuitar(zmq::socket_t& s, TWindow* w, uint gid)
{
    std::string request = "DeleteGuitar" + SENTINEL + std::to_string(gid);

    std::string replyStr = ZmqRequest(s,request);
    
    if (replyStr.find("Invalid") != std::string::npos)
    {
        std::cerr << replyStr << '\n';
        exit(1);
    }

    for (size_t i = 0; i < wzWidgets.size(); ++i) {
        wzWidgets[i]->Hide();
    }
    for (size_t i = 0; i < wzImages.size(); ++i) {
        wzImages[i]->Hide();
    }

    args.push_back(new TArgs(&s,w));
    wzWidgets.push_back(new TButton({WIN_W/12,WIN_H/8},WIN_W/20,WIN_H/20,"@<-",[](Fl_Widget*,void* a)
    {
        TArgs* arg = reinterpret_cast<TArgs*>(a);
        CategoriesPage(*arg->s,arg->w);
    }, reinterpret_cast<void*>(args.back())));
    workzoneGroup->Attach(*wzWidgets.back());

    // Размещаем надпись "Гитара успешно удалена"
    wzWidgets.push_back(new TBox({WZ_X+(WZ_W-WZ_W/5)/2,WZ_Y+WZ_H/2},WZ_W/5,WZ_H/20,"Гитара успешно удалена"));
    workzoneGroup->Attach(*wzWidgets.back());
    reinterpret_cast<TBox*>(wzWidgets.back())->SetColor(fl_rgb_color(50,205,50));
    reinterpret_cast<TBox*>(wzWidgets.back())->SetSize(30);
    reinterpret_cast<TBox*>(wzWidgets.back())->SetFont(FL_BOLD);
}

void FreeAll()
{
    if (atca) 
    {
        delete atca;
        atca = nullptr;
    }

    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i]->pib) {
            delete args[i]->pib;
        }
        delete args[i];
    }
    args.clear();

    for (size_t i = 0; i < wzWidgets.size(); ++i) {
        delete wzWidgets[i];
    }
    for (size_t i = 0; i < wzImages.size(); ++i) {
        delete wzImages[i];
    }
    wzWidgets.clear();
    wzImages.clear();

    if (scroll)
    {
        delete scroll;
        scroll = nullptr;
    }          
}
