#include "spawnscreen.h"
#include <game/localplayer.h>

bool bButtonsInited = false;
float fBtnX, fBtnY;

void SpawnScreen::Draw()
{
    if(!ShouldDraw()) return;
    
    if(!bButtonsInited)
    {
        bButtonsInited = true;
        
        fBtnY = imgui->GetScreenSizeY() * 0.07f;
        fBtnX = 3.0f * fBtnY;
    }
    
    imgui->Begin("SpawnScreen", nullptr, 
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | 
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings);

    if (imgui->Button("<<", ImVec2(fBtnX, fBtnY)))
    {
        CLocalPlayer::ChangeClass(false);
        CLocalPlayer::RequestClass();
    }

    imgui->SameLine(0, 10);
    if (imgui->Button("SPAWN", ImVec2(fBtnX, fBtnY)))
    {
        CLocalPlayer::RequestSpawn();
    }

    imgui->SameLine(0, 10);
    if (imgui->Button(">>", ImVec2(fBtnX, fBtnY)))
    {
        CLocalPlayer::ChangeClass(true);
        CLocalPlayer::RequestClass();
    }

    imgui->SetWindowSize(ImVec2(-1, -1));
    ImVec2 size = imgui->GetWindowSize();
    
    imgui->SetWindowPos(ImVec2(0.5f * (imgui->GetScreenSizeX() - size.x), ((imgui->GetScreenSizeY() * 0.98) - size.y)));
    //imgui->SetWindowPos(ImVec2(800, 600));
	imgui->End();
}

static SpawnScreen spawnuiLocal;
SpawnScreen* spawnui = &spawnuiLocal;
