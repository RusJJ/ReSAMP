#ifndef __REMOTEPLAYER_H
#define __REMOTEPLAYER_H

#include <samp.h>
#include <sampnet/samp_syncdata.h>

#define MAX_SKILL_WEAPONS 11

#define PLAYER_STATE_NONE						0
#define PLAYER_STATE_EXIT_VEHICLE				4
#define PLAYER_STATE_ENTER_VEHICLE_DRIVER		5
#define PLAYER_STATE_ENTER_VEHICLE_PASSENGER	6
#define PLAYER_STATE_SPECTATING					9
#define PLAYER_STATE_ONFOOT						17
#define PLAYER_STATE_PASSENGER					18
#define PLAYER_STATE_DRIVER						19
#define PLAYER_STATE_WASTED						32
#define PLAYER_STATE_SPAWNED					33

class CRemotePlayer
{
public:
    CRemotePlayer();
    ~CRemotePlayer();
    
    inline CPlayerPed* GetEntity() { return m_pEntity; }

    bool Spawn();
    void Update();
    bool IsActive();
    void KillBlip();
    void SetKeys(uint16_t wKeys, uint16_t lrAnalog, uint16_t udAnalog);
    void SetModelIndex(int mdlIdx);
    void SetWeaponSkill(int skill, uint16_t lvl);
    inline void SetName(const char* newname) { snprintf(m_szName, sizeof(m_szName), "%s", newname); }

public:
    bool m_bIsLocal;
    unsigned char m_byteState;
    char m_szName[MAX_PLAYER_NAME+1];

    int m_nID;
    int m_nGtaID;
    uint32_t m_nMarkerID;
    CVector m_vecMarkerPos;
    CPlayerPed* m_pEntity;
    bool m_bOnfootDataChanged;
    bool m_bIncarDataChanged;
    uint16_t m_nWeaponSkills[MAX_SKILL_WEAPONS];

    ONFOOT_SYNC_DATA    m_ofSync;
    INCAR_SYNC_DATA     m_icSync;
    PASSENGER_SYNC_DATA m_psSync;
    AIM_SYNC_DATA       m_aimSync;
    TRAILER_SYNC_DATA   m_trSync;
};

#endif // __REMOTEPLAYER_H
