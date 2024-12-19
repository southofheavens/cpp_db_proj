#ifndef GUI_H
#define GUI_H

#include <FL/Fl_Widget.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Output.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Box.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Wizard.H>
#include <FL/Fl_Image.H>
#include <FL/Fl_JPEG_Image.H>
#include <FL/Fl_Multiline_Output.H>
#include <string>
#include <iostream>
#include "Window.h"

// ---------------------------------------------

class IWidget
{
private:
    IWidget& operator=(const IWidget&) = delete; // don't copy Widgets
    IWidget(const IWidget&) = delete;
    
protected:
    IWidget() {};
    
    std::pair<int,int> loc;
    int width;
    int height;
    std::string label;
    
    void (*callback)(Fl_Widget*,void*) = nullptr;
    void* arg;

    TWindow* own;
    Fl_Widget* pw;
    
public:
    virtual ~IWidget() { delete pw; }

    void SetCallback(void (*cb)(Fl_Widget*,void*), void* a = nullptr) { callback = cb; arg = a; pw->callback(callback,arg); }
    void SetLabel(const std::string& l) { label = l; pw->copy_label(label.c_str()); }

    void Hide() { pw->hide(); }
    void Show() { pw->show(); }
    void Move(int, int);
    
    Fl_Widget* GetP() const { return pw; }
    void SetW(TWindow* w) { own = w; }
};

// ---------------------------------------------

class TButton : public IWidget
{
public:
    TButton(std::pair<int,int> xy, int w, int h,
            const std::string& l = "", void (*cb)(Fl_Widget*,void*) = nullptr, void* a = nullptr);
};

// ---------------------------------------------

class TInBox : public IWidget
{
public:
    TInBox(std::pair<int,int> xy, int w, int h,
           const std::string& l = "");

    std::string GetV() const { return pw ? reinterpret_cast<Fl_Input*>(pw)->value() : ""; }
};

// ---------------------------------------------

class TOutBox : public IWidget
{
public:
    TOutBox(std::pair<int,int> xy, int w, int h,
           const std::string& l = "");

    void Put(const std::string& s) { reinterpret_cast<Fl_Output*>(pw)->value(s.c_str()); }
};

// ---------------------------------------------

class TBox : public IWidget
{
public:
    TBox(std::pair<int,int> xy, int w, int h,
            const std::string& l = "");
    void SetType(const Fl_Boxtype&);
    void SetFont(const Fl_Font&);
    void SetSize(const Fl_Fontsize&);
    void SetColor(const Fl_Color&);
};

// ---------------------------------------------

class TMultilineOutput : public IWidget
{
public:
    TMultilineOutput(std::pair<int,int> xy, int w, int h,
        const std::string& l = "");

    void Put(const std::string& v) { reinterpret_cast<Fl_Multiline_Output*>(pw)->value(v.c_str()); }
    void Wrap(bool);              // Перенос строк (true - переносятся)
    // void TextFont(Fl_Font);
    // void TextSize(Fl_Fontsize);
    // void TextColor(Fl_Color);
};

// ---------------------------------------------

// =*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=

// ---------------------------------------------

class IWidgetCon
{
private:
    IWidgetCon& operator=(const IWidgetCon&) = delete; // don't copy Widget containers
    IWidgetCon(const IWidgetCon&) = delete;

protected:
    IWidgetCon() {}
    
    std::pair<int,int> loc;
    int width;
    int height;
    std::string label;

    TWindow* own;
    Fl_Widget* pw;

public:
    virtual ~IWidgetCon() { delete pw; }

    Fl_Widget* GetP() const { return pw; }
    void SetW(TWindow* w) { own = w; }

    void Hide() { pw->hide(); }
    void Show() { pw->show(); }
    
    virtual void Attach(IWidget&) = 0;
    virtual void Attach(TImage&) = 0;
    virtual void Attach(IWidgetCon&) = 0;
};

// ---------------------------------------------

class TScroll : public IWidgetCon
{
public:
    TScroll(std::pair<int,int> xy, int w, int h);
    void Attach(IWidget&);
    void Attach(TImage&);
    void Attach(IWidgetCon&);
};

// ---------------------------------------------

class TGroup : public IWidgetCon
{
public:
    TGroup(std::pair<int,int> xy, int w, int h, const std::string& l = "");
    void Attach(IWidget&);
    void Attach(TImage&);
    void Attach(IWidgetCon&);
};

//  ---------------------------------------------

class TTabs : public IWidgetCon
{
public:
    TTabs(std::pair<int,int> xy, int w, int h, const std::string& l = "");
    void Attach(IWidget&);
    void Attach(TImage&);
    void Attach(IWidgetCon&);
};

// ---------------------------------------------

class TWizard : public IWidgetCon
{
public:
    TWizard(std::pair<int,int> xy, int w, int h);
    void Attach(IWidget&);
    void Attach(TImage&);
    void Attach(IWidgetCon&);
    const uint Children() const;
    Fl_Widget* Child(uint) const;
    
    void Value(Fl_Widget*);
    void Next() { reinterpret_cast<Fl_Wizard*>(pw)->next(); }
    void Prev() { reinterpret_cast<Fl_Wizard*>(pw)->prev(); }
};

// ---------------------------------------------

// =*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=

// ---------------------------------------------

class TImage
{
private:
    std::pair<int,int> loc;
    int width;
    int height;
    std::string path;
    
    Fl_JPEG_Image* pImg;
    Fl_Box* pBox;
    TWindow* pWin;
    
public:
    TImage(std::pair<int,int> xy, const std::string& p);
    ~TImage() { delete pImg; delete pBox; }
    
    Fl_Box* GetP() const { return pBox; }
    void SetW(TWindow* w) { pWin = w; }

    void Hide() { pBox->hide(); }
    void Show() { pBox->show(); }
};

#endif // GUI_H