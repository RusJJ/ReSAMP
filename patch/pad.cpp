#include <mod/amlmod.h>
#include <mod/logger.h>
#include <samp.h>
#include <gtasa.h>

#include "pad.h"
#include "game/game.h"
#include "game/localplayer.h"

PAD_KEYS LocalPlayerKeys = {0};
PAD_KEYS RemotePlayerKeys[MAX_PLAYERS] = {0};

uint16_t nCurPlayer = 0; bool bLocalPlayer = false;
uint16_t nCurDriver = 0; bool bLocalDriver = false;

char* PadOftenWriteAddr1;
char* PadOftenWriteAddr2;
char* PadOftenWriteAddr3;
char* PadOftenWriteAddr4;
char* ZeldaOftenWriteAddr1;
char* ZeldaOftenWriteAddr2;
char* ZeldaOftenWriteAddr3;
char* ZeldaOftenWriteAddr4;

void (*VehicleAudioEntityService)(CAEVehicleAudioEntity* aeEntity);
CPlayerPed* (*FindPlayerPed)(int);

void ProcessControl(CVehicle* pVehicle)
{
    CPed* pDriver = pVehicle->m_pDriver;
    if(pDriver == CLocalPlayer::GetEntity())
    {
        bLocalDriver = true;
        nCurDriver = CLocalPlayer::GetID();
    }
    else
    {
        bLocalDriver = false;
        if(pVehicle && pDriver)
        {
            unsigned short maxplayers = Game::m_pPlayerPool->GetHighestSlotUsedEver() + 1;
            for(int i = 0; i < maxplayers; ++i)
            {
                if(Game::m_pPlayerPool->GetAt(i)->GetEntity() == pDriver)
                {
                    nCurDriver = i;
                    break;
                }
            }
        }
    }
    if( pDriver && pDriver->m_nPedType == PED_TYPE_PLAYER1 && g_nCurrentPlayerInFocus == 0 && pDriver != FindPlayerPed(-1))
    {
        pDriver->m_nPedType = PED_TYPE_CIVMALE;
        VehicleAudioEntityService(&pVehicle->m_vehicleAudio);
        pDriver->m_nPedType = PED_TYPE_PLAYER1;
    } 
    else
    {
        VehicleAudioEntityService(&pVehicle->m_vehicleAudio);
    }
}

DECL_HOOKv(PadProcessControl, CPlayerPed* ped)
{
    //if(ped == CLocalPlayer::GetEntity())
    if(ped == Game::GetPlayerByGtaID(CLocalPlayer::GetGtaID()))
    {
        bLocalPlayer = true;
        nCurPlayer = CLocalPlayer::GetID();
        
        if(CLocalPlayer::m_bDisableControls) return;
        *ZeldaOftenWriteAddr1 = 0xC4; *ZeldaOftenWriteAddr2 = 0xF8;
        *ZeldaOftenWriteAddr3 = 0x60; *ZeldaOftenWriteAddr4 = 0x55;
        PadProcessControl(ped);
        *ZeldaOftenWriteAddr1 = 0x00; *ZeldaOftenWriteAddr2 = 0x46;
        *ZeldaOftenWriteAddr3 = 0x00; *ZeldaOftenWriteAddr4 = 0x46;
    }
    else
    {
        bLocalPlayer = false;
        unsigned short maxplayers = Game::m_pPlayerPool->GetHighestSlotUsedEver() + 1;
        for(int i = 0; i < maxplayers; ++i)
        {
            if(Game::m_pPlayerPool->GetAt(i)->GetEntity() == ped)
            {
                nCurPlayer = i;
                break;
            }
        }
        *PadOftenWriteAddr1 = 0x00; *PadOftenWriteAddr2 = 0x46;
        *PadOftenWriteAddr3 = 0x00; *PadOftenWriteAddr4 = 0x46;
        PadProcessControl(ped);
        *PadOftenWriteAddr1 = 0xF0; *PadOftenWriteAddr2 = 0xF4;
        *PadOftenWriteAddr3 = 0x42; *PadOftenWriteAddr4 = 0xEB;
    }
}
DECL_HOOKv(ProcessControl_Automobile, CVehicle* vehicle)
{
    if(vehicle == NULL) return;
    ProcessControl(vehicle);
    ProcessControl_Automobile(vehicle);
}
DECL_HOOKv(ProcessControl_Boat, CVehicle* vehicle)
{
    if(vehicle == NULL) return;
    ProcessControl(vehicle);
    ProcessControl_Boat(vehicle);
}
DECL_HOOKv(ProcessControl_Bike, CVehicle* vehicle)
{
    if(vehicle == NULL) return;
    ProcessControl(vehicle);
    ProcessControl_Bike(vehicle);
}
DECL_HOOKv(ProcessControl_Plane, CVehicle* vehicle)
{
    if(vehicle == NULL) return;
    ProcessControl(vehicle);
    ProcessControl_Plane(vehicle);
}
DECL_HOOKv(ProcessControl_Heli, CVehicle* vehicle)
{
    if(vehicle == NULL) return;
    ProcessControl(vehicle);
    ProcessControl_Heli(vehicle);
}
DECL_HOOKv(ProcessControl_Bmx, CVehicle* vehicle)
{
    if(vehicle == NULL) return;
    ProcessControl(vehicle);
    ProcessControl_Bmx(vehicle);
}
DECL_HOOKv(ProcessControl_MonsterTruck, CVehicle* vehicle)
{
    if(vehicle == NULL) return;
    ProcessControl(vehicle);
    ProcessControl_MonsterTruck(vehicle);
}
DECL_HOOKv(ProcessControl_QuadBike, CVehicle* vehicle)
{
    if(vehicle == NULL) return;
    ProcessControl(vehicle);
    ProcessControl_QuadBike(vehicle);
}
DECL_HOOKv(ProcessControl_Train, CVehicle* vehicle)
{
    if(vehicle == NULL) return;
    ProcessControl(vehicle);
    ProcessControl_Train(vehicle);
}

