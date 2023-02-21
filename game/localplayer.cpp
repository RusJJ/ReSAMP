#include <mod/amlmod.h>
#include <mod/logger.h>
#include <samp.h>
#include <gtasa.h>

#include "game.h"
#include "scripting.h"
#include "localplayer.h"
#include "patch/pad.h"

bool CLocalPlayer::m_bDisableControls = false;
bool CLocalPlayer::m_bWaitingToSpawn = false;
bool CLocalPlayer::m_bWasInCar = false;
uint16_t CLocalPlayer::m_nLastSAMPVehId = 0xFFFF;
uint8_t CLocalPlayer::m_byteLastInteriorId = 0;
bool CLocalPlayer::m_bWasted = false;
bool CLocalPlayer::m_bClassChangeRequested = false;

short CLocalPlayer::m_nChosenClassId = 0;
CRemotePlayer* CLocalPlayer::m_pRemote = NULL;
int CLocalPlayer::m_nID = -1;
int CLocalPlayer::m_nGtaID = -1;
CPlayerPed* CLocalPlayer::m_pEntity = NULL;
uint32_t CLocalPlayer::m_nTick = 0;
uint32_t CLocalPlayer::m_nNextTick = 0;
uint32_t CLocalPlayer::m_nNextTick_SendOnFoot = 0;
uint32_t CLocalPlayer::m_nNextTick_SendInCar = 0;

PLAYER_SPAWN_INFO CLocalPlayer::m_SpawnInfo = {0};
ONFOOT_SYNC_DATA CLocalPlayer::m_OnFootData = {0};
INCAR_SYNC_DATA CLocalPlayer::m_InCarData = {0};
PASSENGER_SYNC_DATA CLocalPlayer::m_PassengerData = {0};
//TRAILER_SYNC_DATA CLocalPlayer::m_TrailerData = {0};
AIM_SYNC_DATA CLocalPlayer::m_aimSync = {0};

void CLocalPlayer::GetMe()
{
    CALLSCM(GET_PLAYER_CHAR, 0, &CLocalPlayer::m_nGtaID);
    m_pEntity = Game::GetPlayerByGtaID(CLocalPlayer::m_nGtaID);
}

void CLocalPlayer::PutMeInPool(int id)
{
    m_nID = id;
    m_pEntity = Game::GetPlayerByGtaID(m_nGtaID);
    m_pRemote = Game::m_pPlayerPool->AllocAt(id, true);
    m_pRemote->m_pEntity = m_pEntity;
    m_pRemote->m_nGtaID = m_nGtaID;
    m_pRemote->m_nID = m_nID;
    m_pRemote->m_byteState = PLAYER_STATE_SPAWNED;
    m_pRemote->m_bIsLocal = true; // Did i just really forgot that?
}

void CLocalPlayer::ForceSpawnSelection()
{
    
    
    RequestClass();
}

void CLocalPlayer::Update()
{
    m_nTick = Game::GetTick();
    
    if(m_byteLastInteriorId != m_pEntity->m_nInterior)
    {
        m_byteLastInteriorId = m_pEntity->m_nInterior;
        
        RakNet::BitStream bsSend;
        bsSend.Write(m_byteLastInteriorId);
        samp->GetRakClient()->RPC((int*)&RPC_SetInteriorId, &bsSend, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0,false, UNASSIGNED_NETWORK_ID, nullptr);
    }
    
    if(!m_bWasted)
    {
        if(m_pEntity->IsDead() || m_pEntity->ActionWasted())
        {
            m_bWasted = true;
            
            uint8_t byteWeaponKilled = m_pEntity->LastDamagedWeaponType;
            uint16_t killerId = GetMyKillerID();
            
            RakNet::BitStream bsSend;
            bsSend.Write(byteWeaponKilled);
            bsSend.Write(killerId);
            samp->GetRakClient()->RPC((int*)&RPC_Death, &bsSend, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0,false, UNASSIGNED_NETWORK_ID, nullptr);
        }
    }
    else
    {
        if(m_pEntity->IsAlive() && m_pEntity->m_fHealth > 0)
        {
            if(m_bClassChangeRequested)
            {
                m_bClassChangeRequested = false;
                RequestClass();
            }
            else
            {
                CLocalPlayer::Spawn();
            }
            m_bWasted = false;
        }
    }

    if(m_pEntity->IsInAnyVehicle())
    {
        m_nLastSAMPVehId = Game::GetSAMPVehID(m_pEntity->m_pVehicle);
        
        if(m_pEntity->IsPassenger())
        {
            
            if(!m_bWasInCar)
            {
                m_bWasInCar = true;
                // Notify enter
                RakNet::BitStream bsSend;
                bsSend.Write(m_nLastSAMPVehId);
                bsSend.Write((uint8_t)true); // is passenger
	            samp->GetRakClient()->RPC((int*)&RPC_EnterVehicle, &bsSend, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0,false, UNASSIGNED_NETWORK_ID, nullptr);
            }
        }
        else
        {
            SendSyncData_InCar();
            if(!m_bWasInCar)
            {
                m_bWasInCar = true;
                // Notify enter (done in hooks, but if we're cheating, this will kick us)
                if(m_nLastSAMPVehId == 0xFFFF)
                {
                    RakNet::BitStream bsSend;
                    bsSend.Write(m_nLastSAMPVehId);
                    bsSend.Write((uint8_t)false); // is passenger
	                samp->GetRakClient()->RPC((int*)&RPC_EnterVehicle, &bsSend, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0,false, UNASSIGNED_NETWORK_ID, nullptr);
                }
            }
        }
    }
    else
    {
        if(m_bWasInCar)
        {
            m_bWasInCar = false;
        }
        SendSyncData_OnFoot();
    }
}

