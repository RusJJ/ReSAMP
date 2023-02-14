#ifndef __GTASA_H
#define __GTASA_H

#include <stdint.h>

#include "GTASA_STRUCTS.h"

#define PRE_STRINGIFY(x)                            #x
#define STRINGIFY(x)                                PRE_STRINGIFY(x)
#define CHECKSIZE(__cls,__size)                     static_assert(sizeof(__cls)==__size, "Validating size of " #__cls " is failed! " #__cls "'s size is not " #__size)

extern int g_nCurrentPlayerInFocus;
extern uintptr_t pGTASA;
extern void* hGTASA;



#define	VEHICLE_SUBTYPE_CAR				1
#define	VEHICLE_SUBTYPE_BIKE			2
#define	VEHICLE_SUBTYPE_HELI			3
#define	VEHICLE_SUBTYPE_BOAT			4
#define	VEHICLE_SUBTYPE_PLANE			5
#define	VEHICLE_SUBTYPE_PUSHBIKE		6
#define	VEHICLE_SUBTYPE_TRAIN			7

#define PHYSFLAGS_DISABLE_MOVE_FORCE    (1 << 5)
#define PHYSFLAGS_INFINITE_MASS         (1 << 6)
#define PHYSFLAGS_DISABLE_Z_MOVE        (1 << 7)




/*

extern "C" float sqrtf(float x);
extern "C" float copysignf(float x, float y);
struct CQuaternion
{
public:
    float w, x, y, z;
    CQuaternion() : w(1.0f), x(0.0f), y(0.0f), z(0.0f) {};
    CQuaternion(float X, float Y, float Z, float W) : x(X), y(Y), z(Z), w(W) {};

    inline CQuaternion& operator=(const CQuaternion& other)
    {
        if (this == &other) return *this;
        w = other.w;
        x = other.x;
        y = other.y;
        z = other.z;
        return *this;
    }

    inline void SetFromMatrix(CMatrix mat)
    {
        w = sqrtf( fmax0( 1.0f + mat.right.x + mat.up.y + mat.at.z ) ) * 0.5f;
        x = sqrtf( fmax0( 1.0f + mat.right.x - mat.up.y - mat.at.z ) ) * 0.5f;
        y = sqrtf( fmax0( 1.0f - mat.right.x + mat.up.y - mat.at.z ) ) * 0.5f;
        z = sqrtf( fmax0( 1.0f - mat.right.x - mat.up.y + mat.at.z ) ) * 0.5f;

        x = copysignf( x, mat.at.y - mat.up.z );
        y = copysignf( y, mat.right.z - mat.at.x );
        z = copysignf( z, mat.up.x - mat.right.y );
    }
    inline void SetFromMatrix(CMatrix& mat)
    {
        w = sqrtf( fmax0( 1.0f + mat.right.x + mat.up.y + mat.at.z ) ) * 0.5f;
        x = sqrtf( fmax0( 1.0f + mat.right.x - mat.up.y - mat.at.z ) ) * 0.5f;
        y = sqrtf( fmax0( 1.0f - mat.right.x + mat.up.y - mat.at.z ) ) * 0.5f;
        z = sqrtf( fmax0( 1.0f - mat.right.x - mat.up.y + mat.at.z ) ) * 0.5f;

        x = copysignf( x, mat.at.y - mat.up.z );
        y = copysignf( y, mat.right.z - mat.at.x );
        z = copysignf( z, mat.up.x - mat.right.y );
    }
    inline void SetFromMatrix(CMatrix* mat)
    {
        w = sqrtf( fmax0( 1.0f + mat->right.x + mat->up.y + mat->at.z ) ) * 0.5f;
        x = sqrtf( fmax0( 1.0f + mat->right.x - mat->up.y - mat->at.z ) ) * 0.5f;
        y = sqrtf( fmax0( 1.0f - mat->right.x + mat->up.y - mat->at.z ) ) * 0.5f;
        z = sqrtf( fmax0( 1.0f - mat->right.x - mat->up.y + mat->at.z ) ) * 0.5f;

        x = copysignf( x, mat->at.y - mat->up.z );
        y = copysignf( y, mat->right.z - mat->at.x );
        z = copysignf( z, mat->up.x - mat->right.y );
    }
    inline void GetMatrix(CMatrix* mat)
    {
        if(mat == NULL) return;
        
        float sqw = w * w;
        float sqx = x * x;
        float sqy = y * y;
        float sqz = z * z;

        mat->right.x = ( sqx - sqy - sqz + sqw);
        mat->up.y = (-sqx + sqy - sqz + sqw);
        mat->at.z = (-sqx - sqy + sqz + sqw);

        float tmp1 = x * y;
        float tmp2 = z * w;
        mat->up.x = 2.0 * (tmp1 + tmp2);
        mat->right.y = 2.0 * (tmp1 - tmp2);

        tmp1 = x*z;
        tmp2 = y*w;
        mat->at.x = 2.0 * (tmp1 - tmp2);
        mat->right.z = 2.0 * (tmp1 + tmp2);
        tmp1 = y*z;
        tmp2 = x*w;
        mat->at.y = 2.0 * (tmp1 + tmp2);
        mat->up.z = 2.0 * (tmp1 - tmp2);
    }
    inline void Normalize()
    {
        float n = sqrtf(x*x + y*y + z*z + w*w);
        w /= n;
        x /= n;
        y /= n;
        z /= n;
    }
};*/

#define POOL_BUILDINGS_COUNT 20000
#define POOL_DUMMIES_COUNT   40000

struct CSAPool
{
    void*     objects;
    uint8_t*  flags;
    uint32_t  count;
    uint32_t  top;
    uint32_t  initialized;
};

extern CSAPool* pPtrNodeSingleLinkPool;
extern CSAPool* pPtrNodeDoubleLinkPool;
extern CSAPool* pEntryInfoNodePool;
extern CSAPool* pPedPool;
extern CSAPool* pVehiclePool;
extern CSAPool* pBuildingPool;
extern CSAPool* pObjectPool;
extern CSAPool* pDummyPool;
extern CSAPool* pColModelPool;
extern CSAPool* pTaskPool;
extern CSAPool* pEventPool;
extern CSAPool* pPointRoutePool;
extern CSAPool* pPatrolRoutePool;
extern CSAPool* pNodeRoutePool;
extern CSAPool* pTaskAllocatorPool;
extern CSAPool* pPedIntelligencePool;
extern CSAPool* pPedAttractorPool;

#endif // __GTASA_H
