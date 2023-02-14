#include <mod/amlmod.h>
#include <samp.h>
#include <gtasa.h>

// Reallocated
CPlayerInfo* g_pPlayersContainer;
int g_nCurrentPlayerInFocus; int* pVarPlayerInFocus = &g_nCurrentPlayerInFocus;
CBaseModelInfo* g_pAtomicModelsInfo;
CPedModelInfo* g_pAtomicPedModelsInfo;

// Got directly from the game library
int* g_pnAddedAtomicModels;
int* g_pnAddedPedModels;
CBaseModelInfo** ms_modelInfoPtrs;
void* (*MatrixLinkListInit)(uintptr_t matrixList, unsigned int count);
uintptr_t gMatrixList;

int FindPlayerSlotWithPedPointer(void* playerPtr)
{
    for(int i = 0; i < MAX_PLAYERS; ++i)
    {
        if((void*)(g_pPlayersContainer[i].m_pPed) == playerPtr)
            return i;
    }
    return -1;
}

CBaseModelInfo* AddAtomicModel(int modelId)
{
    int nCount = *g_pnAddedAtomicModels;
    ++*g_pnAddedAtomicModels;
    CBaseModelInfo* modelInfo = &g_pAtomicModelsInfo[nCount];
    ((void(*)(CBaseModelInfo*))(*(uintptr_t*)(modelInfo->vtable + 0x1C)))(modelInfo);
    ms_modelInfoPtrs[modelId] = modelInfo;
    return modelInfo;
}

CBaseModelInfo* AddPedModel(int modelId)
{
    int nCount = *g_pnAddedPedModels;
    ++*g_pnAddedPedModels;
    CBaseModelInfo* modelInfo = &g_pAtomicPedModelsInfo[nCount];
    ((void(*)(CBaseModelInfo*))(*(uintptr_t*)(modelInfo->vtable + 0x1C)))(modelInfo);
    ms_modelInfoPtrs[modelId] = modelInfo;
    return modelInfo;
}

void* InitMatrixArray()
{
    return MatrixLinkListInit(gMatrixList, 10000);
}

