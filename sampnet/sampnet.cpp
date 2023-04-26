#include <mod/amlmod.h>
#include <mod/logger.h>
#include <samp.h>
#include <gtasa.h>

#include "sampnet.h"
#include <game/game.h>
#include <game/localplayer.h>
#include <game/scripting.h>
#include <ui/spawnscreen.h>

SAMPNet* samp = NULL;

int* m_FrameCounter;
void SAMPNet::StartSAMP()
{
    samp = new SAMPNet;
    if(samp == NULL) return; // Memory allocation failed

    SAMPRPC::DoRPCs(false);
    Game::ToggleThePassingOfTime(false);

    SET_TO(m_FrameCounter, aml->GetSym(hGTASA, "_ZN6CTimer14m_FrameCounterE"));
    samp->Message("SAMP Initialized!");
}

unsigned char SAMPNet::GetPacketID(Packet* pak)
{
    if(pak == NULL) return 0xFF;
    if (((unsigned char)pak->data[0]) == ID_TIMESTAMP) return (unsigned char)(pak->data[sizeof(unsigned char) + sizeof(unsigned long)]);
    return (unsigned char) pak->data[0];
}

SAMPNet::SAMPNet()
{
    snprintf(m_szHostName, sizeof(m_szHostName), "San Andreas Multiplayer");
    snprintf(m_szIP, sizeof(m_szIP), "127.0.0.1");
    snprintf(m_szPlayerName, sizeof(m_szPlayerName), "Carl_Johnson");
    m_iPort = 7777;

    m_pRakClient = RakNetworkFactory::GetRakClientInterface();

    m_vecWorldBorderMin = CVector2D(-20000.0f, -20000.0f);
    m_vecWorldBorderMax = CVector2D(20000.0f, 20000.0f);

    m_bStartedForConnecting = false;
    Game::ToggleClock(false);
    ResetVariables();
    SetGameState(GAMESTATE_IDLING);
}

