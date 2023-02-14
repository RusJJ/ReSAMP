#ifndef __SAGAME_H
#define __SAGAME_H

#include <time.h>

#include <third/jpool.h>
#include <pools/remoteplayer.h>
#include <pools/remotevehicle.h>
#include <pools/remoteobject.h>
#include <pools/remotemapicon.h>

#define MAX_BUILDINGS_TO_REMOVE  1000

struct BuildingToRemove
{
    int modelId;
    CVector pos;
    float radius;
};
extern int g_nRemovedBuildings;
extern BuildingToRemove* g_RemoveBuilding[MAX_BUILDINGS_TO_REMOVE];

inline float Twice(float f)
{
    return f * f;
}
inline float DistanceBetweenPoints(CVector2D& a1, CVector2D& a2)
{
    return sqrtf(Twice(a1.x - a2.x) + Twice(a1.y - a2.y));
}
inline float DistanceBetweenPoints(float x1, float y1, CVector2D& a2)
{
    return sqrtf(Twice(x1 - a2.x) + Twice(y1 - a2.y));
}
inline float DistanceBetweenPoints(CVector2D& a1, float x2, float y2)
{
    return sqrtf(Twice(a1.x - x2) + Twice(a1.y - y2));
}
inline float DistanceBetweenPoints(float x1, float y1, float x2, float y2)
{
    return sqrtf(Twice(x1 - x2) + Twice(y1 - y2));
}
inline float DistanceBetweenPoints(CVector& a1, CVector& a2)
{
    return sqrtf(Twice(a1.x - a2.x) + Twice(a1.y - a2.y) + Twice(a1.z - a2.z));
}
inline float DistanceBetweenPoints(float x1, float y1, float z1, CVector& a2)
{
    return sqrtf(Twice(x1 - a2.x) + Twice(y1 - a2.y) + Twice(z1 - a2.z));
}
inline float DistanceBetweenPoints(CVector& a1, float x2, float y2, float z2)
{
    return sqrtf(Twice(a1.x - x2) + Twice(a1.y - y2) + Twice(a1.z - z2));
}
inline float DistanceBetweenPoints(float x1, float y1, float z1, float x2, float y2, float z2)
{
    return sqrtf(Twice(x1 - x2) + Twice(y1 - y2) + Twice(z1 - z2));
}

class Game
{
public:
    static void InitializeGameClass();

    static CRemotePlayer* CreatePlayer(int id, int skin, float x, float y, float z, float rot, bool createMarker);
    static CRemoteVehicle* CreateVehicle(int id, int type, float x, float y, float z, float rot);
    static CRemoteVehicle* CreateVehicle(VehicleData& data);

    static bool IsModelLoaded(int modelId);
    static void RequestModel(int modelId);
    static void RequestModelNow(int modelId);
    static void LoadRequestedModels();
    static void RefreshStreamingAt(CVector2D posxy);

    static CPlayerPed* GetPlayerByGtaID(int gtaId);
    static CVehicle* GetVehicleByGtaID(int gtaId);

    static void ShowBigMsg(const char* msg, int time, int type);
    static void ClearWeapons();
    static void GiveWeapon(int weapId, int ammo, bool armed);
    static void ShowWidgets(bool enabled);
    static void SetWorldTime(char hour, char minute);
    static void SetWeather(short weather);
    static void EnableZoneNames(bool enable);
    static void PlaySound(int sound, float x, float y, float z);
    static void PlaySound(int sound, CVector pos);
    static void GiveMoney(int amount);
    static void SetMoney(int amount);
    static void ToggleEnterExits(bool enable);
    static void EnableStuntBonus(bool enable);
    static void ToggleThePassingOfTime(bool enable);
    static void ToggleClock(bool enable);
    static void SetGravity(float gravity);
    static void RemoveBuilding(int modelId, float x, float y, float z, float radius);
    static bool IsBuildingRemoved(uintptr_t buildingEntity);
    static bool IsBuildingRemoved(int modelId, CVector pos);
    static bool IsBuildingRemoved(int modelId, float x, float y, float z);
    static uint16_t GetSAMPVehID(CVehicle* v);
    static void ToggleCJWalk(bool enable);
    static uint32_t CreateRadarMarkerIcon(int iMarkerType, float fX, float fY, float fZ, int iColor, int iStyle);

    static inline uint32_t GetTick()
    {
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        return (1000 * tv.tv_sec + 0.001f * tv.tv_usec);
    }

public:
    static uint16_t*                         m_pLatestBigMessage;
    static int                               m_nProcessedInPlayerInfo;
    static bool                              m_bDisableWidgets;
    static JPoolCalcHighest<CRemotePlayer>*  m_pPlayerPool;
    static JPoolCalcHighest<CRemoteVehicle>* m_pVehiclePool;
    static JPoolCalcHighest<CRemoteObject>*  m_pObjectPool;
    static JPoolCalcHighest<CRemoteMapIcon>* m_pMapIconPool;
};

#endif // __SAGAME_H