// Buttons pressing
DECL_HOOK(uint16_t, GetPedWalkLeftRight, uintptr_t pad)
{
    if(bLocalPlayer)
    {
        LocalPlayerKeys.wKeyLR = GetPedWalkLeftRight(pad);
        return LocalPlayerKeys.wKeyLR;
    }
    else
    {
        uint16_t dwResult = RemotePlayerKeys[nCurPlayer].wKeyLR;
        if((dwResult == 0xFF80 || dwResult == 0x80) && RemotePlayerKeys[nCurPlayer].bKeys[ePadKeys::KEY_WALK])
        {
            dwResult = 0x40;
        }
        return dwResult;
    }
}
DECL_HOOK(uint16_t, GetPedWalkUpDown, uintptr_t pad)
{
    if(bLocalPlayer)
    {
        LocalPlayerKeys.wKeyUD = GetPedWalkUpDown(pad);
        return LocalPlayerKeys.wKeyUD ;
    }
    else
    {
        uint16_t dwResult = RemotePlayerKeys[nCurPlayer].wKeyUD;
        if((dwResult == 0xFF80 || dwResult == 0x80) && RemotePlayerKeys[nCurPlayer].bKeys[ePadKeys::KEY_WALK])
        {
            dwResult = 0x40;
        }
        return dwResult;
    }
}
DECL_HOOK(bool, GetSprint, uintptr_t pad, int a1)
{
    if(bLocalPlayer)
    {
        LocalPlayerKeys.bKeys[ePadKeys::KEY_SPRINT] = GetSprint(pad, a1);
        return LocalPlayerKeys.bKeys[ePadKeys::KEY_SPRINT];
    }
    else
    {
        return RemotePlayerKeys[nCurPlayer].bKeys[ePadKeys::KEY_SPRINT];
    }
}
DECL_HOOK(bool, JumpJustDown, uintptr_t pad)
{
    if(bLocalPlayer)
    {
        LocalPlayerKeys.bKeys[ePadKeys::KEY_JUMP] = JumpJustDown(pad);
        return LocalPlayerKeys.bKeys[ePadKeys::KEY_JUMP];
    }
    else
    {
        if(!RemotePlayerKeys[nCurPlayer].bIgnoreJump && RemotePlayerKeys[nCurPlayer].bKeys[ePadKeys::KEY_JUMP] && !RemotePlayerKeys[nCurPlayer].bKeys[ePadKeys::KEY_HANDBRAKE])
        {
            RemotePlayerKeys[nCurPlayer].bIgnoreJump = true;
            return RemotePlayerKeys[nCurPlayer].bKeys[ePadKeys::KEY_JUMP];
        }
        return false;
    }
}
DECL_HOOK(bool, GetJump, uintptr_t pad)
{
    if(bLocalPlayer)
    {
        LocalPlayerKeys.bKeys[ePadKeys::KEY_JUMP] = GetJump(pad);
        return LocalPlayerKeys.bKeys[ePadKeys::KEY_JUMP];
    }
    else
    {
        if(RemotePlayerKeys[nCurPlayer].bIgnoreJump) return 0;
        return RemotePlayerKeys[nCurPlayer].bKeys[ePadKeys::KEY_JUMP];
    }
}
DECL_HOOK(bool, GetAutoClimb, uintptr_t pad)
{
    if(bLocalPlayer)
    {
        LocalPlayerKeys.bKeys[ePadKeys::KEY_JUMP] = GetAutoClimb(pad);
        return LocalPlayerKeys.bKeys[ePadKeys::KEY_JUMP];
    }
    else
    {
        return RemotePlayerKeys[nCurPlayer].bKeys[ePadKeys::KEY_JUMP];
    }
}
DECL_HOOK(bool, GetAbortClimb, uintptr_t pad)
{
    if(bLocalPlayer)
    {
        LocalPlayerKeys.bKeys[ePadKeys::KEY_SECONDARY_ATTACK] = GetAutoClimb(pad);
        return LocalPlayerKeys.bKeys[ePadKeys::KEY_SECONDARY_ATTACK];
    }
    else
    {
        return RemotePlayerKeys[nCurPlayer].bKeys[ePadKeys::KEY_SECONDARY_ATTACK];
    }
}
DECL_HOOK(bool, DiveJustDown)
{
    if(bLocalPlayer)
    {
        LocalPlayerKeys.bKeys[ePadKeys::KEY_FIRE] = DiveJustDown();
        return LocalPlayerKeys.bKeys[ePadKeys::KEY_FIRE];
    }
    else
    {
        return RemotePlayerKeys[nCurPlayer].bKeys[ePadKeys::KEY_FIRE];
    }
}
DECL_HOOK(bool, SwimJumpJustDown, uintptr_t pad)
{
    if(bLocalPlayer)
    {
        LocalPlayerKeys.bKeys[ePadKeys::KEY_JUMP] = SwimJumpJustDown(pad);
        return LocalPlayerKeys.bKeys[ePadKeys::KEY_JUMP];
    }
    else
    {
        return RemotePlayerKeys[nCurPlayer].bKeys[ePadKeys::KEY_JUMP];
    }
}
DECL_HOOK(bool, DuckJustDown, uintptr_t pad)
{
    if(bLocalPlayer)
    {
        LocalPlayerKeys.bKeys[ePadKeys::KEY_CROUCH] = DuckJustDown(pad);
        return LocalPlayerKeys.bKeys[ePadKeys::KEY_CROUCH];
    }
    else
    {
        return RemotePlayerKeys[nCurPlayer].bKeys[ePadKeys::KEY_CROUCH];
    }
}
DECL_HOOK(int, MeleeAttackJustDown, uintptr_t pad)
{
    if(bLocalPlayer)
    {
        LocalPlayerKeys.bKeys[ePadKeys::KEY_FIRE] = MeleeAttackJustDown(pad);
        return LocalPlayerKeys.bKeys[ePadKeys::KEY_FIRE];
    }
    else
    {
        if( RemotePlayerKeys[nCurPlayer].bKeys[ePadKeys::KEY_HANDBRAKE] &&
            RemotePlayerKeys[nCurPlayer].bKeys[ePadKeys::KEY_SECONDARY_ATTACK]) 
        {
            return 2;
        }
        return RemotePlayerKeys[nCurPlayer].bKeys[ePadKeys::KEY_FIRE];
    }
}
DECL_HOOK(bool, GetBlock, uintptr_t pad)
{
    if(bLocalPlayer)
    {
        return GetBlock(pad);
    }
    else
    {
        return (RemotePlayerKeys[nCurPlayer].bKeys[ePadKeys::KEY_JUMP] &&
                RemotePlayerKeys[nCurPlayer].bKeys[ePadKeys::KEY_HANDBRAKE]);
    }
}
DECL_HOOK(int16_t, GetSteeringLeftRight, uintptr_t pad)
{
    if(bLocalPlayer)
    {
        LocalPlayerKeys.wKeyLR = GetSteeringLeftRight(pad);
        return LocalPlayerKeys.wKeyLR;
    }
    else
    {
        return (int16_t)RemotePlayerKeys[nCurPlayer].wKeyLR;
    }
}
DECL_HOOK(int16_t, GetSteeringUpDown, uintptr_t pad)
{
    if(bLocalPlayer)
    {
        LocalPlayerKeys.wKeyUD = GetSteeringUpDown(pad);
        return LocalPlayerKeys.wKeyUD;
    }
    else
    {
        return (int16_t)RemotePlayerKeys[nCurPlayer].wKeyUD;
    }
}
DECL_HOOK(uint16_t, GetAccelerate, uintptr_t pad)
{
    if(bLocalPlayer)
    {
        uint16_t accel = GetAccelerate(pad);
        LocalPlayerKeys.bKeys[ePadKeys::KEY_SPRINT] = accel;
        return accel;
    }
    else
    {
        return RemotePlayerKeys[nCurPlayer].bKeys[ePadKeys::KEY_SPRINT] ? 0xFF : 0x00;
    }
}
DECL_HOOK(uint16_t, GetBrake, uintptr_t pad)
{
    if(bLocalPlayer)
    {
        uint16_t accel = GetBrake(pad);
        LocalPlayerKeys.bKeys[ePadKeys::KEY_JUMP] = accel;
        return accel;
    }
    else
    {
        return RemotePlayerKeys[nCurPlayer].bKeys[ePadKeys::KEY_JUMP] ? 0xFF : 0x00;
    }
}
DECL_HOOK(uint16_t, GetHandBrake, uintptr_t pad)
{
    if(bLocalPlayer)
    {
        uint16_t accel = GetHandBrake(pad);
        LocalPlayerKeys.bKeys[ePadKeys::KEY_HANDBRAKE] = accel;
        return accel;
    }
    else
    {
        return RemotePlayerKeys[nCurPlayer].bKeys[ePadKeys::KEY_HANDBRAKE] ? 0xFF : 0x00;
    }
}
DECL_HOOK(bool, GetHorn, uintptr_t pad)
{
    if(bLocalPlayer)
    {
        LocalPlayerKeys.bKeys[ePadKeys::KEY_CROUCH] = GetHorn(pad);
        return LocalPlayerKeys.bKeys[ePadKeys::KEY_CROUCH];
    }
    else
    {
        return RemotePlayerKeys[nCurPlayer].bKeys[ePadKeys::KEY_CROUCH];
    }
}

