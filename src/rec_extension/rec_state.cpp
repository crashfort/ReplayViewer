#include "rec_priv.h"

// Used to communicate with SVR.

// This is duplicated from SVR because these projects are too separated to reasonably share anything.
// No synchronization is needed because all access to this is from the main thread.
struct RecStudioSharedMem
{
    int32_t state;
    float velo[3];
    uint32_t buttons;
    char error[256];
};

HANDLE rec_svr_shared_mem_h;
RecStudioSharedMem* rec_svr_shared_ptr;

bool Rec_CreateSharedMem()
{
    bool ret = false;

    SECURITY_ATTRIBUTES sa = {};
    sa.nLength = sizeof(sa);

    auto mem_size = sizeof(RecStudioSharedMem);

    // Create shared memory handle a name since we have no way to communicate with SVR yet.
    rec_svr_shared_mem_h = CreateFileMappingA(INVALID_HANDLE_VALUE, &sa, PAGE_READWRITE, 0, mem_size, "SVR_RV_SHARED_MEM");

    if (rec_svr_shared_mem_h == NULL)
    {
        auto error = GetLastError();

        smutils->LogError(myself, "ERROR: Could not create svr shared memory (%u)\n", error);
        goto rfail;
    }

    rec_svr_shared_ptr = (RecStudioSharedMem*)MapViewOfFile(rec_svr_shared_mem_h, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);

    // This can't fail in this case, but check anyway I guess.
    if (rec_svr_shared_ptr == NULL)
    {
        auto error = GetLastError();

        smutils->LogError(myself, "ERROR: Could not view svr shared memory (%u)\n", error);
        goto rfail;
    }

    memset(rec_svr_shared_ptr, 0, mem_size); // Put to known state.

    smutils->LogMessage(myself, "Created SVR shared memory\n");

    ret = true;
    goto rexit;

    rfail:

    rexit:
    return ret;
}

bool Rec_Init()
{
    if (!Rec_CreateSharedMem())
    {
        return false;
    }

    return true;
}

void Rec_AllLoaded()
{
    extern sp_nativeinfo_t REC_NATIVES[];
    sharesys->AddNatives(myself, REC_NATIVES); // Add our own natives.
}

void Rec_Shutdown()
{
    if (rec_svr_shared_mem_h)
    {
        CloseHandle(rec_svr_shared_mem_h);
        rec_svr_shared_mem_h = NULL;
    }

    if (rec_svr_shared_ptr)
    {
        UnmapViewOfFile(rec_svr_shared_ptr);
        rec_svr_shared_ptr = NULL;
    }
}

cell_t Rec_MarkState(IPluginContext* context, const cell_t* params)
{
    // Safe write because all access is all on the same thread.
    rec_svr_shared_ptr->state = params[1];
    return 1;
}

cell_t Rec_ProvideData(IPluginContext* context, const cell_t* params)
{
    // Safe writes because all access is all on the same thread.

    cell_t* velo = NULL;
    cell_t buttons = params[2];

    context->LocalToPhysAddr(params[1], &velo);

    rec_svr_shared_ptr->velo[0] = sp_ctof(velo[0]);
    rec_svr_shared_ptr->velo[1] = sp_ctof(velo[1]);
    rec_svr_shared_ptr->velo[2] = sp_ctof(velo[2]);
    rec_svr_shared_ptr->buttons = buttons;
    return 1;
}

cell_t Rec_CreateDummyNavForMap(IPluginContext* context, const cell_t* params)
{
    const char* map = gamehelpers->GetCurrentMap();

    char* source_ptr;
    context->LocalToString(params[1], &source_ptr);

    char source_path_buf[PLATFORM_MAX_PATH];
    smutils->BuildPath(Path_Game, source_path_buf, REC_ARRAY_SIZE(source_path_buf), source_ptr);

    char dest_ptr[PLATFORM_MAX_PATH];
    REC_SNPRINTF(dest_ptr, "maps\\%s.nav", map);

    char dest_path_buf[PLATFORM_MAX_PATH];
    smutils->BuildPath(Path_Game, dest_path_buf, REC_ARRAY_SIZE(dest_path_buf), dest_ptr);

    CopyFileA(source_path_buf, dest_path_buf, TRUE); // Don't need to overwrite if something exists already.

    return 0;
}

sp_nativeinfo_t REC_NATIVES[] = {
    sp_nativeinfo_t { "Rec_MarkState", Rec_MarkState },
    sp_nativeinfo_t { "Rec_ProvideData", Rec_ProvideData },
    sp_nativeinfo_t { "Rec_CreateDummyNavForMap", Rec_CreateDummyNavForMap },
    sp_nativeinfo_t { NULL, NULL },
};
