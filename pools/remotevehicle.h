#ifndef __VEHICLE_H
#define __VEHICLE_H

#include <samp.h>

#define TRAIN_PASSENGER_LOCO			538
#define TRAIN_FREIGHT_LOCO				537
#define TRAIN_PASSENGER					570
#define TRAIN_FREIGHT					569
#define TRAIN_TRAM						449
#define HYDRA							520

class CVehicle;

class CRemoteVehicle
{
public:
    
    inline CVehicle* GetEntity() { return m_pEntity; }

    VehicleData data;
    
    int m_nID;
    int m_nGtaID;
    int m_nMarkerID;
    CVehicle* m_pEntity;
    bool m_bDoorsLocked;
    char m_szPlateName[16];
};

#endif // __VEHICLE_H