void InitializePadHooks()
{
    SET_TO(VehicleAudioEntityService,   aml->GetSym(hGTASA, "_ZN21CAEVehicleAudioEntity7ServiceEv"));
    SET_TO(FindPlayerPed,               aml->GetSym(hGTASA, "_Z13FindPlayerPedi"));

    aml->Unprot(pGTASA + 0x4A2A22, 4);
    SET_TO(PadOftenWriteAddr1,          pGTASA + 0x4A2A22 + 0x0);
    SET_TO(PadOftenWriteAddr2,          pGTASA + 0x4A2A22 + 0x1);
    SET_TO(PadOftenWriteAddr3,          pGTASA + 0x4A2A22 + 0x2);
    SET_TO(PadOftenWriteAddr4,          pGTASA + 0x4A2A22 + 0x3);

    aml->Unprot(pGTASA + 0x539BA6, 4);
    SET_TO(ZeldaOftenWriteAddr1,        pGTASA + 0x539BA6 + 0x0);
    SET_TO(ZeldaOftenWriteAddr2,        pGTASA + 0x539BA6 + 0x1);
    SET_TO(ZeldaOftenWriteAddr3,        pGTASA + 0x539BA6 + 0x2);
    SET_TO(ZeldaOftenWriteAddr4,        pGTASA + 0x539BA6 + 0x3);

    HOOKPLT(PadProcessControl,          pGTASA + 0x6692B4); // CPlayerPed: vtable + 0x2C

    HOOKPLT(ProcessControl_Automobile,  pGTASA + 0x66D6B4);
    HOOKPLT(ProcessControl_Boat,        pGTASA + 0x66DA5C);
    HOOKPLT(ProcessControl_Bike,        pGTASA + 0x66D82C);
    HOOKPLT(ProcessControl_Plane,       pGTASA + 0x66DDC0);
    HOOKPLT(ProcessControl_Heli,        pGTASA + 0x66DB70);
    HOOKPLT(ProcessControl_Bmx,         pGTASA + 0x66D944);
    HOOKPLT(ProcessControl_MonsterTruck,pGTASA + 0x66DC98);
    HOOKPLT(ProcessControl_QuadBike,    pGTASA + 0x66DEE8);
    HOOKPLT(ProcessControl_Train,       pGTASA + 0x66E138);

    // Pads
    HOOKPLT(GetPedWalkUpDown,           pGTASA + 0x6706D0);
    HOOKPLT(GetPedWalkLeftRight,        pGTASA + 0x671014);
    HOOKPLT(GetSprint,                  pGTASA + 0x670CE0);
    HOOKPLT(JumpJustDown,               pGTASA + 0x670274);
    HOOKPLT(GetJump,                    pGTASA + 0x66FAE0);
    HOOKPLT(GetAutoClimb,               pGTASA + 0x674A0C);
    HOOKPLT(DiveJustDown,               pGTASA + 0x672FD0);
    HOOKPLT(SwimJumpJustDown,           pGTASA + 0x674030);
    HOOKPLT(DuckJustDown,               pGTASA + 0x6727CC);
    HOOKPLT(MeleeAttackJustDown,        pGTASA + 0x67127C);
    HOOKPLT(GetBlock,                   pGTASA + 0x66FAD8);
    HOOKPLT(GetSteeringLeftRight,       pGTASA + 0x673D84);
    HOOKPLT(GetSteeringUpDown,          pGTASA + 0x672C14);
    HOOKPLT(GetAccelerate,              pGTASA + 0x67482C);
    HOOKPLT(GetBrake,                   pGTASA + 0x66EBE0);
    HOOKPLT(GetHandBrake,               pGTASA + 0x670514);
    HOOKPLT(GetHorn,                    pGTASA + 0x673010);
}
