#ifndef __SAMPNET_SYNCDATA_H
#define __SAMPNET_SYNCDATA_H

#include <gtasa.h>

#pragma pack(push, 1)

struct PLAYER_SPAWN_INFO
{
    uint8_t     byteTeam;
    int         iSkin;
    uint8_t     unk;
    CVector     vecPos;
    float       fRotation;
    int         iSpawnWeapons[3];
    int         iSpawnWeaponsAmmo[3];
};

struct ONFOOT_SYNC_DATA
{
    uint16_t    lrAnalog;
    uint16_t    udAnalog;
    uint16_t    wKeys;
    CVector     vecPos;
    CQuaternion quat;
    uint8_t     byteHealth;
    uint8_t     byteArmour;
    uint8_t     byteCurrentKeyAndWeapon;
    uint8_t     byteSpecialAction;
    CVector     vecMoveSpeed;
    CVector     vecSurfOffsets;
    uint16_t    wSurfInfo;
    uint32_t    dwAnimation;
};

struct INCAR_SYNC_DATA
{
    uint16_t    VehicleID;
    uint16_t    lrAnalog;
    uint16_t    udAnalog;
    uint16_t    wKeys;
    CQuaternion quat;
    CVector     vecPos;
    CVector     vecMoveSpeed;
    float       fCarHealth;
    uint8_t     bytePlayerHealth;
    uint8_t     bytePlayerArmour;
    uint8_t     byteCurrentKeyAndWeapon;
    uint8_t     byteSirenOn;
    uint8_t     byteLandingGearState;
    uint16_t    TrailerID;
    float       fTrainSpeed;
};

struct PASSENGER_SYNC_DATA
{
    uint16_t    VehicleID;
    uint8_t     byteSeatFlags : 7;
    uint8_t     byteDriveBy : 1;
    uint8_t     byteCurrentKeyAndWeapon;
    uint8_t     bytePlayerHealth;
    uint8_t     bytePlayerArmour;
    uint16_t    lrAnalog;
    uint16_t    udAnalog;
    uint16_t    wKeys;
    CVector     vecPos;
};

struct AIM_SYNC_DATA
{
    uint8_t	    byteCamMode;
    CVector     vecAimf;
    CVector     vecAimPos;
    float       fAimZ;
    uint8_t     byteCamExtZoom : 6;
    uint8_t     byteWeaponState : 2;
    uint8_t     aspect_ratio;
};

struct BULLET_SYNC_DATA
{
    uint8_t     byteHitType;
    uint16_t    PlayerID;
    CVector     vecOrigin;
    CVector     vecPos;
    CVector     vecOffset;
    uint8_t     byteWeaponID;
};

struct SPECTATOR_SYNC_DATA
{
    uint16_t    lrAnalog;
    uint16_t    udAnalog;
    uint16_t    wKeys;
    CVector     vecPos;
};

struct TRAILER_SYNC_DATA
{
    CQuaternion quat;
    CVector     vecPos;
    CVector     vecMoveSpeed;
};

#pragma pack(pop)

#endif // __SAMPNET_SYNCDATA_H
