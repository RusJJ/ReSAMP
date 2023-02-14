#include <mod/amlmod.h>
#include <mod/logger.h>
#include <mod/config.h>
#include <dlfcn.h>
#include <samp.h>
#include <game/game.h>
#include <game/scripting.h>
#include <ui/ui.h>

MYMODCFG(net.rusjj.resamp, GTA:ReSAMP, 1.0, RusJJ)
NEEDGAME(com.rockstargames.gtasa)



uintptr_t pGTASA;
void* hGTASA;

extern "C" void OnModPreLoad()
{
    logger->SetTag("ReSAMP");

    pGTASA = aml->GetLib("libGTASA.so");
    hGTASA = aml->GetLibHandle("libGTASA.so");
    
    Game::InitializeGameClass();
    InitializeScripting();
    PatchTheGame();
    WorkWithPools();
    HookFunctions();
    InitializePadHooks();
    
    InitializeSAMP();
}

extern "C" void OnModLoad()
{
    InitializeDearImGui();
}

extern "C" void OnAllModsLoaded()
{
    HookFunctionsLate();
}