void PatchTheGame()
{
    // CWorld::Players
    g_pPlayersContainer = new CPlayerInfo[MAX_PLAYERS] {0};
    aml->Write(pGTASA + 0x6783C8, (uintptr_t)&g_pPlayersContainer, sizeof(void*));

    // CWorld::PlayerInFocus (by default this variable is in range of [-128; 127])
    aml->Write(pGTASA + 0x679B5C, (uintptr_t)&pVarPlayerInFocus, sizeof(void*));

    // CWorld::FindPlayerSlotWithPedPointer
    aml->Redirect(aml->GetSym(hGTASA, "_ZN6CWorld28FindPlayerSlotWithPedPointerEPv"), (uintptr_t)FindPlayerSlotWithPedPointer);

    // Atomic Models
    g_pAtomicModelsInfo = new CBaseModelInfo[ATOMIC_MODELS_INFO];
    void (*CBaseModelInfoInitializer)(CBaseModelInfo*);
    SET_TO(CBaseModelInfoInitializer, aml->GetSym(hGTASA, "_ZN14CBaseModelInfoC2Ev"));
    uintptr_t CBaseModelInfoVTable = pGTASA + 0x667454;
    for(int i = 0; i < ATOMIC_MODELS_INFO; ++i)
    {
        CBaseModelInfoInitializer(&g_pAtomicModelsInfo[i]);
        g_pAtomicModelsInfo[i].vtable = CBaseModelInfoVTable;
    }
    SET_TO(ms_modelInfoPtrs, aml->GetSym(hGTASA, "_ZN10CModelInfo16ms_modelInfoPtrsE"));
    SET_TO(g_pnAddedAtomicModels, pGTASA + 0x820738);
    aml->Redirect(aml->GetSym(hGTASA, "_ZN10CModelInfo14AddAtomicModelEi"), (uintptr_t)AddAtomicModel);

    // Atomic Ped Models
    g_pAtomicPedModelsInfo = new CPedModelInfo[ATOMIC_PEDMODELS_INFO];
    memset(g_pAtomicPedModelsInfo, 0, sizeof(CPedModelInfo) * ATOMIC_PEDMODELS_INFO);
    uintptr_t CBasePedModelInfoVTable = pGTASA + 0x667668;
    for(int i = 0; i < ATOMIC_PEDMODELS_INFO; ++i)
    {
        CBaseModelInfoInitializer(&g_pAtomicPedModelsInfo[i]);
        g_pAtomicPedModelsInfo[i].vtable = CBasePedModelInfoVTable;
    }
    SET_TO(g_pnAddedPedModels, pGTASA + 0x915FC8);
    aml->Redirect(aml->GetSym(hGTASA, "_ZN10CModelInfo11AddPedModelEi"), (uintptr_t)AddPedModel);

    // Vehicle Colors patch

    // Disable ped models talking
    aml->Write(aml->GetSym(hGTASA, "_ZN23CAEPedSpeechAudioEntity20s_bAllSpeechDisabledE"), (uintptr_t)"\x01", 1);

    // Save file name
    aml->Write(aml->GetSym(hGTASA, "DefaultPCSaveFileName") + 5, (uintptr_t)"MP", 2);

    // CTheZones::*
    memset((void*)(pGTASA + 0x98D252), 1, 100);
    *(int*)(pGTASA + 0x98D2B8) = 100;

    // Matrix for CPlaceable
    gMatrixList = aml->GetSym(hGTASA, "gMatrixList");
    SET_TO(MatrixLinkListInit, aml->GetSym(hGTASA, "_ZN15CMatrixLinkList4InitEi"));
    aml->Redirect(aml->GetSym(hGTASA, "_ZN10CPlaceable15InitMatrixArrayEv"), (uintptr_t)InitMatrixArray);

    // CClothes::RebuildPlayer
    //aml->PlaceRET(aml->GetSym(hGTASA, "_ZN8CClothes13RebuildPlayerEP10CPlayerPedb") & ~0x1);

    // CPlayerInfo::FindObjectToSteal
    aml->PlaceRET(aml->GetSym(hGTASA, "_ZN11CPlayerInfo17FindObjectToStealEP4CPed") & ~0x1);

    // CCarCtrl::GenerateRandomCars
    aml->PlaceRET(aml->GetSym(hGTASA, "_ZN8CCarCtrl18GenerateRandomCarsEv") & ~0x1);

    // CTheCarGenerators::Process
    aml->PlaceRET(aml->GetSym(hGTASA, "_ZN17CTheCarGenerators7ProcessEv") & ~0x1);

    // CPopulation::AddPed
    aml->PlaceRET(aml->GetSym(hGTASA, "_ZN11CPopulation6AddPedE8ePedTypejRK7CVectorb") & ~0x1);

    // CEntryExit::GenerateAmbientPeds
    aml->PlaceRET(aml->GetSym(hGTASA, "_ZN10CEntryExit19GenerateAmbientPedsERK7CVector") & ~0x1);
}

extern "C" void VehiclePool_Reconstruct()
{
    uintptr_t m_pInfoPool = aml->GetSym(hGTASA, "_ZN17CVehicleModelInfo17CVehicleStructure11m_pInfoPoolE");
    char* pool = new char[0x14]; char* poolData = new char[50];
    *(char**)(pool + 0) = new char[812 * MAX_VEHICLES_IN_POOL];
    *(char**)(pool + 4) = poolData;
    *(int*)(pool + 8) = MAX_VEHICLES_IN_POOL;
    *(int*)(pool + 12) = -1;
    *(bool*)(pool + 16) = true;

    *(char*)(poolData + 0) = 0x80;
    *(char*)(poolData + 1) = 0x80;

    *(char*)(*(int*)(pool + 4) + 1) &= 0x80u;
    for (int i = 2; i != MAX_VEHICLES_IN_POOL; ++i)
    {
        *(char*)(*(int*)(pool + 4) + i) |= 0x80u;
        *(char*)(*(int*)(pool + 4) + i) &= 0x80u;
    }
    *(char**)m_pInfoPool = pool;
}

__attribute__((optnone)) __attribute__((naked)) void VehiclePool_Reconstruct_stub(void)
{
    asm("PUSH {R0-R11}");

    asm("BL VehiclePool_Reconstruct");
    asm volatile("MOV R12, %0\n" :: "r"(pGTASA + 0x468BD8 + 0x1));

    asm("POP {R0-R11}");
    asm("BX R12");
}

void DoPoolsPatches()
{
    aml->Redirect(pGTASA + 0x468B76 + 0x1, (uintptr_t)VehiclePool_Reconstruct_stub);
}
