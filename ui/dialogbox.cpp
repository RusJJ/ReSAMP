#include "dialogbox.h"
#include <game/localplayer.h>

static bool bButtonsInited = false;

void DialogBox::Draw()
{
    if(!ShouldDraw()) return;
 
    imgui->Begin("DialogBox", nullptr, 
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | 
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings);

    imgui->Text("ahahshs");

    imgui->SetWindowSize(ImVec2(-1, -1));
    ImVec2 size = imgui->GetWindowSize();
    
    imgui->SetWindowPos(ImVec2(0.5f * (imgui->GetScreenSizeX() - size.x), (0.5f * (imgui->GetScreenSizeY() - size.y))));
    //imgui->SetWindowPos(ImVec2(800, 600));
	imgui->End();
}

static DialogBox dialoguiLocal;
DialogBox* dialogui = &dialoguiLocal;
