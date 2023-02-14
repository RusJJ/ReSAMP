#ifndef __SAPOOL_H
#define __SAPOOL_H

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

#endif // __SAPOOL_H
