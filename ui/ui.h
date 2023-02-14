#ifndef __UI_H
#define __UI_H

#include <string.h> // memset
#include <math.h> // FLT_MAX
#include <stdint.h> // size_t, NULL, etc...
#include "third/iimgui.h"

extern IImGui* imgui;
void InitializeDearImGui();

class SimpleUI
{
public:
    SimpleUI() : m_bDraw(false) {}
    virtual void Draw() {}
    virtual void DrawAlways() {}
    
    inline bool ShouldDraw() { return m_bDraw; }
    inline void SetDrawable(bool b) { m_bDraw = b; }
    
private:
    bool m_bDraw;
};

#endif // __UI_H
