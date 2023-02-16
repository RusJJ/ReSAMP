#include <mod/amlmod.h>
#include <mod/logger.h>
#include <samp.h>

#include "samprpc.h"
#include "sampnet.h"
#include <game/game.h>
#include <game/scripting.h>
#include <game/localplayer.h>
#include <ui/spawnscreen.h>
#include <pools/remoteplayer.h>

#define RAKDATA(_PARAMS) RakNet::BitStream bsData((unsigned char*)(_PARAMS->input), ((int)(_PARAMS->numberOfBitsOfData) / 8) + 1, false)


ONRPC(SetPlayerPos)
{
    RAKDATA(params);
    
    CVector vecPos;
    bsData.Read(vecPos.x);
    bsData.Read(vecPos.y);
	bsData.Read(vecPos.z);
    
    CLocalPlayer::GetEntity()->GetPosition() = vecPos;
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
    
    CLocalPlayer::m_bDisableControls = byteControllable;
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
    
    //CLocalPlayer::GetEntity()->SetHeading(fAngle);
    CLocalPlayer::GetEntity()->m_fRotation1 = DegToRad(fAngle);
    CLocalPlayer::GetEntity()->m_fRotation2 = DegToRad(fAngle);
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
    pRemotePlayer->GetEntity()->m_fHealth = 100.0f;
    pRemotePlayer->GetEntity()->m_fArmour = 0.0f;
}
ONRPC(SetPlayerDrunkLevel)
{
    RAKDATA(params);
    
    
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

    if(iLength > 512) return;

    bsData.Read(szMessage, iLength);
	szMessage[iLength] = '\0';
    
    CALLSCM(CLEAR_PRINTS);
    Game::ShowBigMsg(szMessage, iTime, iType);
}
ONRPC(RequestClass)
{
    RAKDATA(params);
    uint8_t byteRequestOutcome = 0;
    
    bsData.Read(byteRequestOutcome);
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
ONRPC(InitGame)
{
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
    
    unsigned short vehId;
    bsData.Read(vehId);
    CRemoteVehicle* v = Game::m_pVehiclePool->GetAt(vehId);
    
    if(v->m_nMarkerID) CALLSCM(REMOVE_BLIP, v->m_nMarkerID);
    CALLSCM(DELETE_CAR, v->m_nGtaID);
    Game::m_pVehiclePool->Remove(v);
}

void SAMPRPC::DoRPCs(bool bUnregister)
{
    if(!bUnregister) logger->Info("Initializing RPCs...");
    else logger->Info("Shutting down RPCs...");

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
    BINDRPC(SetPlayerDrunkLevel); // 35
    BINDRPC(RemovePlayerBuilding); // 43
    BINDRPC(CreateObject); // 44
    BINDRPC(SetMapIcon); // 56
    BINDRPC(SetPlayerArmour); // 66
    BINDRPC(SetSpawnInfo); // 68
    BINDRPC(DisplayGameText); // 73
    BINDRPC(RequestClass); // 128
    BINDRPC(RequestSpawn); // 129
    BINDRPC(InitGame); // 139
    BINDRPC(DisableMapIcon); // 144
    BINDRPC(Weather); // 152
    BINDRPC(SetInterior); // 156
    BINDRPC(SetCameraPos); // 157
    BINDRPC(SetCameraLookAt); // 158
    BINDRPC(WorldVehicleAdd); // 164
    BINDRPC(WorldVehicleRemove); // 165
}
