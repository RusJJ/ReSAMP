#include <mod/amlmod.h>
#include <mod/logger.h>
#include <samp.h>
#include <gtasa.h>
#include <unistd.h>

#include "game.h"
#include "localplayer.h"
#include "scripting.h"

extern CPlayerInfo* g_pPlayersContainer;
int g_nRemovedBuildings = 0;
BuildingToRemove* g_RemoveBuilding[MAX_BUILDINGS_TO_REMOVE] {0};

// Game::SetWorldTime
static char* ms_nGameClockHours;
static char* ms_nGameClockMinutes;
static int* ms_nLastClockTick;
static int* m_snTimeInMilliseconds;
// Game::SetWeather
static short* OldWeatherType;
static short* NewWeatherType;
static short* ForcedWeatherType;
// Game::ToggleEnterExits
static bool* EntryExitManagerDisabled;
// Game::ToggleClock
static char* clockFormat;
// Game::SetGravity
static float* fGravity;
// Game::RemoveBuilding
static uintptr_t PlaceableVtable;
static void (*DestructBuildingPtr)(CPlaceable*);
static void (*DestructDummyPtr)(CPlaceable*);
// Pool Getters
CVehicle* (*GetVehicleById)(int);
CPlayerPed* (*GetPoolPed)(int);
// Others
void (*ClearPedWeapons)(CPed*);
int (*GivePedWeapon)(CPed*, eWeaponType, uint32_t, bool);
void (*AddBigMessage)(uint16_t*, uint32_t, uint16_t);
void (*AsciiToGxt)(const char*, uint16_t*);

// STATIC INITIALIZATION
uint16_t* Game::m_pLatestBigMessage = new uint16_t[0xFF] {0};
int Game::m_nProcessedInPlayerInfo = -1;
bool Game::m_bDisableWidgets = false;
JPoolCalcHighest<CRemotePlayer>* Game::m_pPlayerPool = NULL;
JPoolCalcHighest<CRemoteVehicle>* Game::m_pVehiclePool = NULL;
JPoolCalcHighest<CRemoteObject>* Game::m_pObjectPool = NULL;
JPoolCalcHighest<CRemoteMapIcon>* Game::m_pMapIconPool = NULL;


void Game::InitializeGameClass()
{
    // Game::SetWorldTime
    SET_TO(ms_nGameClockHours,         aml->GetSym(hGTASA, "_ZN6CClock18ms_nGameClockHoursE"));
    SET_TO(ms_nGameClockMinutes,       aml->GetSym(hGTASA, "_ZN6CClock20ms_nGameClockMinutesE"));
    SET_TO(ms_nLastClockTick,          aml->GetSym(hGTASA, "_ZN6CClock17ms_nLastClockTickE"));
    SET_TO(m_snTimeInMilliseconds,     aml->GetSym(hGTASA, "_ZN6CTimer22m_snTimeInMillisecondsE"));

    // Game::SetWeather
    SET_TO(OldWeatherType,             aml->GetSym(hGTASA, "_ZN8CWeather14OldWeatherTypeE"));
    SET_TO(NewWeatherType,             aml->GetSym(hGTASA, "_ZN8CWeather14NewWeatherTypeE"));
    SET_TO(ForcedWeatherType,          aml->GetSym(hGTASA, "_ZN8CWeather17ForcedWeatherTypeE"));

    // Game::ToggleEnterExits
    SET_TO(EntryExitManagerDisabled,   aml->GetSym(hGTASA, "_ZN17CEntryExitManager12ms_bDisabledE"));

    // Game::ToggleClock
    SET_TO(clockFormat,                pGTASA + 0x60FEE4);
    aml->Unprot(pGTASA + 0x60FEE4,     10);

    // Game::SetGravity
    SET_TO(fGravity,                   pGTASA + 0x3FE7C0);
    aml->Unprot(pGTASA + 0x3FE7C0,     sizeof(float));

    // Game::RemoveBuilding
    SET_TO(PlaceableVtable,            pGTASA + 0x667D24);
    SET_TO(DestructBuildingPtr,        pGTASA + 0x28021C + 0x1);
    SET_TO(DestructDummyPtr,           pGTASA + 0x3EADB0 + 0x1);

    // Pools
    m_pPlayerPool = new JPoolCalcHighest<CRemotePlayer>(MAX_PLAYERS);
    m_pVehiclePool = new JPoolCalcHighest<CRemoteVehicle>(MAX_VEHICLES);
    m_pObjectPool = new JPoolCalcHighest<CRemoteObject>(MAX_OBJECTS);
    m_pMapIconPool = new JPoolCalcHighest<CRemoteMapIcon>(MAX_MAPICONS);

    // Pool Getters
    SET_TO(GetVehicleById,             aml->GetSym(hGTASA, "_ZN6CPools10GetVehicleEi"));
    SET_TO(GetPoolPed,                 aml->GetSym(hGTASA, "_Z10GetPoolPedi"));
    
    // Others
    SET_TO(ClearPedWeapons,            aml->GetSym(hGTASA, "_ZN4CPed12ClearWeaponsEv"));
    SET_TO(GivePedWeapon,              aml->GetSym(hGTASA, "_ZN4CPed10GiveWeaponE11eWeaponTypejb"));
    SET_TO(AddBigMessage,              aml->GetSym(hGTASA, "_ZN9CMessages13AddBigMessageEPtjt"));
    SET_TO(AsciiToGxt,                 aml->GetSym(hGTASA, "_Z14AsciiToGxtCharPKcPt"));
}