void gen_auth_key(char buf[260], char* auth_in);
DEFPAK(12, AuthKey)
{
    RakNet::BitStream bsAuth((unsigned char *)pak->data, pak->length, false);

    uint8_t byteAuthLen;
    char szAuth[260];

    bsAuth.IgnoreBits(8);
    bsAuth.Read(byteAuthLen);
    bsAuth.Read(szAuth, byteAuthLen);
    szAuth[byteAuthLen] = '\0';

    char szAuthKey[260];
    gen_auth_key(szAuthKey, szAuth);

    RakNet::BitStream bsKey;
    uint8_t byteAuthKeyLen = (uint8_t)strlen(szAuthKey);

    bsKey.Write((uint8_t)ID_AUTH_KEY);
    bsKey.Write((uint8_t)byteAuthKeyLen);
    bsKey.Write(szAuthKey, byteAuthKeyLen);
    samp->m_pRakClient->Send(&bsKey, SYSTEM_PRIORITY, RELIABLE, 0);
}
DEFPAK(29, ConnectionAttemptFailed)
{
    samp->Message("The server didn't respond. Retrying...");
    samp->SetGameState(GAMESTATE_ATTEMPT_TO_JOIN);
}
DEFPAK(31, ConnectionNoFree)
{
    samp->Message("The server is full. Retrying...");
    samp->SetGameState(GAMESTATE_ATTEMPT_TO_JOIN);
}
DEFPAK(32, ConnectionDisconnected)
{
    samp->Message("Server closed the connection.");
    samp->SetGameState(GAMESTATE_DISCONNECTED);

    samp->m_pRakClient->Disconnect(2000);
}
DEFPAK(33, ConnectionLost)
{
    samp->Message("Lost connection to the server. Reconnecting...");
    samp->SetGameState(GAMESTATE_ATTEMPT_TO_JOIN);
}
DEFPAK(34, ConnectionAccepted)
{
    uint16_t nMyPlayerID;
    uint32_t uiChallenge;

    samp->SetGameState(GAMESTATE_AWAIT_JOIN);

    RakNet::BitStream bsSuccAuth((unsigned char *)pak->data, pak->length, false);  
    bsSuccAuth.IgnoreBits(8);
    bsSuccAuth.IgnoreBits(32);
    bsSuccAuth.IgnoreBits(16);
    bsSuccAuth.Read(nMyPlayerID);
    bsSuccAuth.Read(uiChallenge);

    snprintf(samp->m_szCurrentPlayerName, sizeof(samp->m_szCurrentPlayerName), "%s", samp->m_szPlayerName);
    uint8_t byteNameLen = (uint8_t)strlen(samp->m_szCurrentPlayerName);
    CLocalPlayer::PutMeInPool(nMyPlayerID);

    RakNet::BitStream bsSend;
    bsSend.Write(NETGAME_VERSION);
    bsSend.Write((char)0x01); // Should be 1 char!
    bsSend.Write(byteNameLen);
    bsSend.Write(samp->m_szCurrentPlayerName, byteNameLen);
    bsSend.Write(uiChallenge ^ NETGAME_VERSION);
    bsSend.Write(SVAR(AUTH_BS));
    bsSend.Write(AUTH_BS, SVAR(AUTH_BS));
    bsSend.Write(SVAR(SAMP_VERSION)); // Useless!
    bsSend.Write(SAMP_VERSION, SVAR(SAMP_VERSION));
    samp->m_pRakClient->RPC((int*)&RPC_ClientJoin, &bsSend, HIGH_PRIORITY, RELIABLE, 0, false, UNASSIGNED_NETWORK_ID, NULL);
}
DEFPAK(35, FailedToAuth)
{
    samp->Message("Failed to initialize encryption.");
    samp->SetGameState(GAMESTATE_DISCONNECTED);
}
DEFPAK(203, AimSync)
{
    
}
DEFPAK(205, PlayerStatsSync)
{
    
}
DEFPAK(207, PlayerSync)
{
    if(samp->GetGameState() != GAMESTATE_CONNECTED) return;
    samp->Message("Player syncing...");
    
    RakNet::BitStream bsPlayerSync((unsigned char *)pak->data, pak->length, false);
    
    uint8_t bytePacketID;
    uint16_t playerId;

    bool bHasLR,bHasUD;
    bool bHasVehicleSurfingInfo;

    bsPlayerSync.Read(bytePacketID);
    bsPlayerSync.Read(playerId);
    
    CRemotePlayer* p = Game::m_pPlayerPool->GetAt(playerId);
    if(!p || p->m_bIsLocal) return; // bruh
    
    p->m_byteState = PLAYER_STATE_ONFOOT;
    ONFOOT_SYNC_DATA& ofSync = p->m_ofSync;
    memset(&p->m_ofSync, 0, sizeof(ONFOOT_SYNC_DATA));
    
    // LEFT/RIGHT KEYS
    bsPlayerSync.Read(bHasLR);
    if(bHasLR) bsPlayerSync.Read(ofSync.lrAnalog);

    // UP/DOWN KEYS
    bsPlayerSync.Read(bHasUD);
    if(bHasUD) bsPlayerSync.Read(ofSync.udAnalog);

    // GENERAL KEYS
    bsPlayerSync.Read(ofSync.wKeys);

    // VECTOR POS
    bsPlayerSync.Read((char*)&ofSync.vecPos,sizeof(CVector));

    // QUATERNION
    float tw, tx, ty, tz;
    bsPlayerSync.ReadNormQuat(tw, tx, ty, tz);
    ofSync.quat.w = tw;
    ofSync.quat.x = tx;
    ofSync.quat.y = ty;
    ofSync.quat.z = tz;

    // HEALTH/ARMOUR (COMPRESSED INTO 1 BYTE)
    uint8_t byteHealthArmour;
    uint8_t byteArmTemp=0,byteHlTemp=0;

    bsPlayerSync.Read(byteHealthArmour);
    
    logger->Info("PlayerSync %d %d", (int)playerId, (int)byteHealthArmour);
    
    byteArmTemp = (byteHealthArmour & 0x0F);
    byteHlTemp = (byteHealthArmour >> 4);

    /*if(byteArmTemp == 0xF) ofSync.byteArmour = 100;
    else if(byteArmTemp == 0) ofSync.byteArmour = 0;
    else*/ ofSync.byteArmour = byteArmTemp * 7;

    /*if(byteHlTemp == 0xF) ofSync.byteHealth = 100;
    else if(byteHlTemp == 0) ofSync.byteHealth = 0;
    else*/ ofSync.byteHealth = byteHlTemp * 7;

    // CURRENT WEAPON
    bsPlayerSync.Read(ofSync.byteCurrentKeyAndWeapon);
    // SPECIAL ACTION
    bsPlayerSync.Read(ofSync.byteSpecialAction);

    // READ MOVESPEED VECTORS
    bsPlayerSync.ReadVector(tx, ty, tz);
    ofSync.vecMoveSpeed.x = tx;
    ofSync.vecMoveSpeed.y = ty;
    ofSync.vecMoveSpeed.z = tz;

    bsPlayerSync.Read(bHasVehicleSurfingInfo);
    if (bHasVehicleSurfingInfo) 
    {
        bsPlayerSync.Read(ofSync.wSurfInfo);
        bsPlayerSync.Read(ofSync.vecSurfOffsets.x);
        bsPlayerSync.Read(ofSync.vecSurfOffsets.y);
        bsPlayerSync.Read(ofSync.vecSurfOffsets.z);
    } 
    else
        ofSync.wSurfInfo = INVALID_VEHICLE_ID;
        
    p->m_bOnfootDataChanged = true;
}


