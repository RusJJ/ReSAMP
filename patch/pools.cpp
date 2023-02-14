#include <mod/amlmod.h>
#include <mod/logger.h>
#include <samp.h>
#include <gtasa.h>


static CSAPool* AllocatePool(size_t count, size_t size)
{
    CSAPool *p = new CSAPool;
    p->objects = new char[size*count];
    p->flags = new uint8_t[count];
    p->count = count;
    p->top = 0xFFFFFFFF;
    p->initialized = 1;
    for (size_t i = 0; i < count; ++i)
    {
        p->flags[i] |= 0x80;
        p->flags[i] &= 0x80;
    }
    return p;
}

CSAPool* pPtrNodeSingleLinkPool;
CSAPool* pPtrNodeDoubleLinkPool;
CSAPool* pEntryInfoNodePool;
CSAPool* pPedPool;
CSAPool* pVehiclePool;
CSAPool* pBuildingPool;
CSAPool* pObjectPool;
CSAPool* pDummyPool;
CSAPool* pColModelPool;
CSAPool* pTaskPool;
CSAPool* pEventPool;
CSAPool* pPointRoutePool;
CSAPool* pPatrolRoutePool;
CSAPool* pNodeRoutePool;
CSAPool* pTaskAllocatorPool;
CSAPool* pPedIntelligencePool;
CSAPool* pPedAttractorPool;

void InitializePools()
{
    *(CSAPool**)(aml->GetSym(hGTASA, "_ZN6CPools25ms_pPtrNodeSingleLinkPoolE")) = (pPtrNodeSingleLinkPool = AllocatePool(100000, 8));
    *(CSAPool**)(aml->GetSym(hGTASA, "_ZN6CPools25ms_pPtrNodeDoubleLinkPoolE")) = (pPtrNodeDoubleLinkPool = AllocatePool(20000,  12));
    *(CSAPool**)(aml->GetSym(hGTASA, "_ZN6CPools21ms_pEntryInfoNodePoolE")) =     (pEntryInfoNodePool =     AllocatePool(20000,  20));
    *(CSAPool**)(aml->GetSym(hGTASA, "_ZN6CPools11ms_pPedPoolE")) =               (pPedPool =               AllocatePool(240,    1996));
    *(CSAPool**)(aml->GetSym(hGTASA, "_ZN6CPools15ms_pVehiclePoolE")) =           (pVehiclePool =           AllocatePool(2000,   2604));
    *(CSAPool**)(aml->GetSym(hGTASA, "_ZN6CPools16ms_pBuildingPoolE")) =          (pBuildingPool =          AllocatePool(POOL_BUILDINGS_COUNT, 60));
    *(CSAPool**)(aml->GetSym(hGTASA, "_ZN6CPools14ms_pObjectPoolE")) =            (pObjectPool =            AllocatePool(3000,   420));
    *(CSAPool**)(aml->GetSym(hGTASA, "_ZN6CPools13ms_pDummyPoolE")) =             (pDummyPool =             AllocatePool(POOL_DUMMIES_COUNT, 60));
    *(CSAPool**)(aml->GetSym(hGTASA, "_ZN6CPools16ms_pColModelPoolE")) =          (pColModelPool =          AllocatePool(50000,  48));
    *(CSAPool**)(aml->GetSym(hGTASA, "_ZN6CPools12ms_pTaskPoolE")) =              (pTaskPool =              AllocatePool(5000,   128));
    *(CSAPool**)(aml->GetSym(hGTASA, "_ZN6CPools13ms_pEventPoolE")) =             (pEventPool =             AllocatePool(1000,   68));
    *(CSAPool**)(aml->GetSym(hGTASA, "_ZN6CPools18ms_pPointRoutePoolE")) =        (pPointRoutePool =        AllocatePool(200,    100));
    *(CSAPool**)(aml->GetSym(hGTASA, "_ZN6CPools19ms_pPatrolRoutePoolE")) =       (pPatrolRoutePool =       AllocatePool(200,    420));
    *(CSAPool**)(aml->GetSym(hGTASA, "_ZN6CPools17ms_pNodeRoutePoolE")) =         (pNodeRoutePool =         AllocatePool(200,    36));
    *(CSAPool**)(aml->GetSym(hGTASA, "_ZN6CPools21ms_pTaskAllocatorPoolE")) =     (pTaskAllocatorPool =     AllocatePool(3000,   32));
    *(CSAPool**)(aml->GetSym(hGTASA, "_ZN6CPools23ms_pPedIntelligencePoolE")) =   (pPedIntelligencePool =   AllocatePool(240,    664));
    *(CSAPool**)(aml->GetSym(hGTASA, "_ZN6CPools20ms_pPedAttractorPoolE")) =      (pPedAttractorPool =      AllocatePool(200,    236));
}

void WorkWithPools()
{
    aml->Redirect(aml->GetSym(hGTASA, "_ZN6CPools10InitialiseEv"), (uintptr_t)InitializePools);
}
