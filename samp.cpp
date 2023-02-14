#include <mod/amlmod.h>
#include <samp.h>
#include <gtasa.h>
#include <sampnet/sampnet.h>

// Our things
uintptr_t pJumpOutTo;
bool bSAMPStarted = false;

// Things from GTASA library to make our injects correct
uintptr_t TheCamera;
void (*RenderMotionBlur)(uintptr_t camera);
void (*Render2DStuff)();

extern "C" void InGameLoop()
{
    RenderMotionBlur(TheCamera);
    Render2DStuff();

    if(!samp)
    {
        SAMPNet::StartSAMP();
        samp->Connect("185.128.106.195", 7777);
        return;
    }

    samp->Update();
}

__attribute__((optnone)) __attribute__((naked)) void GameIdleLoop_stub(void)
{
    asm("PUSH {R0-R11}");

    asm("BL InGameLoop");
    asm volatile("MOV R12, %0\n" :: "r"(pJumpOutTo));

    asm("POP {R0-R11}");
    asm("BX R12");
}

void InitializeSAMP()
{
    pJumpOutTo = pGTASA + 0x3F6BEC + 0x1;
    TheCamera = aml->GetSym(hGTASA, "TheCamera");
    SET_TO(RenderMotionBlur, aml->GetSym(hGTASA, "_ZN7CCamera16RenderMotionBlurEv"));
    SET_TO(Render2DStuff, aml->GetSym(hGTASA, "_Z13Render2dStuffv"));
    aml->Redirect(pGTASA + 0x3F6BE4 + 0x1, (uintptr_t)GameIdleLoop_stub);
}
