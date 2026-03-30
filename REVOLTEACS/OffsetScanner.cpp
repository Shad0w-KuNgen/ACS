#include <windows.h>
#include <cstdio>
#include <cstring>
#include <cstdint>

#define KO_PTR_CHR 0x01092964
#define KO_PTR_DLG 0x01092A14

static bool g_ScanDone = false;

static bool SafeRead(void* addr, void* out, size_t sz) {
    __try { memcpy(out, addr, sz); return true; }
    __except(EXCEPTION_EXECUTE_HANDLER) { return false; }
}

void RunOffsetScan() {
    if (g_ScanDone) return;
    g_ScanDone = true;

    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    printf("[Scanner v4] Inventory Explorer\n");
    printf("50 saniye bekleniyor...\n");
    Sleep(50000);

    DWORD chrPtr = *(DWORD*)KO_PTR_CHR;
    DWORD dlgPtr = *(DWORD*)KO_PTR_DLG;
    if (!chrPtr) { Sleep(30000); chrPtr = *(DWORD*)KO_PTR_CHR; dlgPtr = *(DWORD*)KO_PTR_DLG; }
    if (!chrPtr) { printf("chrBase NULL!\n"); return; }

    BYTE* base = (BYTE*)chrPtr;
    BYTE* dlg = (BYTE*)dlgPtr;
    printf("chrBase=0x%08X dlgBase=0x%08X\n\n", chrPtr, dlgPtr);

    // Dogrulanmis offset'leri yazdir
    printf("=== DOGRULANMIS ===\n");
    printf("NAME=%s LV=%u HP=%d/%d MP=%d/%d\n",
        (char*)(base+0x694), *(uint8_t*)(base+0x6C0),
        *(int32_t*)(base+0x6C8), *(int32_t*)(base+0x6C4),
        *(int32_t*)(base+0xBC0), *(int32_t*)(base+0xBBC));
    printf("GOLD=%u ATK=%u DEF=%u ZONE=%u\n\n",
        *(uint32_t*)(base+0xBCC), *(uint32_t*)(base+0xC1C),
        *(uint32_t*)(base+0xC24), *(uint32_t*)(base+0xC60));

    // === INVENTORY EXPLORER ===
    printf("=== INVENTORY POINTER CHAIN ===\n");
    
    // DLG+0x494 = inventory UI pointer
    DWORD invPtr = *(DWORD*)(dlg + 0x494);
    printf("DLG+0x494 (InvPtr) = 0x%08X\n\n", invPtr);
    
    if (invPtr > 0x10000 && invPtr < 0x7FFFFFFF) {
        BYTE* inv = (BYTE*)invPtr;
        
        // Dump ilk 0x400 byte'i (pointer ve degerler)
        printf("--- InvPtr struct dump (u32, non-zero) ---\n");
        for (DWORD off = 0; off < 0x400; off += 4) {
            uint32_t v = 0;
            if (SafeRead(inv + off, &v, 4) && v != 0) {
                // Pointer mi deger mi?
                if (v > 0x10000 && v < 0x7FFFFFFF)
                    printf("  Inv+0x%03X = 0x%08X (ptr)\n", off, v);
                else if (v < 0x10000)
                    printf("  Inv+0x%03X = %u\n", off, v);
            }
        }
        
        // Ozellikle 0x260 ve 0x298 civarini detayli bak
        printf("\n--- Inv+0x250-0x2B0 detay ---\n");
        for (DWORD off = 0x250; off < 0x2B0; off += 4) {
            uint32_t v = 0;
            if (SafeRead(inv + off, &v, 4))
                printf("  Inv+0x%03X = 0x%08X (%u)\n", off, v, v);
        }
    }

    // === DLG POINTER CHAIN - tum UI pointer'lari tara ===
    printf("\n=== DLG UI POINTER CHAIN ===\n");
    // En cok referans alan DLG offset'leri
    DWORD dlgOffsets[] = {0x1D4, 0x1C4, 0x494, 0x1CC, 0x1C8, 0x200, 0x61C, 0x1F8, 0x66C, 0x45C, 0x1DC, 0x5A4, 0x3A8, 0x22C, 0x370, 0x348, 0x2EC};
    const char* dlgNames[] = {"GameMain", "GameBase", "Inventory", "SkillMgr", "ChrSelect", "Minimap", "ChatMgr", "TargetBar", "Merchant", "SkillBar", "CharInfo", "Exchange", "Quest", "Warehouse", "Party", "Knights", "Friend"};
    
    for (int i = 0; i < 17; i++) {
        DWORD ptr = *(DWORD*)(dlg + dlgOffsets[i]);
        printf("DLG+0x%03X (%s) = 0x%08X", dlgOffsets[i], dlgNames[i], ptr);
        if (ptr > 0x10000 && ptr < 0x7FFFFFFF) {
            // Bu pointer'in ilk birkac degerini oku
            BYTE* p = (BYTE*)ptr;
            uint32_t v0=0, v1=0, v2=0;
            SafeRead(p, &v0, 4);
            SafeRead(p+4, &v1, 4);
            SafeRead(p+8, &v2, 4);
            printf(" -> [%08X %08X %08X]", v0, v1, v2);
        }
        printf("\n");
    }

    // === SKILL BAR EXPLORER ===
    printf("\n=== SKILL BAR (DLG+0x45C) ===\n");
    DWORD sbarPtr = *(DWORD*)(dlg + 0x45C);
    if (sbarPtr > 0x10000 && sbarPtr < 0x7FFFFFFF) {
        BYTE* sbar = (BYTE*)sbarPtr;
        printf("SkillBar struct dump (u32, non-zero, 0x000-0x200):\n");
        for (DWORD off = 0; off < 0x200; off += 4) {
            uint32_t v = 0;
            if (SafeRead(sbar + off, &v, 4) && v != 0) {
                if (v > 100000 && v < 999999)
                    printf("  SBar+0x%03X = %u (SKILL ID?)\n", off, v);
                else if (v < 1000)
                    printf("  SBar+0x%03X = %u\n", off, v);
            }
        }
    }

    // === RECV FONKSIYON ADRESI BULMA ===
    printf("\n=== RECV HANDLER ANALIZI ===\n");
    
    // KO_PTR_RECV1 = 0x01092A28 (packet_tool loglarindan)
    DWORD recvPtr = *(DWORD*)0x01092A28;
    printf("KO_PTR_RECV1 (0x01092A28) -> 0x%08X\n", recvPtr);
    if (recvPtr > 0x10000 && recvPtr < 0x7FFFFFFF) {
        printf("  Recv fonksiyon ilk 32 byte: ");
        BYTE* fn = (BYTE*)recvPtr;
        for (int j = 0; j < 32; j++) printf("%02X ", fn[j]);
        printf("\n");
    }
    
    // 0x0082C7D0 fonksiyon baslangicini bul
    printf("\n--- 0x0082C7D0 (game recv) fonksiyon baslangici ---\n");
    for (int i = 0; i < 0x100; i++) {
        BYTE* p = (BYTE*)(0x0082C7D0 - i);
        if (p[0] == 0x55 && p[1] == 0x8B && p[2] == 0xEC) {
            printf("  Baslangic: 0x%08X\n  Bytes: ", (DWORD)p);
            for (int j = 0; j < 32; j++) printf("%02X ", p[j]);
            printf("\n");
            break;
        }
    }

    printf("\n=== SCAN TAMAMLANDI ===\n");
}

DWORD WINAPI ScanThread(LPVOID) { RunOffsetScan(); return 0; }

extern "C" BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(NULL, 0, ScanThread, NULL, 0, NULL);
    }
    return TRUE;
}
