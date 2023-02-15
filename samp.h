#ifndef __SAMP_H
#define __SAMP_H

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <sampnet/sampnet.h>
#include <sampnet/samp_netencr.h>
#include <sampnet/samprpc.h>

#define SAMP_VERSION           "0.3.7-R4"

// SAMP LIMITS
#define MAX_PLAYERS             1004
#define MAX_VEHICLES            2000
#define MAX_OBJECTS             1000
#define MAX_MAPICONS            100
#define MAX_PICKUPS             4096
#define MAX_PLAYER_NAME         24


#define ATOMIC_MODELS_INFO      20000
#define ATOMIC_PEDMODELS_INFO   315
#define MAX_VEHICLES_IN_POOL    300

#define RAKSAMP_CLIENT
#define NETCODE_CONNCOOKIELULZ  0x6969
#define NETGAME_VERSION         4057
#define AUTH_BS                 "39FB2DEEDB49ACFB8D4EECE6953D2507988CCCF4410"

extern uintptr_t pGTASA;
extern void* hGTASA;
void Redirect(uintptr_t addr, uintptr_t to);
void PatchTheGame();
void HookFunctions();
void HookFunctionsLate();
void DoPoolsPatches();
void WorkWithPools();
void InitializeSAMP();
void InitializePadHooks();

// Just because it uses less instructions ¯\_(ツ)_/¯
inline size_t __strlen(const char *str)
{
    const char* s = str;
    while(*s) ++s;
    return (s - str);
}

#pragma pack(1)
struct VehicleData
{
    unsigned short VehicleID;
    int            iVehicleType;
    CVector        vecPos;
    float          fRotation;
    uint8_t        aColor1;
    uint8_t        aColor2;
    float          fHealth;
    uint8_t        byteInterior;
    uint32_t       dwDoorDamageStatus;
    uint32_t       dwPanelDamageStatus;
    uint8_t        byteLightDamageStatus;
    uint8_t        byteTireDamageStatus;
    uint8_t        byteAddSiren;
    uint8_t        byteModSlots[14];
    uint8_t        bytePaintjob;
    uint32_t       cColor1;
    uint32_t       cColor2;
    uint8_t        byteUnk;
};
#pragma pack(pop)

#endif // __SAMP_H
