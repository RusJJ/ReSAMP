#include "ui.h"

#include "spawnscreen.h"

#include <mod/amlmod.h>
#include <mod/logger.h>

IImGui* imgui;

static void DoDrawUIs()
{
    spawnui->Draw();
}

void InitializeDearImGui()
{
    imgui = (IImGui*)GetInterface("ImGui");
    if(!imgui)
    {
        // Uh-Oh, something is wrong...
        logger->Error("Ayo, we got no ImGui");
        return;
    }
    
    imgui->AddRenderListener((void*)DoDrawUIs);
}