DEFPAK(208, MarkerSync)
{
    if(samp->GetGameState() != GAMESTATE_CONNECTED) return;

    uint8_t bytePacketID;
    uint32_t nPlayers;
    
    RakNet::BitStream bsMarkerSync((unsigned char *)pak->data, pak->length, false);
    bsMarkerSync.Read(bytePacketID);
	bsMarkerSync.Read(nPlayers);
    
    if(nPlayers)
    {
        uint16_t pid;
        bool bStreamMarker;
        
        int16_t shortTemp;
        CVector vecTemp;
        
        for(uint32_t i = 0; i < nPlayers; ++i)
        {
            bsMarkerSync.Read(pid);
            bsMarkerSync.Read(bStreamMarker);
            if(bStreamMarker)
            {
                bsMarkerSync.Read(shortTemp); vecTemp.x = (float)shortTemp;
                bsMarkerSync.Read(shortTemp); vecTemp.y = (float)shortTemp;
                bsMarkerSync.Read(shortTemp); vecTemp.z = (float)shortTemp;
                
                CRemotePlayer* p = Game::m_pPlayerPool->GetAt(pid);
                if(p)
                {
                    p->m_vecMarkerPos = vecTemp;
                }
            }
        }
    }
}

void SAMPNet::Connect(const char* ip, unsigned short port, const char* password)
{
    snprintf(m_szIP, sizeof(m_szIP), "%s", ip);
    m_iPort = port;
    if(password != NULL && password[0] != 0) m_pRakClient->SetPassword(password);
    SetGameState(GAMESTATE_ATTEMPT_TO_JOIN);
}

void SAMPNet::SetPlayerName(const char* name)
{
    snprintf(m_szPlayerName, sizeof(m_szPlayerName), "%s", name);
}