void CLocalPlayer::Spawn()
{
    logger->Info("Spawn");
    
    Game::ShowWidgets(true);
    m_pEntity = Game::GetPlayerByGtaID(m_nGtaID);
    m_pEntity->GetPosition() = m_SpawnInfo.vecPos;
    CALLSCM(ADD_HOSPITAL_RESTART, m_SpawnInfo.vecPos.x, m_SpawnInfo.vecPos.y, m_SpawnInfo.vecPos.z, m_SpawnInfo.fRotation, 0);
    CALLSCM(RESTORE_CAMERA_JUMPCUT);
    m_bDisableControls = false;
    m_bWasted = false;
    
    // Hey server, we're spawned!
    RakNet::BitStream bsSpawnRequest;
    samp->GetRakClient()->RPC((int*)&RPC_Spawn, &bsSpawnRequest, HIGH_PRIORITY, RELIABLE, 0, false, UNASSIGNED_NETWORK_ID, 0);
}

void CLocalPlayer::SetModelIndex(int mdlIdx)
{
    Game::RequestModelNow(mdlIdx);
    m_pEntity->SetModelIndex(mdlIdx);
}

void CLocalPlayer::OnConnected()
{
    logger->Info("Connect");
    
    CLocalPlayer::RequestClass();
}

void CLocalPlayer::ChangeClass(bool next)
{
    if(next)
    {
        ++m_nChosenClassId;
        if(m_nChosenClassId >= samp->GetServerVars().m_iSpawnsAvailable)
        {
            m_nChosenClassId = 0;
        }
        Game::PlaySound(1052, m_pEntity->GetPosition());
    }
    else
    {
        --m_nChosenClassId;
        if(m_nChosenClassId < 0)
        {
            m_nChosenClassId = samp->GetServerVars().m_iSpawnsAvailable - 1;
        }
        Game::PlaySound(1053, m_pEntity->GetPosition());
    }
}

void CLocalPlayer::RequestClass(short num, bool overwrite)
{
    RakNet::BitStream bsClassRequest;
    if(overwrite)
    {
        if(num >= 0) m_nChosenClassId = num;
        bsClassRequest.Write((int)m_nChosenClassId);
    }
    else
    {
        bsClassRequest.Write((int)(num < 0 ? m_nChosenClassId : num));
    }
	samp->GetRakClient()->RPC((int*)&RPC_RequestClass, &bsClassRequest, HIGH_PRIORITY, RELIABLE, 0, false, UNASSIGNED_NETWORK_ID, 0);
}

void CLocalPlayer::RequestSpawn()
{
    RakNet::BitStream bsSpawnRequest;
    samp->GetRakClient()->RPC((int*)&RPC_RequestSpawn, &bsSpawnRequest, HIGH_PRIORITY, RELIABLE, 0, false, UNASSIGNED_NETWORK_ID, 0);
}

void CLocalPlayer::SetInterior(uint8_t id)
{
    m_pEntity->m_nInterior = id;
    CALLSCM(SET_AREA_VISIBLE, id);
    Game::RefreshStreamingAt(CLocalPlayer::GetEntity()->GetPosition().m_vec2D);
}

