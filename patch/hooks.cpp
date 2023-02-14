#include <mod/amlmod.h>
#include <mod/logger.h>
#include <samp.h>
#include <gtasa.h>
#include <stdio.h>

#include <game/game.h>
#include <game/localplayer.h>

// Saves

// Functions
void (*AddImageToList)(const char* imgName, bool notPlayerImg);
void* (*LoadTextureDB)(const char* dbFile, bool fullLoad, int txdbFormat);
void* (*GetTextureDB)(const char* dbFile);
void* (*RegisterTextureDB)(void* db);
void* (*UnregisterTextureDB)(void* db);
void* (*GetTextureFromTextureDB)(const char* tex);
void* (*RwTexDictionaryGetCurrent)();
void* (*RwTexDictionaryFindNamedTexture)(void*, const char*);
int* texStorageVar1;
char** texStorageVar2;
int* texStorageVar3;

DECL_HOOK(void*, OS_FileOpen, int dataArea, void** data, const char *path, int accessType)
{
    const char* newPath = path;

         if(!strncmp(path, "data\\script\\mainV1.scm", 0x16))   newPath = "SAMP\\main.scm";
    else if(!strncmp(path, "DATA\\SCRIPT\\SCRIPTV1.IMG", 0x18)) newPath = "SAMP\\script.scm";
    else if(!strncmp(path, "DATA\\GTA.DAT", 0xC))               newPath = "SAMP\\gta.dat";
    else if(!strncmp(path, "DATA\\PEDS.IDE", 0xD))              newPath = "SAMP\\peds.ide";
    else if(!strncmp(path, "DATA\\TIMECYC.DAT", 0x10))          newPath = "SAMP\\timecyc.dat";
    else if(!strncmp(path, "data\\paths\\tracks2.dat", 0x15))   newPath = "SAMP\\tracks2.dat";
    else if(!strncmp(path, "data\\paths\\tracks4.dat", 0x15))   newPath = "SAMP\\tracks4.dat";

    return OS_FileOpen(dataArea, data, newPath, accessType);
}
void InitImageList()
{
	aml->Unprot(pGTASA + 0x792D78, 0x180);

  	*(uint8_t *) (pGTASA + 0x792DA8) = 0;
  	*(uint32_t *)(pGTASA + 0x792DA4) = 0;
  	*(uint32_t *)(pGTASA + 0x792DD4) = 0;
  	*(uint8_t *) (pGTASA + 0x792DD8) = 0;
  	*(uint32_t *)(pGTASA + 0x792E04) = 0;
  	*(uint8_t *) (pGTASA + 0x792E08) = 0;
  	*(uint32_t *)(pGTASA + 0x792E34) = 0;
  	*(uint8_t *) (pGTASA + 0x792E38) = 0;
  	*(uint32_t *)(pGTASA + 0x792E64) = 0;
  	*(uint8_t *) (pGTASA + 0x792E68) = 0;
  	*(uint32_t *)(pGTASA + 0x792E94) = 0;
  	*(uint8_t *) (pGTASA + 0x792E98) = 0;
  	*(uint32_t *)(pGTASA + 0x792EC4) = 0;
  	*(uint8_t *) (pGTASA + 0x792EC8) = 0;
  	*(uint32_t *)(pGTASA + 0x792EF4) = 0;
  	*(uint8_t *) (pGTASA + 0x792D78) = 0;
    AddImageToList("TEXDB\\SAMPCOL.IMG", true);
    AddImageToList("TEXDB\\SAMP.IMG", true);
    AddImageToList("TEXDB\\GTA3.IMG", true);
    AddImageToList("TEXDB\\GTA_INT.IMG", true);
}

DECL_HOOKv(InitialiseRenderWare)
{
    InitialiseRenderWare();
    void* db = NULL;
    if((db = LoadTextureDB("samp", false, 5)) == NULL)
    {
        logger->Info("Failed to load texture DB \"samp\"!");
    }
    else
    {
        RegisterTextureDB(db);
        logger->Info("Texture DB \"samp\" has been loaded!");
    }
}

