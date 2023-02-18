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
}

void CRemotePlayer::Update()
{
    if(m_bIsLocal)
    {
        CLocalPlayer::Update();
        return;
    }
    
    if(m_nMarkerID >= 0)
    {
        CALLSCM(REMOVE_BLIP, m_nMarkerID);
    }

    m_nMarkerID = Game::CreateRadarMarkerIcon(0, m_vecMarkerPos.x, m_vecMarkerPos.y, m_vecMarkerPos.z, m_nID, 0);
    
    if(!m_pEntity) return;
    
    //m_pEntity->m_fHealth = (float)m_ofSync.byteHealth;
    if(m_byteState == PLAYER_STATE_ONFOOT)
    {
        SetKeys(m_ofSync.wKeys, m_ofSync.lrAnalog, m_ofSync.udAnalog);
        m_pEntity->m_vecMoveSpeed = CVector(0);
        
        if(m_bOnfootDataChanged)
        {
            Game::AddEntityToWorld(m_pEntity, true); // Start changes
            m_ofSync.quat.GetMatrix(m_pEntity->m_matrix);
            m_pEntity->GetPosition() = m_ofSync.vecPos;
            m_pEntity->m_vecMoveSpeed = m_ofSync.vecMoveSpeed;
            Game::AddEntityToWorld(m_pEntity, false); // End changes!
            
            m_bOnfootDataChanged = false;
        }
    }
}

bool CRemotePlayer::IsActive() // TODO: Check thiz
{
    return (!m_bIsLocal && m_byteState != PLAYER_STATE_NONE);
}

void CRemotePlayer::SetKeys(uint16_t wKeys, uint16_t lrAnalog, uint16_t udAnalog)
{
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
