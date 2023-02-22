#include "ui.h"

class DialogBox : public SimpleUI
{
public:
    virtual void Draw();
};

extern DialogBox* dialogui;
