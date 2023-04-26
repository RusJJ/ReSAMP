#include "dialogbox.h"
#include <game/localplayer.h>

static bool bButtonsInited = false;

void DialogBox::Draw()
{
    if(!ShouldDraw() || m_nCurrentDialogID < 0) return;
 
    imgui->Begin("DialogBox", nullptr, 
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | 
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings);

    imgui->SetWindowSize(ImVec2(imgui->GetScreenSizeY() * 0.8f, imgui->GetScreenSizeY() * 0.6f));
    ImVec2 size = imgui->GetWindowSize();
    
    imgui->Text("Title:%s", m_szDialogTitle);
    imgui->Separator();
    imgui->Text(" %s", m_szDialogBody);
    
    imgui->SetWindowPos(ImVec2(0.5f * (imgui->GetScreenSizeX() - size.x), (0.5f * (imgui->GetScreenSizeY() - size.y))));
	imgui->End();
}

static DialogBox dialoguiLocal;
DialogBox* dialogui = &dialoguiLocal;
