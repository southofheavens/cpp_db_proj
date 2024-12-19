#include "include/UserInteraction.h"
#include "include/ShopPages.h"

int main() 
try
{
    // Создание контекста ZeroMQ
    zmq::context_t context(1);

    // Создаем сокет типа REQ (запрос)
    zmq::socket_t socket(context, zmq::socket_type::req);

    // Подключение к серверу по адресу tcp://localhost:5555
    socket.connect("tcp://localhost:5555");

    TWindow* win = new TWindow({200, 30}, WIN_W, WIN_H, "Guitar Shop");
    TBox* workZone = new TBox({WZ_X,WZ_Y},WZ_W,WZ_H);
    workZone->SetType(FL_DOWN_BOX);

    win->Attach(*workZone);

    InitShopPages(socket, win);
    InitUserInteraction(socket, win);

    GuiMain();

    FreeAll();
    FreeAll2();
    FreeResources();
    delete win;
    delete workZone;

    ZmqRequest(socket,"Logout");

    return 0;
}
catch (const std::invalid_argument& e)
{
    std::cerr << e.what() << '\n';
    exit(1);
}