uint16_t CLocalPlayer::GetMyKillerID()
{
    if(!m_pEntity->m_pDamageEntity) return 0xFFFF;
    
    if(m_pEntity->LastDamagedWeaponType == WEAPON_DROWNING) return WEAPON_DROWNING;
    
    if(m_pEntity->m_pDamageEntity->m_nType == ENTITY_TYPE_PED)
    {
        uint16_t pid = Game::GetSAMPPlayerID(m_pEntity->m_pDamageEntity);
    }
    else if(m_pEntity->m_pDamageEntity->m_nType == ENTITY_TYPE_VEHICLE)
    {
        uint16_t vid = Game::GetSAMPVehID(m_pEntity->m_pDamageEntity);
    }
    
    
    
    return 0xFFFF;
}

void CLocalPlayer::SendSyncData_OnFoot()
{
    if(m_nTick > m_nNextTick_SendOnFoot)
    {
        m_nNextTick_SendOnFoot = m_nTick + samp->GetServerVars().m_iNetModeNormalOnfootSendRate;

        ONFOOT_SYNC_DATA ofSync;
        
        CMatrix* mat = m_pEntity->GetMatrix();
        CVector& speed = m_pEntity->GetSpeedVector();
        uint8_t keys;
        ofSync.wKeys = GetKeys(&ofSync.lrAnalog, &ofSync.udAnalog, &keys);
        ofSync.vecPos.x = mat->pos.x;
        ofSync.vecPos.y = mat->pos.y;
        ofSync.vecPos.z = mat->pos.z;

        ofSync.quat.SetFromMatrix(mat);
        ofSync.quat.Normalize();

        ofSync.byteHealth = (uint8_t)m_pEntity->m_fHealth;
        ofSync.byteArmour = (uint8_t)m_pEntity->m_fArmour;
        ofSync.byteCurrentKeyAndWeapon = (m_pEntity->LastDamagedWeaponType | keys << 6);
        ofSync.byteSpecialAction = 0; // GetSpecialAction();

        ofSync.vecMoveSpeed.x = speed.x;
        ofSync.vecMoveSpeed.y = speed.y;
        ofSync.vecMoveSpeed.z = speed.z;

        ofSync.vecSurfOffsets.x = 0.0f;
        ofSync.vecSurfOffsets.y = 0.0f;
        ofSync.vecSurfOffsets.z = 0.0f;
        ofSync.wSurfInfo = 0;

        ofSync.dwAnimation = 0;

        if(memcmp(&m_OnFootData, &ofSync, sizeof(ONFOOT_SYNC_DATA)))
        {
            RakNet::BitStream bsPlayerSync;
            bsPlayerSync.Write((uint8_t)ID_PLAYER_SYNC);
            bsPlayerSync.Write((char*)&ofSync, sizeof(ONFOOT_SYNC_DATA));
            samp->GetRakClient()->Send(&bsPlayerSync, HIGH_PRIORITY, UNRELIABLE_SEQUENCED, 0);
            memcpy(&m_OnFootData, &ofSync, sizeof(ONFOOT_SYNC_DATA));
        }
    }
}

void CLocalPlayer::SendSyncData_InCar()
{
    if(m_nTick > m_nNextTick_SendInCar)
    {
        m_nNextTick_SendInCar = m_nTick + samp->GetServerVars().m_iNetModeNormalInCarSendRate;

        CVehicle* pVeh = CLocalPlayer::GetEntity()->m_pVehicle;
        INCAR_SYNC_DATA icSync;
        
        CMatrix* mat = pVeh->GetMatrix();
        CVector& speed = pVeh->GetSpeedVector();
        uint8_t keys;
        icSync.wKeys = GetKeys(&icSync.lrAnalog, &icSync.udAnalog, &keys);
        icSync.vecPos.x = mat->pos.x;
        icSync.vecPos.y = mat->pos.y;
        icSync.vecPos.z = mat->pos.z;

        icSync.quat.SetFromMatrix(mat);
        icSync.quat.Normalize();
        icSync.VehicleID = m_nLastSAMPVehId;
        
        icSync.fCarHealth = pVeh->m_fHealth;
        icSync.bytePlayerHealth = (uint8_t)CLocalPlayer::GetEntity()->m_fHealth;
		icSync.bytePlayerArmour = (uint8_t)CLocalPlayer::GetEntity()->m_fArmour;

        if(memcmp(&m_InCarData, &icSync, sizeof(INCAR_SYNC_DATA)))
        {
            RakNet::BitStream bsPlayerSync;
            bsPlayerSync.Write((uint8_t)ID_VEHICLE_SYNC);
            bsPlayerSync.Write((char*)&icSync, sizeof(INCAR_SYNC_DATA));
            samp->GetRakClient()->Send(&bsPlayerSync, HIGH_PRIORITY, UNRELIABLE_SEQUENCED, 0);
            memcpy(&m_InCarData, &icSync, sizeof(INCAR_SYNC_DATA));
        }
    }
}