#define TRY_TO_GET(__dbName)       /*logger->Info("Trying to get %s from 0x%X = " #__dbName, tex, __dbName);*/ RegisterTextureDB(__dbName); texture = GetTextureFromTextureDB(tex); UnregisterTextureDB(__dbName); if(texture != NULL) { return texture; }
void* TxdStoreFindCB(const char* tex)
{
    int i;
    void* texture = NULL;
    void* dbInt = GetTextureDB("gta_int"), *db3 = GetTextureDB("gta3"), *dbSAMP = GetTextureDB("samp");
    if(dbSAMP == NULL) dbSAMP = LoadTextureDB("samp", false, 5);

    TRY_TO_GET(dbInt);
    TRY_TO_GET(db3);
    TRY_TO_GET(dbSAMP);

  LAST_CHANCE:
    void* curDictionary = RwTexDictionaryGetCurrent();
    if(curDictionary == NULL) return NULL;
    do
    {
        texture = RwTexDictionaryFindNamedTexture(curDictionary, tex);
        if(texture != NULL) return texture;
    }
    while((curDictionary = *(void**)((int)curDictionary + *texStorageVar3)) != NULL);
    return texture;
}

DECL_HOOK(void*, LoadObjectInstance, uintptr_t obj, const char* a1)
{
    if(Game::IsBuildingRemoved(*(int*)(obj + 28), *(float*)(obj + 0), *(float*)(obj + 4), *(float*)(obj + 8)))
    {
        *(int*)(obj + 28) = 19300; // blankmodel.dff
    }
    return LoadObjectInstance(obj, a1);
}
 // Late hooks
DECL_HOOKv(DrawAllWidgets, bool a1)
{
    if(!Game::m_bDisableWidgets)
    {
        DrawAllWidgets(a1);
    }
}
DECL_HOOKv(PlayerInfoProcess, CPlayerInfo* self, int pedId)
{
    Game::m_nProcessedInPlayerInfo = pedId;
    PlayerInfoProcess(self, pedId);
    Game::m_nProcessedInPlayerInfo = -1;
}
DECL_HOOK(void*, Task_LeaveCar, void* self, CVehicle* pVehicle, int a1, int a2, bool a3, bool a4)
{
    if(Game::m_nProcessedInPlayerInfo == 0)
    {
        //if(CLocalPlayer::GetEntity()->m_pVehicle == pVehicle)
        {
            // Notify exit
            RakNet::BitStream bsSend;
            bsSend.Write(CLocalPlayer::m_nLastSAMPVehId);
	        samp->GetRakClient()->RPC((int*)&RPC_ExitVehicle, &bsSend, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0,false, UNASSIGNED_NETWORK_ID, nullptr);
        }
    }
    return Task_LeaveCar(self, pVehicle, a1, a2, a3, a4);
}
uintptr_t EnterCarAsDriver_BackTo, EnterCarAsDriver_BackToContinue, EnterCarAsDriver_BackToAbort;
extern "C" void EnterCarAsDriver_patch(CVehicle* pVehicle)
{
    // m_pVehicle is null at this moment
    //if(CLocalPlayer::GetEntity()->m_pVehicle == pVehicle)
    if(Game::m_nProcessedInPlayerInfo == 0)
    {
        uint16_t vehId = Game::GetSAMPVehID(pVehicle);
        logger->Info("veh 0x%04X", vehId);
        if(vehId == 0xFFFF || Game::m_pVehiclePool->GetAt(vehId)->m_bDoorsLocked)
        {
            logger->Info("aborted");
            EnterCarAsDriver_BackTo = EnterCarAsDriver_BackToAbort;
            return;
        }
        
        RakNet::BitStream bsSend;
        bsSend.Write(vehId);
        bsSend.Write((uint8_t)false); // is passenger
	    samp->GetRakClient()->RPC((int*)&RPC_EnterVehicle, &bsSend, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0,false, UNASSIGNED_NETWORK_ID, nullptr);
    }

    EnterCarAsDriver_BackTo = EnterCarAsDriver_BackToContinue;
}
__attribute__((optnone)) __attribute__((naked)) void EnterCarAsDriver_inject()
{
    asm volatile(
        "MOV R0, R8\n"
        "BL EnterCarAsDriver_patch\n"
        "PUSH {R0}\n");
    asm volatile(
        "MOV R12, %0\n"
        "POP {R0}\n"
        "LDR R0, [R4]\n"
        "LDR R6, [R0, #0x440]\n"
        "MOVS R0, #0x50\n"
        "BX R12\n"
    :: "r" (EnterCarAsDriver_BackTo));
}

