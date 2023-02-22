#include <mod/amlmod.h>
#include <mod/logger.h>
#include <samp.h>
#include <gtasa.h>

#include "patch/pad.h"
#include "remoteplayer.h"
#include <game/game.h>
#include <game/localplayer.h>
#include <game/scripting.h>

CRemotePlayer::CRemotePlayer()
{
    m_bIsLocal = false;
    m_byteState = PLAYER_STATE_NONE;
    m_nID = -1;
    m_nGtaID = -1;
    m_nMarkerID = -1;
    m_pEntity = NULL;
    m_bOnfootDataChanged = false;
    memset(m_szName, sizeof(m_szName), 0);
    
    for(uint8_t i = 0; i < MAX_SKILL_WEAPONS; ++i)
    {
        SetWeaponSkill(i, 999);
    }
}

void CRemotePlayer::Update()
{
    if(m_bIsLocal)
    {
        CLocalPlayer::Update();
        return;
    }
    
    KillBlip();
    m_nMarkerID = Game::CreateRadarMarkerIcon(0, m_vecMarkerPos.x, m_vecMarkerPos.y, m_vecMarkerPos.z, m_nID, 0);
    
    if(!m_pEntity) return;
    
    //m_pEntity->m_fHealth = (float)m_ofSync.byteHealth;
    switch(m_byteState)
    {
        case PLAYER_STATE_ONFOOT:
        {
            if(m_pEntity->m_pVehicle && m_bOnfootDataChanged)
            {
                // Force leave a car
            }
            
            SetKeys(m_ofSync.wKeys, m_ofSync.lrAnalog, m_ofSync.udAnalog);
            m_pEntity->m_vecMoveSpeed = CVector(0);
        
            if(m_bOnfootDataChanged)
            {
                m_ofSync.quat.GetMatrix(m_pEntity->m_matrix);
                m_pEntity->Teleport(m_ofSync.vecPos);
                //m_pEntity->GetPosition() = m_ofSync.vecPos;
                m_pEntity->m_vecMoveSpeed = m_ofSync.vecMoveSpeed;
                Game::UpdateGameMatrix(m_pEntity->m_matrix); // Need it
            
                m_bOnfootDataChanged = false;
            }
            break;
        }
        case PLAYER_STATE_DRIVER:
        {
            // That means we're IN the car
            // Because we got syncing data
            if(!m_pEntity->m_pVehicle && m_bIncarDataChanged)
            {
                // Force enter a car
            }
            
            if(m_pEntity->m_pVehicle)
            {
                
            }
            break;
        }
        
        default: // Broken? Or unimplemented?
            break;
    }
}

bool CRemotePlayer::IsActive() // TODO: Check thiz
{
    return (!m_bIsLocal && m_byteState != PLAYER_STATE_NONE);
}

void CRemotePlayer::KillBlip()
{
    if(m_nMarkerID >= 0)
    {
        CALLSCM(REMOVE_BLIP, m_nMarkerID);
        m_nMarkerID = 0;
    }
}

void CRemotePlayer::SetKeys(uint16_t wKeys, uint16_t lrAnalog, uint16_t udAnalog)
{
    if(m_bIsLocal) return;
    
    PAD_KEYS *pad = &RemotePlayerKeys[m_nID];

    // LEFT/RIGHT
    pad->wKeyLR = lrAnalog;
    // UP/DOWN
    pad->wKeyUD = udAnalog;

    // KEY_ACTION
    pad->bKeys[ePadKeys::KEY_ACTION] = (wKeys & 1);
    wKeys >>= 1;
    // KEY_CROUCH
    pad->bKeys[ePadKeys::KEY_CROUCH] = (wKeys & 1);
    wKeys >>= 1;
    // KEY_FIRE
    pad->bKeys[ePadKeys::KEY_FIRE] = (wKeys & 1);
    wKeys >>= 1;
    // KEY_SPRINT
    pad->bKeys[ePadKeys::KEY_SPRINT] = (wKeys & 1);
    wKeys >>= 1;
    // KEY_SECONDARY_ATTACK
    pad->bKeys[ePadKeys::KEY_SECONDARY_ATTACK] = (wKeys & 1);
    wKeys >>= 1;
    // KEY_JUMP
    pad->bKeys[ePadKeys::KEY_JUMP] = (wKeys & 1);
    if(!pad->bKeys[ePadKeys::KEY_JUMP]) pad->bIgnoreJump = false;
    wKeys >>= 1;
    // KEY_LOOK_RIGHT
    pad->bKeys[ePadKeys::KEY_LOOK_RIGHT] = (wKeys & 1);
    wKeys >>= 1;
    // KEY_HANDBRAKE
    pad->bKeys[ePadKeys::KEY_HANDBRAKE] = (wKeys & 1);
    wKeys >>= 1;
    // KEY_LOOK_LEFT
    pad->bKeys[ePadKeys::KEY_LOOK_LEFT] = (wKeys & 1);
    wKeys >>= 1;
    // KEY_SUBMISSION
    pad->bKeys[ePadKeys::KEY_SUBMISSION] = (wKeys & 1);
    wKeys >>= 1;
    // KEY_WALK
    pad->bKeys[ePadKeys::KEY_WALK] = (wKeys & 1);
    wKeys >>= 1;
    // KEY_ANALOG_UP
    pad->bKeys[ePadKeys::KEY_ANALOG_UP] = (wKeys & 1);
    wKeys >>= 1;
    // KEY_ANALOG_DOWN
    pad->bKeys[ePadKeys::KEY_ANALOG_DOWN] = (wKeys & 1);
    wKeys >>= 1;
    // KEY_ANALOG_LEFT
    pad->bKeys[ePadKeys::KEY_ANALOG_LEFT] = (wKeys & 1);
    wKeys >>= 1;
    // KEY_ANALOG_RIGHT
    pad->bKeys[ePadKeys::KEY_ANALOG_RIGHT] = (wKeys & 1);
}

void CRemotePlayer::SetModelIndex(int mdlIdx)
{
    if(mdlIdx < 0 || mdlIdx > 315) return;
    Game::RequestModelNow(mdlIdx);
    m_pEntity->SetModelIndex(mdlIdx);
}

void CRemotePlayer::SetWeaponSkill(int skill, uint16_t lvl)
{
    if(skill < 0 || skill >= MAX_SKILL_WEAPONS) return;
    
    if(lvl > 999) lvl = 999;
    else if(lvl < 0) lvl = 0;
    
    m_nWeaponSkills[skill] = lvl;
    if(m_bIsLocal) Game::GetFloatStat(skill + WEAPONTYPE_PISTOL_SKILL) = (float)lvl;
}
