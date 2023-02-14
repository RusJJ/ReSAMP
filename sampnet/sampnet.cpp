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

    m_vecWorldBorderMin = new CVector(-20000.0f, -20000.0f, -500.0f);
    m_vecWorldBorderMax = new CVector(20000.0f, 20000.0f, 20000.0f);

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
    m_pRakClient->Send(&bsKey, SYSTEM_PRIORITY, RELIABLE, 0);
}
DEFPAK(29, ConnectionAttemptFailed)
{
    Message("The server didn't respond. Retrying...");
    SetGameState(GAMESTATE_ATTEMPT_TO_JOIN);
}
DEFPAK(31, ConnectionNoFree)
{
    Message("The server is full. Retrying...");
    SetGameState(GAMESTATE_ATTEMPT_TO_JOIN);
}
DEFPAK(32, ConnectionDisconnected)
{
    Message("Server closed the connection.");
    SetGameState(GAMESTATE_DISCONNECTED);

    m_pRakClient->Disconnect(2000);
}
DEFPAK(33, ConnectionLost)
{
    Message("Lost connection to the server. Reconnecting...");
    SetGameState(GAMESTATE_ATTEMPT_TO_JOIN);
}
DEFPAK(34, ConnectionAccepted)
{
    unsigned short nMyPlayerID;
    unsigned int uiChallenge;

    SetGameState(GAMESTATE_AWAIT_JOIN);

    RakNet::BitStream bsSuccAuth((unsigned char *)pak->data, pak->length, false);  
    bsSuccAuth.IgnoreBits(8);
    bsSuccAuth.IgnoreBits(32);
    bsSuccAuth.IgnoreBits(16);
    bsSuccAuth.Read(nMyPlayerID);
    bsSuccAuth.Read(uiChallenge);

    snprintf(m_szCurrentPlayerName, sizeof(m_szCurrentPlayerName), "%s", m_szPlayerName);
    char bNameLen = (char)strlen(m_szCurrentPlayerName);
    CLocalPlayer::PutMeInPool(nMyPlayerID);

    RakNet::BitStream bsSend;
    bsSend.Write(NETGAME_VERSION);
    bsSend.Write((char)0x01); // Should be 1 char!
    bsSend.Write(bNameLen);
    bsSend.Write(m_szCurrentPlayerName, bNameLen);
    bsSend.Write(uiChallenge ^ NETGAME_VERSION);
    bsSend.Write(SVAR(AUTH_BS));
    bsSend.Write(AUTH_BS, SVAR(AUTH_BS));
    bsSend.Write(SVAR(SAMP_VERSION));
    bsSend.Write(SAMP_VERSION, SVAR(SAMP_VERSION));
    m_pRakClient->RPC((int*)&RPC_ClientJoin, &bsSend, HIGH_PRIORITY, RELIABLE, 0, false, UNASSIGNED_NETWORK_ID, NULL);
}
DEFPAK(35, FailedToAuth)
{
    Message("Failed to initialize encryption.");
    SetGameState(GAMESTATE_DISCONNECTED);
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
    // Our controllable puppet
    CLocalPlayer::Update();

    // Players pool
    CRemotePlayer* player;
    unsigned short maxplayers = Game::m_pPlayerPool->GetHighestSlotUsedEver() + 1;
    for(int i = 0; i < maxplayers; ++i)
    {
        player = Game::m_pPlayerPool->GetAt(i);
        if(player && player->IsActive()) player->Update();
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