void HookFunctions()
{
    // IMG
    SET_TO(AddImageToList, aml->GetSym(hGTASA, "_ZN10CStreaming14AddImageToListEPKcb"));
    HOOKPLT(OS_FileOpen, pGTASA + 0x6753A4); // Because CLEO already hooking this.
    aml->Redirect(aml->GetSym(hGTASA, "_ZN10CStreaming13InitImageListEv"), (uintptr_t)InitImageList);

    // TXD
    SET_TO(LoadTextureDB, aml->GetSym(hGTASA, "_ZN22TextureDatabaseRuntime4LoadEPKcb21TextureDatabaseFormat"));
    SET_TO(GetTextureDB, aml->GetSym(hGTASA, "_ZN22TextureDatabaseRuntime11GetDatabaseEPKc"));
    SET_TO(RegisterTextureDB, aml->GetSym(hGTASA, "_ZN22TextureDatabaseRuntime8RegisterEPS_"));
    SET_TO(UnregisterTextureDB, aml->GetSym(hGTASA, "_ZN22TextureDatabaseRuntime10UnregisterEPS_"));
    SET_TO(GetTextureFromTextureDB, aml->GetSym(hGTASA, "_ZN22TextureDatabaseRuntime10GetTextureEPKc"));
    SET_TO(RwTexDictionaryGetCurrent, aml->GetSym(hGTASA, "_Z25RwTexDictionaryGetCurrentv"));
    SET_TO(RwTexDictionaryFindNamedTexture, aml->GetSym(hGTASA, "_Z31RwTexDictionaryFindNamedTextureP15RwTexDictionaryPKc"));
    SET_TO(texStorageVar1, pGTASA + 0x6BD178);
    SET_TO(texStorageVar2, pGTASA + 0x6BD17C);
    SET_TO(texStorageVar3, pGTASA + 0xA83F5C);
    HOOKPLT(InitialiseRenderWare, pGTASA + 0x66F2D0);
    aml->Redirect(aml->GetSym(hGTASA, "_ZN9CTxdStore14TxdStoreFindCBEPKc"), (uintptr_t)TxdStoreFindCB);

    // Other things
    HOOKPLT(LoadObjectInstance, pGTASA + 0x675E6C);
    aml->PlaceRET(aml->GetSym(hGTASA, "_ZN11CPopulation25GeneratePedsAtStartOfGameEv"));
    
    
}

void HookFunctionsLate()
{
    // UI HAX
    HOOK(DrawAllWidgets, aml->GetSym(hGTASA, "_ZN15CTouchInterface7DrawAllEb"));
    
    // Car thingies?
    HOOK(PlayerInfoProcess, aml->GetSym(hGTASA, "_ZN11CPlayerInfo7ProcessEi"));
    HOOK(Task_LeaveCar, aml->GetSym(hGTASA, "_ZN20CTaskComplexLeaveCarC2EP8CVehicleiibb"));
    aml->Redirect(pGTASA + 0x40AC20 + 0x1, (uintptr_t)EnterCarAsDriver_inject);
    EnterCarAsDriver_BackToContinue = pGTASA + 0x40AC28 + 0x1;
    EnterCarAsDriver_BackToAbort = pGTASA + 0x4095F6 + 0x1;
}