void Game::ShowBigMsg(const char* msg, int time, int type)
{
    if(msg) AsciiToGxt(msg, m_pLatestBigMessage);
    AddBigMessage(m_pLatestBigMessage, time, type);
}

void Game::ClearWeapons()
{
    ClearPedWeapons(CLocalPlayer::GetEntity());
}

void Game::GiveWeapon(int weapId, int ammo, bool armed)
{
    GivePedWeapon(CLocalPlayer::GetEntity(), (eWeaponType)weapId, ammo, true);
}

void Game::ShowWidgets(bool enabled)
{
    m_bDisableWidgets = !enabled;
}

CRemotePlayer* Game::CreatePlayer(int id, int skin, float x, float y, float z, float rot, bool createMarker)
{
    CRemotePlayer* player = m_pPlayerPool->AllocAt(id, true);
    if(id == 0) // Create local player???
    {
        // This may happen if we loaded in a non-empty server
        // So there's player with id 0
        id = CLocalPlayer::GetID();
    }
    player->m_nID = id;
    CALLSCM(CREATE_PLAYER, id, x, y, z, &player->m_nGtaID);
    CALLSCM(GET_PLAYER_CHAR, id, &player->m_nGtaID);

    CALLSCM(SET_CHAR_DROPS_WEAPONS_WHEN_DEAD, player->m_nGtaID, true);
    CALLSCM(SET_CHAR_PROOFS, player->m_nGtaID, false, false, true, false, false);
    CALLSCM(SET_CHAR_SUFFERS_CRITICAL_HITS, player->m_nGtaID, false);
    CALLSCM(SET_CHAR_MONEY, player->m_nGtaID, samp->GetServerVars().m_iDeathDropMoney);

    player->m_pEntity = GetPlayerByGtaID(player->m_nGtaID);
    Game::RequestModelNow(skin);
    player->m_pEntity->SetModelIndex(skin);
    //player->m_pEntity->GetEntityFlags() &= 0x1;

    return player;
}

CRemoteVehicle* Game::CreateVehicle(int id, int type, float x, float y, float z, float rot)
{
    CRemoteVehicle* vehicle = m_pVehiclePool->AllocAt(id, true);

    return vehicle;
}

