#include "../include/GUI.h"

TWindow::TWindow(std::pair<int, int> xy, int w, int h, const std::string& l)
    : loc{xy}, width{w}, height{h}, label{l}
{
    winPoint = new Fl_Window(loc.first, loc.second, width, height, label.c_str());
    winPoint->end();
    //winPoint->resizable(winPoint);
    winPoint->show();
}

void TWindow::Attach(IWidget& w)
{
    winPoint->add(w.GetP());
    w.SetW(this);
}

void TWindow::Attach(IWidgetCon& w)
{
    winPoint->add(w.GetP());
    w.SetW(this);
}

void TWindow::Attach(TImage& i)
{
    winPoint->add(i.GetP());
    i.SetW(this);
}

void TWindow::Redraw()
{
    winPoint->redraw();
}

void TWindow::SetColor(const Fl_Color& c)
{
    winPoint->color(c);
}

void GuiMain() { Fl::run(); }
