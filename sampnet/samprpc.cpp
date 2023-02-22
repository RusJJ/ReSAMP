#include <mod/amlmod.h>
#include <mod/logger.h>
#include <samp.h>

#include "samprpc.h"
#include "sampnet.h"
#include <game/game.h>
#include <game/scripting.h>
#include <game/localplayer.h>
#include <ui/spawnscreen.h>
#include <ui/dialogbox.h>
#include <pools/remoteplayer.h>

#define RAKDATA(_PARAMS) int dataLen = ((int)(_PARAMS->numberOfBitsOfData) / 8) + 1; RakNet::BitStream bsData((unsigned char*)(_PARAMS->input), dataLen, false)
#define RAKDATA_DBG(_NAME) char* out = new char[dataLen+1] {0}; bsData.Read(out, dataLen); for(int i = 0; i < dataLen; ++i) logger->Info(#_NAME"[%d]=0x%02X", i, out[i]);


ONRPC(SetPlayerName)
{
    RAKDATA(params);
    
    uint16_t playerId;
    uint8_t byteLen;
    char szName[MAX_PLAYER_NAME];
    
    bsData.Read(playerId);
    CRemotePlayer* p = Game::m_pPlayerPool->GetAt(playerId);
    if(!p) return;
    
    bsData.Read(byteLen);
    if(byteLen >= MAX_PLAYER_NAME) byteLen = MAX_PLAYER_NAME - 1;
    bsData.Read(szName, byteLen);
    szName[byteLen] = 0;
    
    snprintf(p->m_szName, MAX_PLAYER_NAME, "%s", szName);
}
ONRPC(SetPlayerPos)
{
    RAKDATA(params);
    
    CVector vecPos;
    bsData.Read(vecPos.x);
    bsData.Read(vecPos.y);
	bsData.Read(vecPos.z);
    
    //CLocalPlayer::GetEntity()->GetPosition() = vecPos;
    CLocalPlayer::GetEntity()->Teleport(vecPos);
    Game::RefreshStreamingAt(vecPos.m_vec2D);
}
ONRPC(SetPlayerHealth)
{
    RAKDATA(params);
    
    float fHealth;
    bsData.Read(fHealth);
    
    CLocalPlayer::GetEntity()->m_fHealth = fHealth;
}
ONRPC(TogglePlayerControllable)
{
    RAKDATA(params);
    
    uint8_t byteControllable;
    bsData.Read(byteControllable);
    
    CLocalPlayer::m_bDisableControls = !byteControllable;
}
ONRPC(PlayerPlaySound)
{
    RAKDATA(params);
    
    int iSound;
    float fX, fY, fZ;
    bsData.Read(iSound);
    bsData.Read(fX);
    bsData.Read(fY);
    bsData.Read(fZ);
    Game::PlaySound(iSound, fX, fY, fZ);
}
ONRPC(SetPlayerWorldBounds)
{
    RAKDATA(params);

    bsData.Read(samp->GetWorldBorderMax()->x);
    bsData.Read(samp->GetWorldBorderMin()->x);
    bsData.Read(samp->GetWorldBorderMax()->y);
    bsData.Read(samp->GetWorldBorderMin()->y);
    
    CLocalPlayer::m_vecWorldBorderCenter = CVector2D(
        0.5f * (samp->GetWorldBorderMax()->x - samp->GetWorldBorderMin()->x),
        0.5f * (samp->GetWorldBorderMax()->y - samp->GetWorldBorderMin()->y)
    );
    
    logger->Info("Border %f %f %f %f", samp->GetWorldBorderMax()->x, samp->GetWorldBorderMin()->x, samp->GetWorldBorderMax()->y, samp->GetWorldBorderMin()->y);
}
ONRPC(GivePlayerMoney)
{
    RAKDATA(params);
    int iAmount;
    bsData.Read(iAmount);

    Game::GiveMoney(iAmount);
}
ONRPC(SetPlayerFacingAngle)
{
    RAKDATA(params);
    
    float fAngle;
    bsData.Read(fAngle);
    
    CALLSCM(SET_CHAR_ROTATION, CLocalPlayer::GetGtaID(), 0.0f, 0.0f, fAngle);
    //CLocalPlayer::GetEntity()->m_fRotation1 = DegToRad(fAngle);
    //CLocalPlayer::GetEntity()->m_fRotation2 = DegToRad(fAngle);
}
ONRPC(ResetPlayerMoney)
{
    Game::SetMoney(0);
}
ONRPC(ResetPlayerWeapons)
{
    Game::ClearWeapons();
}
ONRPC(GivePlayerWeapon)
{
    RAKDATA(params);
    
    int iWeaponID;
    int iAmmo;
    bsData.Read(iWeaponID);
    bsData.Read(iAmmo);
    
    Game::GiveWeapon(iWeaponID, iAmmo, true);
}
ONRPC(SetTimeEx)
{
    RAKDATA(params);
    
    uint8_t byteHour;
    uint8_t byteMinute;
    bsData.Read(byteHour);
    bsData.Read(byteMinute);

    Game::SetWorldTime(byteHour, byteMinute);
    samp->GetServerVars().m_byteWorldHour = byteHour;
    samp->GetServerVars().m_byteWorldMinute = byteMinute;
}
ONRPC(ToggleClock)
{
    RAKDATA(params);
    
    bool enable;
    bsData.Read(enable);

    Game::ToggleClock(enable);
    if(enable)
    {
        Game::SetWorldTime(samp->GetServerVars().m_byteWorldHour, samp->GetServerVars().m_byteWorldMinute);
    }
}
ONRPC(WorldPlayerAdd)
{
    RAKDATA(params);

    unsigned short playerId;
    uint8_t byteFightingStyle = 4;
    uint8_t byteTeam = 0;
    unsigned int iSkin = 0;
    CVector vecPos;
    float fRotation = 0;
    uint32_t dwColor = 0;
    bool bVisible;

    bsData.Read(playerId);
    bsData.Read(byteTeam);
    bsData.Read(iSkin);
    bsData.Read(vecPos.x);
    bsData.Read(vecPos.y);
    bsData.Read(vecPos.z);
    bsData.Read(fRotation);
    bsData.Read(dwColor);
    bsData.Read(byteFightingStyle);
    bsData.Read(bVisible);

    CRemotePlayer* pRemotePlayer = Game::CreatePlayer(playerId, iSkin, vecPos.x, vecPos.y, vecPos.z, fRotation, bVisible);
    pRemotePlayer->m_pEntity->m_fHealth = 100.0f;
    pRemotePlayer->m_pEntity->m_fArmour = 0.0f;
    CALLSCM(GIVE_MELEE_ATTACK_TO_CHAR, pRemotePlayer->m_nGtaID, (int)byteFightingStyle, 6);
    //pRemotePlayer->m_pEntity->m_byteFightingStyle = (eFightingStyle)byteFightingStyle;
}
ONRPC(SetPlayerShopName)
{
    RAKDATA(params);
    
    uint8_t byteLen;
    char shopname[32];
    
    bsData.Read(byteLen);
    if(byteLen >= 32) byteLen = 31;
    bsData.Read(shopname, byteLen);
    shopname[byteLen] = 0;
    
    CALLSCM(LOAD_SHOP, shopname);
}
ONRPC(SetPlayerDrunkLevel)
{
    RAKDATA(params);
    
    // TODO:
    // 50000+ lvl?
    // Obviously there's some other logic, 
    // than just set player drunkess
}
ONRPC(RemovePlayerBuilding)
{
    RAKDATA(params);

    uint32_t nModelID;
    float fX, fY, fZ, fRadius;

    bsData.Read(nModelID);
    bsData.Read(fX);
    bsData.Read(fY);
    bsData.Read(fZ);
    bsData.Read(fRadius);

    Game::RemoveBuilding(nModelID, fX, fY, fZ, fRadius);
}
ONRPC(CreateObject)
{
    RAKDATA(params);
    
    uint16_t wObjectID;
    unsigned long ModelID;
    float fDrawDistance;

    uint8_t byteNoCamCol;
    uint16_t AttachedObjectID;
    unsigned short AttachedVehicleID;
    CVector AttachOffset, AttachRot;
    uint8_t bSyncRotation;

    CVector vecPos, vecRot;
    bsData.Read(wObjectID);
    bsData.Read(ModelID);

    bsData.Read(vecPos.x);
    bsData.Read(vecPos.y);
    bsData.Read(vecPos.z);

    bsData.Read(vecRot.x);
    bsData.Read(vecRot.y);
    bsData.Read(vecRot.z);

    bsData.Read(fDrawDistance);

    bsData.Read(byteNoCamCol);
    bsData.Read(AttachedVehicleID);
    bsData.Read(AttachedObjectID);

    if(AttachedObjectID != INVALID_OBJECT_ID || AttachedVehicleID != INVALID_VEHICLE_ID)
    {
        bsData.Read(AttachOffset.x);
        bsData.Read(AttachOffset.y);
        bsData.Read(AttachOffset.z);
        bsData.Read(AttachRot.x);
        bsData.Read(AttachRot.y);
        bsData.Read(AttachRot.z);
        bsData.Read(bSyncRotation);
    }

    uint8_t byteMaterialsCount = 0;
    bsData.Read(byteMaterialsCount);

    uint32_t dwRetID = 0;
    CALLSCM(CREATE_OBJECT, ModelID, vecPos.x, vecPos.y, vecPos.z, &dwRetID);
    CALLSCM(SET_OBJECT_ROTATION, dwRetID, vecRot.x, vecRot.y, vecRot.z);
}
ONRPC(SetMapIcon)
{
    RAKDATA(params);
    
    uint8_t byteIndex, byteIcon, byteStyle;
    uint32_t iColor;
    CVector pos;

    bsData.Read(byteIndex);
    bsData.Read(pos.x);
    bsData.Read(pos.y);
    bsData.Read(pos.z);
    bsData.Read(byteIcon);
	bsData.Read(iColor);
    bsData.Read(byteStyle);
    
    if(Game::m_pMapIconPool->IsSlotOccupied(byteIndex))
    {
        CALLSCM(REMOVE_BLIP, Game::m_pMapIconPool->GetAt(byteIndex)->m_nGtaID);
        Game::m_pMapIconPool->RemoveAt(byteIndex);
    }
    CRemoteMapIcon* m = Game::m_pMapIconPool->AllocAt(byteIndex);
    m->m_nID = byteIndex;
    m->m_nGtaID = Game::CreateRadarMarkerIcon(byteIcon, pos.x, pos.y, pos.z, iColor, byteStyle);
}
ONRPC(DialogBox)
{
    logger->Info("DialogBox");
    
    dialogui->SetDrawable(true);
}
ONRPC(SetPlayerArmour)
{
    RAKDATA(params);
    
    float fHealth;
    bsData.Read(fHealth);
    
    CLocalPlayer::GetEntity()->m_fArmour = fHealth;
}
ONRPC(SetSpawnInfo)
{
    RAKDATA(params);
	bsData.Read((char*)&CLocalPlayer::m_SpawnInfo, sizeof(PLAYER_SPAWN_INFO));
}
ONRPC(DisplayGameText)
{
    RAKDATA(params);
    
    char szMessage[512];
    int iType;
    int iTime;
    int iLength;

    bsData.Read(iType);
    bsData.Read(iTime);
    bsData.Read(iLength);

    if(iLength >= 512) iLength = 511;

    bsData.Read(szMessage, iLength);
	szMessage[iLength] = '\0';
    
    CALLSCM(CLEAR_PRINTS);
    Game::ShowBigMsg(szMessage, iTime, iType);
}
ONRPC(ForceSpawnSelection)
{
    CLocalPlayer::ForceSpawnSelection();
}
ONRPC(InterpolateCamera)
{
    RAKDATA(params);
    
    bool bSetPos;
    uint8_t byteCutType;
    uint32_t nTime;
    CVector from, to;
    bsData.Read(bSetPos);
    bsData.ReadVector(from.x, from.y, from.z);
    bsData.ReadVector(to.x, to.y, to.z);
    bsData.Read(nTime);
    bsData.Read(byteCutType);
    
    if(bSetPos)
    {
        CALLSCM(CAMERA_SET_VECTOR_MOVE, from.x, from.y, from.z, to.x, to.y, to.z, nTime, byteCutType - 1);
    }
    else
    {
        CALLSCM(CAMERA_SET_VECTOR_TRACK, from.x, from.y, from.z, to.x, to.y, to.z, nTime, byteCutType - 1);
    }
}
ONRPC(SetFightingStyle)
{
    RAKDATA(params);
    
    uint8_t byteFightingStyle = 4;
    bsData.Read(byteFightingStyle);
    CALLSCM(GIVE_MELEE_ATTACK_TO_CHAR, CLocalPlayer::GetGtaID(), (int)byteFightingStyle, 6);
    //CLocalPlayer::GetEntity()->m_byteFightingStyle = (eFightingStyle)byteFightingStyle;
}
ONRPC(SetPlayerVelocity)
{
    if(CLocalPlayer::GetEntity()->m_pVehicle) return;
    
    RAKDATA(params);
    
    CVector force;
    bsData.ReadVector(force.x, force.y, force.z);
    CLocalPlayer::GetEntity()->m_vecMoveSpeed = force;
}
ONRPC(SetVehicleVelocity)
{
    if(!CLocalPlayer::GetEntity()->m_pVehicle || CLocalPlayer::GetEntity()->IsPassenger()) return;
    
    RAKDATA(params);
    
    uint8_t forceType;
    CVector force;
    bsData.Read(forceType);
    bsData.ReadVector(force.x, force.y, force.z);
    if(forceType == 0) CLocalPlayer::GetEntity()->m_pVehicle->m_vecMoveSpeed = force;
    else if(forceType == 1) CLocalPlayer::GetEntity()->m_pVehicle->m_vecTurnSpeed = force;
}
ONRPC(ClientMessage)
{
    RAKDATA(params);
    
    uint32_t color;
    uint8_t byteLen;
    char msg[512];
    
    bsData.Read(color);
    bsData.Read(byteLen);
    bsData.Read(msg, byteLen);
    msg[byteLen] = 0;
    
    samp->Message(msg);
}
ONRPC(NumberPlate)
{
    RAKDATA(params);
    
    uint16_t vehId;
    uint8_t byteLen; char plateTxt[16];
    
    bsData.Read(vehId);
    bsData.Read(byteLen);
    if(byteLen >= 16) byteLen = 15;
    bsData.Read(plateTxt, byteLen);
    plateTxt[byteLen] = 0;
    
    CRemoteVehicle* p = Game::m_pVehiclePool->GetAt(vehId);
    if(p) Game::SetVehiclePlate(p->m_pEntity, plateTxt);
}
ONRPC(RequestClass)
{
    RAKDATA(params);
    uint8_t byteRequestOutcome = 0;
    
    bsData.Read(byteRequestOutcome);
    logger->Info("RequestClass (%d)", (uint32_t)byteRequestOutcome);
    
	bsData.Read((char*)&CLocalPlayer::m_SpawnInfo, sizeof(PLAYER_SPAWN_INFO));
    
    if(byteRequestOutcome)
    {
        CLocalPlayer::SetModelIndex(CLocalPlayer::m_SpawnInfo.iSkin);
        spawnui->SetDrawable(true);
        
        CLocalPlayer::GetEntity()->m_fHealth = 100;
        CLocalPlayer::GetEntity()->m_fArmour = 0;
        CLocalPlayer::m_bDisableControls = true;
    }
}
ONRPC(RequestSpawn)
{
    RAKDATA(params);

    uint8_t byteRequestOutcome = 0;
	bsData.Read(byteRequestOutcome);
    
    CLocalPlayer::Spawn();
    spawnui->SetDrawable(false);
}
ONRPC(ServerJoin)
{
    RAKDATA(params);
    
    char szPlayerName[MAX_PLAYER_NAME+1];
    uint16_t playerId;
    uint8_t byteNameLen;
    uint8_t bIsNPC;
    uint32_t color;
    
    bsData.Read(playerId);
    bsData.Read(color);
    bsData.Read(bIsNPC);
    bsData.Read(byteNameLen);
    if(byteNameLen >= MAX_PLAYER_NAME) byteNameLen = MAX_PLAYER_NAME-1;
    bsData.Read(szPlayerName, (int)byteNameLen);
    szPlayerName[byteNameLen] = 0;
    
    CRemotePlayer* p = Game::m_pPlayerPool->AllocAt(playerId, true);
    
    p->SetName(szPlayerName);
    p->m_nID = playerId;
}
ONRPC(ServerQuit)
{
    RAKDATA(params);
    
    uint16_t pid;
    uint8_t reason;
    
    bsData.Read(pid);
    bsData.Read(reason);
    
    CRemotePlayer* p = Game::m_pPlayerPool->GetAt(pid);
    if(!p) return; // bruh
    
    p->KillBlip();
    Game::RemovePlayerFromWorld(p);
    Game::m_pPlayerPool->RemoveAt(pid);
}
ONRPC(InitGame)
{
    logger->Info("InitGame");
    
    RAKDATA(params);
        
    unsigned short MyPlayerID;
    bool bLanMode, bStuntBonus;
    
    bsData.ReadCompressed(samp->GetServerVars().m_bZoneNames);
    bsData.ReadCompressed(samp->GetServerVars().m_bUseCJWalk);
    bsData.ReadCompressed(samp->GetServerVars().m_bAllowWeapons);
    bsData.ReadCompressed(samp->GetServerVars().m_bLimitGlobalChatRadius);
    bsData.Read(samp->GetServerVars().m_fGlobalChatRadius);
    bsData.ReadCompressed(samp->GetServerVars().m_bStuntBonus);
    bsData.Read(samp->GetServerVars().m_fNameTagDrawDistance);
    bsData.ReadCompressed(samp->GetServerVars().m_bDisableEnterExits);
    bsData.ReadCompressed(samp->GetServerVars().m_bNameTagLOS);
    bsData.ReadCompressed(samp->GetServerVars().m_bManualVehicleEngineAndLight);
    bsData.Read(samp->GetServerVars().m_iSpawnsAvailable);
    bsData.Read(MyPlayerID);
    bsData.ReadCompressed(samp->GetServerVars().m_bShowPlayerTags);
    bsData.Read(samp->GetServerVars().m_iShowPlayerMarkers);
    bsData.Read(samp->GetServerVars().m_byteWorldHour);
    bsData.Read(samp->GetServerVars().m_byteWeather);
    bsData.Read(samp->GetServerVars().m_fGravity);
    bsData.ReadCompressed(samp->GetServerVars().m_bLanMode);
    bsData.Read(samp->GetServerVars().m_iDeathDropMoney);
    bsData.ReadCompressed(samp->GetServerVars().m_bInstagib); // Always false

    bsData.Read(samp->GetServerVars().m_iNetModeNormalOnfootSendRate);
    bsData.Read(samp->GetServerVars().m_iNetModeNormalInCarSendRate);
    bsData.Read(samp->GetServerVars().m_iNetModeFiringSendRate);
    bsData.Read(samp->GetServerVars().m_iNetModeSendMultiplier);
    bsData.Read(samp->GetServerVars().m_iLagCompensation);

    uint8_t byteStrLen;
    bsData.Read(byteStrLen);
    if(byteStrLen)
    {
        char* name = new char[byteStrLen]; name[0] = 0;
        bsData.Read(name, byteStrLen);
        name[byteStrLen] = 0;
        samp->SetHostname(name);
        delete[] name;
    }

    char byteVehicleModels[212];
    bsData.Read(&byteVehicleModels[0], 212); // Unused?
    bsData.Read(samp->GetServerVars().m_iVehicleFriendlyFire);

    samp->SetGameState(GAMESTATE_CONNECTED);
    samp->InitializeFromVars();
    
    Game::ToggleCJWalk(samp->GetServerVars().m_bUseCJWalk);
    
    CLocalPlayer::OnConnected();
}
ONRPC(DisableMapIcon)
{
    RAKDATA(params);
    
    uint8_t byteIndex;
    bsData.Read(byteIndex);
    
    if(Game::m_pMapIconPool->IsSlotOccupied(byteIndex))
    {
        CALLSCM(REMOVE_BLIP, Game::m_pMapIconPool->GetAt(byteIndex)->m_nGtaID);
        Game::m_pMapIconPool->RemoveAt(byteIndex);
    }
}
ONRPC(Weather)
{
    RAKDATA(params);
    
    uint8_t byteInterior;
    bsData.Read(byteInterior);
    
    samp->GetServerVars().m_byteWeather = byteInterior;
}
ONRPC(SetPlayerSkin)
{
    RAKDATA(params);
    
    uint32_t playerId, skin;
    bsData.Read(playerId); // Why does it use 32bit val here, lol?
    bsData.Read(skin);
    
    CRemotePlayer* p = Game::m_pPlayerPool->GetAt(playerId);
    if(p) p->SetModelIndex(skin);
}
ONRPC(SetInterior)
{
    RAKDATA(params);
    
    uint8_t byteInterior;
    bsData.Read(byteInterior);
    
    CLocalPlayer::SetInterior(byteInterior);
}
ONRPC(SetCameraPos)
{
    RAKDATA(params);
        
    CVector vecPos;
    bsData.Read(vecPos.x);
    bsData.Read(vecPos.y);
	bsData.Read(vecPos.z);
    
    CALLSCM(SET_FIXED_CAMERA_POSITION, vecPos.x, vecPos.y, vecPos.z, 0.0f, 0.0f, 0.0f);
}
ONRPC(SetCameraLookAt)
{
    RAKDATA(params);
        
    CVector vecPos;
    bsData.Read(vecPos.x);
    bsData.Read(vecPos.y);
	bsData.Read(vecPos.z);
    
    CALLSCM(POINT_CAMERA_AT_POINT, vecPos.x, vecPos.y, vecPos.z, 2);
}
ONRPC(SetCameraBehindPlayer)
{
    CALLSCM(RESTORE_CAMERA_JUMPCUT);
}
ONRPC(WorldPlayerRemove)
{
    RAKDATA(params);
    
    uint16_t pid;
    bsData.Read(pid);
    
    CRemotePlayer* p = Game::m_pPlayerPool->GetAt(pid);
    if(!p) return; // bruh
    
    Game::RemovePlayerFromWorld(p);
    
    p->m_pEntity = NULL;
    p->m_nGtaID = -1;
}
ONRPC(WorldVehicleAdd)
{
    RAKDATA(params);
        
    VehicleData NewVehicle;
    bsData.Read((char*)&NewVehicle, sizeof(VehicleData));

    if(NewVehicle.iVehicleType < 400 || NewVehicle.iVehicleType > 611) return;

    samp->Message("WorldVehicleAdd %d", (int)NewVehicle.iVehicleType);
    CRemoteVehicle* vehInfo = Game::CreateVehicle(NewVehicle);
    if(vehInfo)
    {
        CVehicle* pVehicle = vehInfo->m_pEntity;
        vehInfo->data = NewVehicle;
        if(NewVehicle.iVehicleType != 537 || NewVehicle.iVehicleType != 538)
        {
            if ( pVehicle->m_nVehicleSubType == 1 )
            {
                //for(int i = 0; i < 14; i++)
                //{
                //    uint32_t modslot = NewVehicle.byteModSlots[i];
                //    if(modslot) pVehicle->AddComponent(modslot + 999);
                //}
                //
                //if(NewVehicle.bytePaintjob)
                //{
                //    pVehicle->SetPaintJob(NewVehicle.bytePaintjob - 1);
                //}
            }
            if(NewVehicle.cColor1 != -1 || NewVehicle.cColor2 != -1)
            {
                //pVehicle->SetColor(NewVehicle.cColor1, NewVehicle.cColor2);
            }
        }
    }
}
ONRPC(WorldVehicleRemove)
{
    RAKDATA(params);
    
    uint16_t vehId;
    bsData.Read(vehId);
    Game::RemoveVehicle(vehId);
}

