#ifndef __PLAYER_H
#define __PLAYER_H

#include <mod/amlmod.h>
#include <samp.h>
#include <gtasa.h>
#include <sampnet/samp_syncdata.h>

class CRemotePlayer;

class CLocalPlayer
{
public:
    static void                 GetMe();
    static void                 PutMeInPool(int id);
    static void                 Update();
    static void                 Spawn();
    static void                 SetModelIndex(int mdlIdx);
    static void                 OnConnected();
    static void                 ChangeClass(bool next);
    static void                 RequestClass(short num = -1, bool overwrite = false);
    static void                 RequestSpawn();
    static void                 SetInterior(uint8_t id);
    static uint16_t             GetMyKillerID();

    static void                 SendSyncData_OnFoot();
    static void                 SendSyncData_InCar();

    static uint16_t             GetKeys(uint16_t* lrAnalog, uint16_t* udAnalog, uint8_t* keys);

    static inline CPlayerPed*   GetEntity() { return m_pEntity; }
    static inline int           GetID() { return m_nID; }
    static inline int           GetGtaID() { return m_nGtaID; }

public:
    static bool                 m_bDisableControls;
    static bool                 m_bWaitingToSpawn;
    static bool                 m_bWasInCar;
    static uint16_t             m_nLastSAMPVehId;
    static uint8_t              m_byteLastInteriorId;
    static bool                 m_bWasted;
    static bool                 m_bClassChangeRequested;
    
private:
    static CRemotePlayer*       m_pRemote;
    static short                m_nChosenClassId;
    static int                  m_nID;
    static int                  m_nGtaID;
    static CPlayerPed*          m_pEntity;
    static uint32_t             m_nTick;
    static uint32_t             m_nNextTick;
    static uint32_t             m_nNextTick_SendOnFoot;
    static uint32_t             m_nNextTick_SendInCar;

public:
    static PLAYER_SPAWN_INFO    m_SpawnInfo;
    static ONFOOT_SYNC_DATA     m_OnFootData;
    static INCAR_SYNC_DATA      m_InCarData;
    static PASSENGER_SYNC_DATA  m_PassengerData;
    static TRAILER_SYNC_DATA    m_TrailerData;
    static AIM_SYNC_DATA        m_aimSync;
};

#endif // __PLAYER_H
