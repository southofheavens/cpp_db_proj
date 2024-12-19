#include "../include/GUI.h"

// ---------------------------------------------

void IWidget::Move(int dx, int dy)
{
    Hide();
    pw->position(loc.first+=dx, loc.second+=dy);
    Show(); 
}

// ---------------------------------------------

TButton::TButton(std::pair<int,int> xy, int w, int h,
        const std::string& l, void (*cb)(Fl_Widget*,void*), void* a)
{
    loc = xy;
    width = w;
    height = h;
    label = l;
    callback = cb;
    arg = a;
    pw = new Fl_Button(loc.first,loc.second,width,height,label.c_str());
    if (callback) {
        pw->callback(callback,arg);
    }
}

// ---------------------------------------------

TInBox::TInBox(std::pair<int,int> xy, int w, int h,
        const std::string& l)
{
    loc = xy;
    width = w;
    height = h;
    label = l;
    pw = new Fl_Input(loc.first, loc.second, width, height, label.c_str());
}

// ---------------------------------------------

TOutBox::TOutBox(std::pair<int,int> xy, int w, int h,
       const std::string& l)
{
    loc = xy;
    width = w;
    height = h;
    label = l;
    pw = new Fl_Output(loc.first, loc.second, width, height, label.c_str());
}

// ---------------------------------------------

TBox::TBox(std::pair<int,int> xy, int w, int h,
            const std::string& l)
{
    loc = xy;
    width = w;
    height = h;
    label = l;
    pw = new Fl_Box(loc.first, loc.second, width, height, label.c_str());
}

void TBox::SetType(const Fl_Boxtype& t)
{
    pw->box(t);
}

void TBox::SetFont(const Fl_Font& f)
{
    pw->labelfont(f);
}

void TBox::SetSize(const Fl_Fontsize& s)
{
    pw->labelsize(s);
}

void TBox::SetColor(const Fl_Color& c)
{
    pw->labelcolor(c);
}

// ---------------------------------------------

TMultilineOutput::TMultilineOutput(std::pair<int,int> xy, int w, int h,
        const std::string& l)
{
    loc = xy;
    width = w;
    height = h;
    label = l;
    pw = new Fl_Multiline_Output(loc.first, loc.second, width, height, label.c_str());
}

void TMultilineOutput::Wrap(bool b)
{
    if (b) {
        reinterpret_cast<Fl_Multiline_Output*>(pw)->wrap(1);
    }
    else {
        reinterpret_cast<Fl_Multiline_Output*>(pw)->wrap(0);
    }
}

// ---------------------------------------------

// =*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=

// ---------------------------------------------

TScroll::TScroll(std::pair<int,int> xy, int w, int h)
{
    loc = xy;
    width = w;
    height = h;
    pw = new Fl_Scroll(loc.first, loc.second, width, height);
    reinterpret_cast<Fl_Scroll*>(pw)->end();
}

void TScroll::Attach(IWidget& w)
{
    reinterpret_cast<Fl_Scroll*>(pw)->add(w.GetP());
}

void TScroll::Attach(TImage& i)
{
    reinterpret_cast<Fl_Scroll*>(pw)->add(i.GetP());
}

void TScroll::Attach(IWidgetCon& wc)
{
    reinterpret_cast<Fl_Scroll*>(pw)->add(wc.GetP());
}

// ---------------------------------------------

TGroup::TGroup(std::pair<int,int> xy, int w, int h, const std::string& l)
{
    loc = xy;
    width = w;
    height = h;
    label = l;
    pw = new Fl_Group(loc.first, loc.second, width, height, label.c_str());
    reinterpret_cast<Fl_Group*>(pw)->end();
}

void TGroup::Attach(IWidget& w)
{
    reinterpret_cast<Fl_Group*>(pw)->add(w.GetP());
}

void TGroup::Attach(TImage& i)
{
    reinterpret_cast<Fl_Group*>(pw)->add(i.GetP());
}

void TGroup::Attach(IWidgetCon& wc)
{
    reinterpret_cast<Fl_Group*>(pw)->add(wc.GetP());
}

// ---------------------------------------------

TTabs::TTabs(std::pair<int,int> xy, int w, int h, const std::string& l)
{
    loc = xy;
    width = w;
    height = h;
    label = l;
    pw = new Fl_Tabs(loc.first, loc.second, width, height, label.c_str());
    reinterpret_cast<Fl_Group*>(pw)->end();
}

void TTabs::Attach(IWidget& w)
{
    reinterpret_cast<Fl_Tabs*>(pw)->add(w.GetP());
}

void TTabs::Attach(TImage& i)
{
    reinterpret_cast<Fl_Tabs*>(pw)->add(i.GetP());
}

void TTabs::Attach(IWidgetCon& wc)
{
    reinterpret_cast<Fl_Tabs*>(pw)->add(wc.GetP());
}

// ---------------------------------------------

TWizard::TWizard(std::pair<int,int> xy, int w, int h)
{
    loc = xy;
    width = w;
    height = h;
    pw = new Fl_Wizard(loc.first, loc.second, width, height);
    reinterpret_cast<Fl_Wizard*>(pw)->end();
}

void TWizard::Attach(IWidget& w)
{
    reinterpret_cast<Fl_Wizard*>(pw)->add(w.GetP());
}

void TWizard::Attach(TImage& i)
{
    reinterpret_cast<Fl_Wizard*>(pw)->add(i.GetP());
}

void TWizard::Attach(IWidgetCon& wc)
{
    reinterpret_cast<Fl_Wizard*>(pw)->add(wc.GetP());
}

const uint TWizard::Children() const
{
    return static_cast<uint>(reinterpret_cast<Fl_Wizard*>(pw)->children());
}

Fl_Widget* TWizard::Child(uint n) const
{
    return reinterpret_cast<Fl_Wizard*>(pw)->child(n);
}

void TWizard::Value(Fl_Widget* w)
{
    reinterpret_cast<Fl_Wizard*>(pw)->value(w);
}

// ---------------------------------------------

// =*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=

// ---------------------------------------------

TImage::TImage(std::pair<int,int> xy, const std::string& p)
{
    loc = xy;
    path = p;
    pImg = new Fl_JPEG_Image(path.c_str());
    if (pImg->fail()) throw std::runtime_error("Error loading image!");
    pBox = new Fl_Box(loc.first, loc.second, pImg->w(), pImg->h());
    pBox->image(*pImg);
}