void SAMPNet::Update()
{
    ProcessPackets();
    Game::SetWorldTime(m_vars.m_byteWorldHour, m_vars.m_byteWorldMinute);
    Game::SetWeather(m_vars.m_byteWeather);
    
    if(GetGameState() != GAMESTATE_CONNECTED && !m_bStartedForConnecting)
    {
        m_bStartedForConnecting = true;
        CLocalPlayer::GetMe();
        if(CLocalPlayer::GetGtaID() != 0)
        {
            if(CALLSCM(IS_CHAR_IN_CAR, CLocalPlayer::GetGtaID()))
            {
                CALLSCM(WARP_CHAR_FROM_CAR_TO_COORD, CLocalPlayer::GetGtaID(), 1093.4f, -2036.5f, 82.7106f);
            }
            else
            {
                CALLSCM(SET_CHAR_COORDINATES, CLocalPlayer::GetGtaID(), 1093.4f, -2036.5f, 82.7106f);
            }
        }
        CALLSCM(SET_FIXED_CAMERA_POSITION, 1093.0f, -2036.0f, 90.0f, 0.0f, 0.0f, 0.0f);
        CALLSCM(POINT_CAMERA_AT_POINT, 384.0f, -1557.0f, 20.0f, 2);
        CALLSCM(DISPLAY_RADAR, false);
        CALLSCM(DISPLAY_HUD, false);
        Game::SetWeather(1);
        Game::ShowWidgets(false);
        CLocalPlayer::m_bDisableControls = true;
    }

    if(GetGameState() == GAMESTATE_ATTEMPT_TO_JOIN && *m_FrameCounter > 120)
    {
        m_pRakClient->Connect(m_szIP, m_iPort, 0, 0, 5);
        Message("Connecting to %s:%d...", m_szIP, m_iPort);
        SetGameState(GAMESTATE_CONNECTING);
        return;
    }

    if(GetGameState() == GAMESTATE_CONNECTED)
    {
        if(m_bStartedForConnecting)
        {
            m_bStartedForConnecting = false;
            CALLSCM(DISPLAY_RADAR, true);
            CALLSCM(DISPLAY_HUD, true);
        }
        ProcessPools();
    }
    else
    {

    }
}

void SAMPNet::ProcessPools()
{
    // Players pool
    CRemotePlayer* player;
    unsigned short maxplayers = Game::m_pPlayerPool->GetHighestSlotUsedEver();
    for(int i = 0; i <= maxplayers; ++i)
    {
        player = Game::m_pPlayerPool->GetAt(i);
        if(player) player->Update();
    }
}

void SAMPNet::ProcessPackets()
{
    static Packet* pak = NULL;

    while((pak = m_pRakClient->Receive()) != NULL)
    {
        switch(GetPacketID(pak))
        {
            PROCPAK(AuthKey);
            PROCPAK(ConnectionAttemptFailed);
            PROCPAK(ConnectionNoFree);
            PROCPAK(ConnectionDisconnected);
            PROCPAK(ConnectionLost);
            PROCPAK(ConnectionAccepted);
            PROCPAK(FailedToAuth);
            PROCPAK(AimSync);
            PROCPAK(PlayerSync);
            PROCPAK(MarkerSync);

            default: break;
        }
        m_pRakClient->DeallocatePacket(pak);
    }
}

void SAMPNet::Message(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    logger->InfoV(format, args);

    va_end(args);
}

void SAMPNet::ShutDownForGameRestart()
{
    SetGameState(GAMESTATE_RESTARTING);
}

void SAMPNet::SendChatMessage(const char* msg)
{
    if(GetGameState() != GAMESTATE_CONNECTED) return;

}

void SAMPNet::SendChatCommand(const char* msg)
{
    if(GetGameState() != GAMESTATE_CONNECTED) return;

}

void SAMPNet::ResetVariables()
{
    m_vars.m_byteWorldHour = 12;
    m_vars.m_byteWorldMinute = 0;
    m_vars.m_byteWeather = 10;
    m_vars.m_fGravity = 0.008f;
    m_vars.m_iDeathDropMoney = 0;
}

void SAMPNet::InitializeFromVars()
{
    CALLSCM(FORCE_WEATHER_NOW, m_vars.m_byteWeather);
    Game::SetWorldTime(m_vars.m_byteWorldHour, m_vars.m_byteWorldMinute);
    if(m_vars.m_bDisableEnterExits) Game::ToggleEnterExits(false);
    Game::SetGravity(m_vars.m_fGravity);
}

const char* SAMPNet::GetRejectedString(uint8_t rejectReason)
{
    switch(rejectReason)
    {
        case 0: return "Ignore";
        case 1: return "Version mismatch";
        case 2: return "Bad name";
        case 3: return "Bad mod code";
        case 4: return "No player slot";
        case 5: return "Success";
        default: break;
    }
    return "Unknown";
}