void SAMPRPC::DoRPCs(bool bUnregister)
{
    if(!bUnregister) logger->Info("Initializing RPCs...");
    else logger->Info("Shutting down RPCs...");

    BINDRPC(SetPlayerName); // 11
    BINDRPC(SetPlayerPos); // 12
    BINDRPC(SetPlayerHealth); // 14
    BINDRPC(TogglePlayerControllable); // 15
    BINDRPC(PlayerPlaySound); // 16
    BINDRPC(SetPlayerWorldBounds); // 17
    BINDRPC(GivePlayerMoney); // 18
    BINDRPC(SetPlayerFacingAngle); // 19
    BINDRPC(ResetPlayerMoney); // 20
    BINDRPC(ResetPlayerWeapons); // 21
    BINDRPC(GivePlayerWeapon); // 22
    BINDRPC(SetTimeEx); // 29
    BINDRPC(ToggleClock); // 30
    BINDRPC(WorldPlayerAdd); // 32
    BINDRPC(SetPlayerShopName); // 33
    BINDRPC(SetPlayerDrunkLevel); // 35
    BINDRPC(RemovePlayerBuilding); // 43
    BINDRPC(CreateObject); // 44
    BINDRPC(SetMapIcon); // 56
    BINDRPC(DialogBox); // 61
    BINDRPC(SetPlayerArmour); // 66
    BINDRPC(SetSpawnInfo); // 68
    BINDRPC(DisplayGameText); // 73
    BINDRPC(ForceSpawnSelection); // 74
    BINDRPC(InterpolateCamera); // 82
    BINDRPC(SetFightingStyle); // 89
    BINDRPC(SetPlayerVelocity); // 90
    BINDRPC(SetVehicleVelocity); // 91
    BINDRPC(ClientMessage); // 93
    BINDRPC(NumberPlate); // 123
    BINDRPC(RequestClass); // 128
    BINDRPC(RequestSpawn); // 129
    BINDRPC(ServerJoin); // 137
    BINDRPC(ServerQuit); // 138
    BINDRPC(InitGame); // 139
    BINDRPC(DisableMapIcon); // 144
    BINDRPC(Weather); // 152
    BINDRPC(SetPlayerSkin); // 153
    BINDRPC(SetInterior); // 156
    BINDRPC(SetCameraPos); // 157
    BINDRPC(SetCameraLookAt); // 158
    BINDRPC(SetCameraBehindPlayer); // 162
    BINDRPC(WorldPlayerRemove); // 163
    BINDRPC(WorldVehicleAdd); // 164
    BINDRPC(WorldVehicleRemove); // 165
}
