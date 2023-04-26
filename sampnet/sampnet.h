#ifndef __SAMPNET_H
#define __SAMPNET_H

#include "raknet/RakClientInterface.h"
#include "raknet/RakNetworkFactory.h"
#include "raknet/PacketEnumerations.h"
#include "raknet/StringCompressor.h"

#include <gtasa.h>
#include "samprpc.h"

#define GAMESTATE_IDLING            0
#define GAMESTATE_ATTEMPT_TO_JOIN   1
#define GAMESTATE_DISCONNECTED      2
#define GAMESTATE_CONNECTING        3
#define GAMESTATE_CONNECTED         4
#define GAMESTATE_AWAIT_JOIN        5
#define GAMESTATE_RESTARTING        6

#define INVALID_PLAYER_ID           0xFFFF
#define INVALID_VEHICLE_ID          0xFFFF
#define INVALID_OBJECT_ID           0xFFF9


#define SVAR(_var)                  (char)(sizeof(_var)-1)
#define DEFPAK(__id, __name)        static const unsigned char ID_##__name = (unsigned char)__id; static void Process_##__name(Packet* pak)
#define PROCPAK(__name)             case ID_##__name: Process_##__name(pak); break

struct ServerVariables
{
    bool  m_bZoneNames;
    bool  m_bUseCJWalk;
    bool  m_bAllowWeapons;
    bool  m_bLimitGlobalChatRadius;
    float m_fGlobalChatRadius;
    bool  m_bStuntBonus;
    float m_fNameTagDrawDistance;
    bool  m_bDisableEnterExits;
    bool  m_bNameTagLOS;
    bool  m_bManualVehicleEngineAndLight;
    int   m_iSpawnsAvailable;
    bool  m_bShowPlayerTags;
    int   m_iShowPlayerMarkers;
    char  m_byteWorldHour = 12;
    char  m_byteWorldMinute = 0;
    char  m_byteWeather;
    float m_fGravity;
    bool  m_bLanMode;
    int   m_iDeathDropMoney;
    bool  m_bInstagib; // Always false
    int   m_iNetModeNormalOnfootSendRate;
    int   m_iNetModeNormalInCarSendRate;
    int   m_iNetModeFiringSendRate;
    int   m_iNetModeSendMultiplier;
    int   m_iLagCompensation;
    int   m_iVehicleFriendlyFire;
};

class SAMPNet
{
public:
    static void                     StartSAMP();
    static unsigned char            GetPacketID(Packet* pak);

                                    SAMPNet();

    void                            Connect(const char* ip = "127.0.0.1", unsigned short port = 7777, const char* password = NULL);
    void                            SetPlayerName(const char* name);
    void                            Update();
    void                            ProcessPools();
    void                            ProcessPackets();
    void                            Message(const char* format, ...);
    void                            ShutDownForGameRestart();
    void                            SendChatMessage(const char* msg);
    void                            SendChatCommand(const char* msg);
    void                            ResetVariables();
    void                            InitializeFromVars();

    inline void                     SetHostname(const char* name) { snprintf(m_szHostName, sizeof(m_szHostName), "%s", name); };
    inline RakClientInterface*      GetRakClient() { return m_pRakClient; };
    inline int                      GetGameState() { return m_nGameState; }
    inline void                     SetGameState(char gameState) { m_nGameState = gameState; }
    inline CVector2D*               GetWorldBorderMin() { return &m_vecWorldBorderMin; }
    inline CVector2D*               GetWorldBorderMax() { return &m_vecWorldBorderMax; }
    inline ServerVariables&         GetServerVars() { return m_vars; }
    
    static const char*              GetRejectedString(uint8_t rejectReason);

public:
    bool                            m_bStartedForConnecting;
    char                            m_szHostName[0xFF];
    char                            m_szIP[0x7F];
    char                            m_szPlayerName[24];
    char                            m_szCurrentPlayerName[24];
    unsigned short                  m_iPort;
    char                            m_nGameState;
    RakClientInterface*             m_pRakClient;
    CVector2D                       m_vecWorldBorderMin;
    CVector2D                       m_vecWorldBorderMax;

    ServerVariables                 m_vars;
};

extern SAMPNet* samp;

#endif // __SAMPNET_H
