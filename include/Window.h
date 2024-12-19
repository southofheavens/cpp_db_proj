#ifndef WINDOW_H
#define WINDOW_H

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <vector>
#include <string>

const uint WIN_W = 1024;
const uint WIN_H = 768;

class IWidget;
class IWidgetCon;
class TImage;

class TWindow
{
private:
    std::pair<int,int> loc;
    int width;
    int height;
    std::string label;
    Fl_Window* winPoint;
    
public:
    TWindow(std::pair<int, int> c, int w, int h, const std::string& t = "");
    
    void Attach(IWidget&);
    void Attach(IWidgetCon&);
    void Attach(TImage&);
    void Redraw();
    void SetColor(const Fl_Color&);
};

void GuiMain();

#endif // WINDOW_H