uint16_t CLocalPlayer::GetKeys(uint16_t* lrAnalog, uint16_t* udAnalog, uint8_t* keys)
{
    *lrAnalog = LocalPlayerKeys.wKeyLR;
    *udAnalog = LocalPlayerKeys.wKeyUD;
	
    if(keys != NULL)
    {
        *keys = 0;

        // KEY_YES
        if(LocalPlayerKeys.bKeys[ePadKeys::KEY_YES]) *keys = 1;

        // KEY_NO
        if(LocalPlayerKeys.bKeys[ePadKeys::KEY_NO]) *keys = 2;

        // KEY_CTRL_BACK
        if(LocalPlayerKeys.bKeys[ePadKeys::KEY_CTRL_BACK]) *keys = 3;
    }

    uint16_t wRet = 0;
    // KEY_ANALOG_RIGHT
    if(LocalPlayerKeys.bKeys[ePadKeys::KEY_ANALOG_RIGHT]) wRet |= 1;
    wRet <<= 1;
    // KEY_ANALOG_LEFT
    if(LocalPlayerKeys.bKeys[ePadKeys::KEY_ANALOG_LEFT]) wRet |= 1;
    wRet <<= 1;
    // KEY_ANALOG_DOWN
    if(LocalPlayerKeys.bKeys[ePadKeys::KEY_ANALOG_DOWN]) wRet |= 1;
    wRet <<= 1;
    // KEY_ANALOG_UP
    if(LocalPlayerKeys.bKeys[ePadKeys::KEY_ANALOG_UP]) wRet |= 1;
    wRet <<= 1;
    // KEY_WALK
    if(LocalPlayerKeys.bKeys[ePadKeys::KEY_WALK]) wRet |= 1;
    wRet <<= 1;
    // KEY_SUBMISSION
    if(LocalPlayerKeys.bKeys[ePadKeys::KEY_SUBMISSION]) wRet |= 1;
    wRet <<= 1;
    // KEY_WALK
    if(LocalPlayerKeys.bKeys[ePadKeys::KEY_WALK]) wRet |= 1;
    wRet <<= 1;
    // KEY_SUBMISSION
    if(LocalPlayerKeys.bKeys[ePadKeys::KEY_SUBMISSION]) wRet |= 1;
    wRet <<= 1;
    // KEY_LOOK_LEFT
    if(LocalPlayerKeys.bKeys[ePadKeys::KEY_LOOK_LEFT]) wRet |= 1;
    wRet <<= 1;
    // KEY_HANDBRAKE
    if(LocalPlayerKeys.bKeys[ePadKeys::KEY_HANDBRAKE]) wRet |= 1;
    wRet <<= 1;
    // KEY_LOOK_RIGHT
    if(LocalPlayerKeys.bKeys[ePadKeys::KEY_LOOK_RIGHT]) wRet |= 1;
    wRet <<= 1;
    // KEY_JUMP
    if(LocalPlayerKeys.bKeys[ePadKeys::KEY_JUMP]) wRet |= 1;
    wRet <<= 1;
    // KEY_SECONDARY_ATTACK
    if(LocalPlayerKeys.bKeys[ePadKeys::KEY_SECONDARY_ATTACK]) wRet |= 1;
    wRet <<= 1;
    // KEY_SPRINT
    if(LocalPlayerKeys.bKeys[ePadKeys::KEY_SPRINT]) wRet |= 1;
    wRet <<= 1;
    // KEY_FIRE
    if(LocalPlayerKeys.bKeys[ePadKeys::KEY_FIRE]) wRet |= 1;
    wRet <<= 1;
    // KEY_CROUCH
    if(LocalPlayerKeys.bKeys[ePadKeys::KEY_CROUCH]) wRet |= 1;
    wRet <<= 1;
    // KEY_ACTION
    if(LocalPlayerKeys.bKeys[ePadKeys::KEY_ACTION]) wRet |= 1;

    memset(LocalPlayerKeys.bKeys, 0, ePadKeys::SIZE);

    return wRet;
}