CRemoteVehicle* Game::CreateVehicle(VehicleData& data)
{
    CRemoteVehicle* vehicle = m_pVehiclePool->AllocAt(data.VehicleID, true);
    vehicle->m_nMarkerID = 0;
    vehicle->data = data;
    vehicle->m_nID = data.VehicleID;
    int modelId = data.iVehicleType;
    Game::RequestModelNow(modelId);
    
    if(modelId == TRAIN_PASSENGER_LOCO || modelId == TRAIN_FREIGHT_LOCO || modelId == TRAIN_TRAM)
    {

    }
    else if(modelId == TRAIN_PASSENGER || modelId == TRAIN_FREIGHT)
    {

    }
    else
    {
        CALLSCM(CREATE_CAR, modelId, data.vecPos.x, data.vecPos.y, data.vecPos.z, &vehicle->m_nGtaID);
        CALLSCM(SET_CAR_HEADING, vehicle->m_nGtaID, data.fRotation);
        CALLSCM(SET_PETROL_TANK_WEAKPOINT, vehicle->m_nGtaID, false);
        CALLSCM(SET_CAR_HYDRAULICS, vehicle->m_nGtaID, false);
        CALLSCM(SET_CAN_BURST_CAR_TYRES, vehicle->m_nGtaID, false);

        vehicle->m_pEntity = Game::GetVehicleByGtaID(vehicle->m_nGtaID);
        vehicle->m_bDoorsLocked = false;

        if( vehicle->m_pEntity->m_nVehicleSubType != VEHICLE_SUBTYPE_BIKE && vehicle->m_pEntity->m_nVehicleSubType != VEHICLE_SUBTYPE_PUSHBIKE)
            vehicle->m_pEntity->GetPosition().z = data.vecPos.z + 0.25f;
    }
    
    CALLSCM(ADD_BLIP_FOR_CAR_OLD, vehicle->m_nGtaID, 1, 2, &vehicle->m_nMarkerID);
    CALLSCM(CHANGE_BLIP_COLOUR, vehicle->m_nMarkerID, 200);
    
    return vehicle;
}

bool Game::IsModelLoaded(int modelId)
{
    return CALLSCM(HAS_MODEL_LOADED, modelId) != 0;
}

void Game::RequestModel(int modelId)
{
    CALLSCM(REQUEST_MODEL, modelId);
}

void Game::RequestModelNow(int modelId)
{
    CALLSCM(REQUEST_MODEL, modelId);
    CALLSCM(LOAD_ALL_MODELS_NOW);
    while(CALLSCM(HAS_MODEL_LOADED, modelId) == 0) usleep(10);
}

void Game::LoadRequestedModels()
{
    CALLSCM(LOAD_ALL_MODELS_NOW);
}

void Game::RefreshStreamingAt(CVector2D posxy)
{
    CALLSCM(REQUEST_COLLISION, posxy.x, posxy.y);
}

CVehicle* Game::GetVehicleByGtaID(int gtaId)
{
    return GetVehicleById(gtaId);
}

CPlayerPed* Game::GetPlayerByGtaID(int gtaId)
{
    return GetPoolPed(gtaId);
}

void Game::SetWorldTime(char hour, char minute)
{
    *ms_nGameClockHours =   hour;
    *ms_nGameClockMinutes = minute;
    *ms_nLastClockTick =   *m_snTimeInMilliseconds;
}

void Game::SetWeather(short weather)
{
    *OldWeatherType =    weather;
    *NewWeatherType =    weather;
    *ForcedWeatherType = weather;
}

void Game::EnableZoneNames(bool enable)
{
    CALLSCM(DISPLAY_ZONE_NAMES, enable);
}

void Game::PlaySound(int sound, float x, float y, float z)
{
    CALLSCM(ADD_ONE_OFF_SOUND, x, y, z, sound);
}

void Game::PlaySound(int sound, CVector pos)
{
    CALLSCM(ADD_ONE_OFF_SOUND, pos.x, pos.y, pos.z, sound);
}

void Game::GiveMoney(int amount)
{
    g_pPlayersContainer[0].m_nMoney += amount;
}

void Game::SetMoney(int amount)
{
    g_pPlayersContainer[0].m_nMoney = amount;
}

