#include "ui.h"

const int16_t INVALID_DIALOG_ID = -1;

enum eDialogStyle : uint8_t
{
    
};

class DialogBox : public SimpleUI
{
public:
    virtual void Draw();
    
    int16_t m_nCurrentDialogID;
    eDialogStyle m_nCurrentDialogStyle;
    char m_szDialogTitle[64];
    char m_szDialogBody[256];
    char m_szDialogButton[2][32];
};

extern DialogBox* dialogui;