void Game::ToggleEnterExits(bool enable)
{
    *EntryExitManagerDisabled = !enable;
    //int pEntries = *(int*)aml->GetSym(hGTASA, "_ZN17CEntryExitManager17mp_poolEntryExitsE");
    //int count = *(int*)(pEntries + 8);
    //for(int i = 0; i < count; ++i)
    //{
    //    *(uint16_t*)(pEntries + 0x30) = 0;
    //    pEntries += 0x3C;
    //}
}

void Game::EnableStuntBonus(bool enable)
{

}

void Game::ToggleThePassingOfTime(bool enable)
{
    if(enable)
    {
        aml->Write(pGTASA + 0x3E3378, (uintptr_t)"\xD0\xB5", 2);
    }
    else
    {
        aml->PlaceRET(pGTASA + 0x3E3378);
    }
}

void Game::ToggleClock(bool enable)
{
    clockFormat[0] = enable ? '%' : 0;
    ToggleThePassingOfTime(enable);
}

void Game::SetGravity(float gravity)
{
    *fGravity = gravity;
}

void Game::RemoveBuilding(int modelId, float x, float y, float z, float radius)
{
    if(modelId == 19300) return; // We cant delete technical models!

    BuildingToRemove* b = new BuildingToRemove;
    b->modelId = modelId;
    b->pos.x = x;
    b->pos.y = y;
    b->pos.z = z;
    b->radius = radius;
    g_RemoveBuilding[g_nRemovedBuildings] = b;
    ++g_nRemovedBuildings;

    CBuilding* foundEnt;
    int i;
    if(modelId == -1)
    {
        for(i = 0; i < POOL_BUILDINGS_COUNT; ++i)
        {
            foundEnt = (CBuilding*)((uintptr_t)pBuildingPool->objects + 60 * i);
            if(foundEnt && DistanceBetweenPoints(x, y, z, foundEnt->GetPosition()) <= radius)
            {
                //DestructBuildingPtr(foundEnt);
                foundEnt->m_nModelIndex = 19300;
            }
        }
        for(i = 0; i < POOL_DUMMIES_COUNT; ++i)
        {
            foundEnt = (CBuilding*)((uintptr_t)pDummyPool->objects + 60 * i);
            if(foundEnt && DistanceBetweenPoints(x, y, z, foundEnt->GetPosition()) <= radius)
            {
                //DestructDummyPtr(foundEnt);
                foundEnt->m_nModelIndex = 19300;
            }
        }
    }
    else
    {
        for(i = 0; i < POOL_BUILDINGS_COUNT; ++i)
        {
            foundEnt = (CBuilding*)((uintptr_t)pBuildingPool->objects + 60 * i);
            if(foundEnt && foundEnt->m_nModelIndex == modelId && DistanceBetweenPoints(x, y, z, foundEnt->GetPosition()) <= radius)
            {
                //DestructBuildingPtr(foundEnt);
                foundEnt->m_nModelIndex = 19300;
            }
        }
        for(i = 0; i < POOL_DUMMIES_COUNT; ++i)
        {
            foundEnt = (CBuilding*)((uintptr_t)pDummyPool->objects + 60 * i);
            if(foundEnt && foundEnt->m_nModelIndex == modelId && DistanceBetweenPoints(x, y, z, foundEnt->GetPosition()) <= radius)
            {
                //DestructDummyPtr(foundEnt);
                foundEnt->m_nModelIndex = 19300;
            }
        }
    }
}

bool Game::IsBuildingRemoved(uintptr_t buildingEntity)
{
    if(g_nRemovedBuildings > 0)
    {
        static int i;
        static BuildingToRemove* b;
        for(i = 0; i < g_nRemovedBuildings; ++i)
        {
            b = g_RemoveBuilding[i];
            if((b->modelId == -1 || b->modelId == *(short*)(buildingEntity + 38)) && *(int*)(buildingEntity + 20) != 0 && DistanceBetweenPoints(*(CVector*)(*(int*)(buildingEntity + 20) + 48), b->pos) <= b->radius)
            {
                //*(int*)(buildingEntity + 28) &= 0xFFFFFFFE;
                //samp->Message("Removed MODELID %d", *(short*)(buildingEntity + 38));
                return true;
            }
        }
    }
    return false;
}

bool Game::IsBuildingRemoved(int modelId, CVector pos)
{
    if(g_nRemovedBuildings > 0)
    {
        static int i;
        static BuildingToRemove* b;
        for(i = 0; i < g_nRemovedBuildings; ++i)
        {
            b = g_RemoveBuilding[i];
            if((b->modelId == -1 || b->modelId == modelId) && DistanceBetweenPoints(pos, b->pos) <= b->radius)
            {
                return true;
            }
        }
    }
    return false;
}

bool Game::IsBuildingRemoved(int modelId, float x, float y, float z)
{
    if(g_nRemovedBuildings > 0)
    {
        static int i;
        static BuildingToRemove* b;
        for(i = 0; i < g_nRemovedBuildings; ++i)
        {
            b = g_RemoveBuilding[i];
            if((b->modelId == -1 || b->modelId == modelId) && DistanceBetweenPoints(x, y, z, b->pos) <= b->radius)
            {
                return true;
            }
        }
    }
    return false;
}

uint16_t Game::GetSAMPVehID(CEntity* v)
{
    int maxvehicles = Game::m_pVehiclePool->GetHighestSlotUsedEver();
    for(uint16_t i = 0; i < maxvehicles; ++i)
    {
        if(Game::m_pVehiclePool->IsSlotOccupied(i) && Game::m_pVehiclePool->GetAt(i)->m_pEntity == (CVehicle*)v) return i;
    }
    return 0xFFFF;
}

uint16_t Game::GetSAMPPlayerID(CEntity* p)
{
    int maxplayers = Game::m_pPlayerPool->GetHighestSlotUsedEver();
    for(uint16_t i = 0; i < maxplayers; ++i)
    {
        if(Game::m_pPlayerPool->IsSlotOccupied(i) && Game::m_pPlayerPool->GetAt(i)->m_pEntity == p) return i;
    }
    return 0xFFFF;
}

void Game::ToggleCJWalk(bool enable)
{
    if(enable)
    {
        aml->Write(pGTASA + 0x4C5EFA, (uintptr_t)"\xCA\xF8\xE0\x04", 4);
    }
    else
    {
        aml->PlaceNOP(pGTASA + 0x4C5EFA + 0x1, 2);
    }
}

uint32_t Game::CreateRadarMarkerIcon(int iMarkerType, float fX, float fY, float fZ, int iColor, int iStyle)
{
    uint32_t dwMarkerID = 0;

    if(iStyle == 1) 
        CALLSCM(ADD_SPRITE_BLIP_FOR_COORD, fX, fY, fZ, iMarkerType, &dwMarkerID);
    else if(iStyle == 2) 
        CALLSCM(ADD_SHORT_RANGE_SPRITE_BLIP_FOR_CONTACT_POINT, fX, fY, fZ, iMarkerType, &dwMarkerID);
    else if(iStyle == 3) 
        CALLSCM(ADD_SPRITE_BLIP_FOR_CONTACT_POINT, fX, fY, fZ, iMarkerType, &dwMarkerID);
    else 
        CALLSCM(ADD_SHORT_RANGE_SPRITE_BLIP_FOR_COORD, fX, fY, fZ, iMarkerType, &dwMarkerID);

    if(iMarkerType == 0)
    {
        if(iColor >= 1004)
        {
            CALLSCM(CHANGE_BLIP_COLOUR, dwMarkerID, iColor);
            CALLSCM(CHANGE_BLIP_SCALE, dwMarkerID, 3);
        }
        else
        {
            CALLSCM(CHANGE_BLIP_COLOUR, dwMarkerID, iColor);
            CALLSCM(CHANGE_BLIP_SCALE, dwMarkerID, 2);
        }
    }
    return dwMarkerID;
}
