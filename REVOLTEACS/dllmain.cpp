// dllmain.cpp
// Injection noktasi: REVOLTELAUNCHER/GDIHelper.cpp InjectDLL()
//   CreateProcessA("KnightOnLine.exe") -> CreateRemoteThread(LoadLibraryA, "REVOLTEACS.dll")
//   -> Sleep(2000) -> ResumeThread()
//   -> DllMain(DLL_PROCESS_ATTACH) -> REVOLTEACSLoad() -> REVOLTEACSRemap()
#include "pch.h"
#define CONSOLE_MODE 0

PacketHandler g_PacketHandler;
CUIManager    g_UIManager;
CGameHooks    g_GameHooks;

// ============================================================================
// RevLog — C:\REVOLTEACS.log + OutputDebugStringA
// ============================================================================
static CRITICAL_SECTION g_LogCs;
static BOOL             g_LogInit = FALSE;

static void RevLog(const char* fmt, ...)
{
    if (!g_LogInit) {
        InitializeCriticalSection(&g_LogCs);
        g_LogInit = TRUE;
    }
    EnterCriticalSection(&g_LogCs);

    char msg[1024];
    SYSTEMTIME st; GetLocalTime(&st);
    int hdr = sprintf_s(msg, sizeof(msg),
        "[%02d:%02d:%02d.%03d T=%05lu] ",
        st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
        GetCurrentThreadId());

    va_list ap; va_start(ap, fmt);
    vsnprintf(msg + hdr, sizeof(msg) - hdr - 2, fmt, ap);
    va_end(ap);
    strcat_s(msg, sizeof(msg), "\n");

    FILE* fp = nullptr;
    if (fopen_s(&fp, "C:\\REVOLTEACS.log", "a") == 0 && fp) {
        fputs(msg, fp);
        fflush(fp);
        fclose(fp);
    }
    OutputDebugStringA(msg);

    LeaveCriticalSection(&g_LogCs);
}

// ============================================================================
// XIGNCODE Dispatcher Bypass
//
// dword_F661D0 @ 0x00F661D0  — ana XIGNCODE SDK function pointer slot'u.
// dword_F6654C @ 0x00F6654C  — TBL validation slot'u.
//
// XIGNCODE SDK init bu slotlara gercek check fn adresini yaziyor.
// Watchdog her 100ms'de ikisini de kontrol edip NopHandler'a geri yazar.
//
// NopHandler: mov eax, 1; ret 4  (__stdcall 1-arg)
//   eax=1 → TBL validation cagrilari non-zero bekliyor → gecer.
//   Protection check call-site'lari icin ayrica 0x0050BAD5 byte patch gerekiyor
//   (jz -> jmp, 0x74 -> 0xEB) — Themida unpack bitmeden yapilmamali.
// ============================================================================

static unsigned char g_NopHandler[] = {
    0xB8, 0x01, 0x00, 0x00, 0x00,  // mov eax, 1
    0xC2, 0x04, 0x00               // ret 4
};

struct SlotEntry {
    DWORD       addr;
    const char* name;
    void*       lastSeen;
};

DWORD WINAPI XignDispatcherWatchdog(LPVOID)
{
    // dword_F661D0 → 21 call site var, hepsi buradan gecuyor.
    // NopHandler stub'i yazarak tum XIGNCODE check'leri bypass ediyoruz.
    // Timing: XIGNCODE init ~400ms sonra, dialog ~69ms sonra geliyor.
    // 50ms interval + aninda override ile window'u yakalariz.
    SlotEntry slots[] = {
        { 0x00F661D0, "F661D0 (dispatcher)",   nullptr },
        { 0x00F661F4, "F661F4 (slot-2)",       nullptr },
        // { 0x00F6654C, "F6654C (tbl-validate)", nullptr },  // calling convention belirsiz, su an disable
    };

    RevLog("watchdog: started, stub=%p", (void*)g_NopHandler);

    DWORD tick = 0;
    while (true)
    {
        for (int i = 0; i < (int)ARRAYSIZE(slots); i++)
        {
            __try {
                void** slot = (void**)slots[i].addr;
                void*  cur  = *slot;

                if (cur != (void*)g_NopHandler)
                {
                    // Slot ilk doldugunda veya XIGNCODE yeniden yazdiginda aninda override et
                    if (cur != slots[i].lastSeen)
                        RevLog("watchdog: %s changed %p -> %p, overriding now", slots[i].name, slots[i].lastSeen, cur);

                    DWORD oldProt;
                    VirtualProtect(slot, sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProt);
                    *slot = (void*)g_NopHandler;
                    VirtualProtect(slot, sizeof(void*), oldProt, &oldProt);

                    slots[i].lastSeen = (void*)g_NopHandler;
                    RevLog("watchdog: %s overrode -> NopHandler", slots[i].name);
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                if ((tick % 100) == 0)
                    RevLog("watchdog: %s AV (code=%08lx)", slots[i].name, GetExceptionCode());
            }
        }

        if ((++tick % 200) == 0)
            RevLog("watchdog: alive tick=%lu", tick);
        Sleep(50);
    }
    return 0;
}


// ============================================================================
// CreateWindowExA/W hook — XIGNCODE pencere tespiti
//
// XIGNCODE her seferinde random caption urettigi icin string search calismaz.
// Tum window olusumlarini loglayip class + caption'i tespit ediyoruz.
// Tespit sonrasi: NULL dondurup pencereyi engelleyecegiz.
// ============================================================================
typedef HWND(WINAPI* tCreateWindowExA)(DWORD, LPCSTR,  LPCSTR,  DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID);
typedef HWND(WINAPI* tCreateWindowExW)(DWORD, LPCWSTR, LPCWSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID);

static tCreateWindowExA oCreateWindowExA = nullptr;
static tCreateWindowExW oCreateWindowExW = nullptr;

// XIGNCODE pencere tespiti icin kullanilan kriterler:
//   1. Class adi — log'dan ogrenince buraya yazilacak (TODO)
//   2. Caption uzunlugu — XIGNCODE random 40+ karakter uretir, oyun pencereleri oyle yapmaz
static bool IsXigncodeWindow(LPCSTR lpClass, LPCSTR lpCaption)
{
    // Kriter 1: class adi eslesmesi (log'dan ogrenince aktif edilecek)
    // if (lpClass && strcmp(lpClass, "BURAYA_CLASS_ADI") == 0) return true;

    // Kriter 2: caption 32+ karakter ve sadece alfanumerik → XIGNCODE random string
    if (lpCaption)
    {
        size_t len = strlen(lpCaption);
        if (len >= 32)
        {
            bool allAlnum = true;
            for (size_t i = 0; i < len; i++)
                if (!isalnum((unsigned char)lpCaption[i])) { allAlnum = false; break; }
            if (allAlnum) return true;
        }
    }
    return false;
}

static HWND WINAPI hkCreateWindowExA(
    DWORD dwExStyle, LPCSTR lpClass, LPCSTR lpCaption,
    DWORD dwStyle, int X, int Y, int W, int H,
    HWND hParent, HMENU hMenu, HINSTANCE hInst, LPVOID lpParam)
{
    RevLog("CreateWindowExA: class='%s' caption='%s' style=0x%08lX exstyle=0x%08lX",
        lpClass   ? lpClass   : "(null)",
        lpCaption ? lpCaption : "(null)",
        dwStyle, dwExStyle);

    if (IsXigncodeWindow(lpClass, lpCaption))
    {
        RevLog("xigncode: window BLOCKED (class='%s')", lpClass ? lpClass : "(null)");
        return NULL;
    }

    return oCreateWindowExA(dwExStyle, lpClass, lpCaption,
        dwStyle, X, Y, W, H, hParent, hMenu, hInst, lpParam);
}

static HWND WINAPI hkCreateWindowExW(
    DWORD dwExStyle, LPCWSTR lpClass, LPCWSTR lpCaption,
    DWORD dwStyle, int X, int Y, int W, int H,
    HWND hParent, HMENU hMenu, HINSTANCE hInst, LPVOID lpParam)
{
    char classA[128]   = "(null)";
    char captionA[256] = "(null)";
    if (lpClass)   WideCharToMultiByte(CP_ACP, 0, lpClass,   -1, classA,   sizeof(classA),   NULL, NULL);
    if (lpCaption) WideCharToMultiByte(CP_ACP, 0, lpCaption, -1, captionA, sizeof(captionA), NULL, NULL);

    RevLog("CreateWindowExW: class='%s' caption='%s' style=0x%08lX exstyle=0x%08lX",
        classA, captionA, dwStyle, dwExStyle);

    if (IsXigncodeWindow(classA, captionA))
    {
        RevLog("xigncode: window BLOCKED (class='%s')", classA);
        return NULL;
    }

    return oCreateWindowExW(dwExStyle, lpClass, lpCaption,
        dwStyle, X, Y, W, H, hParent, hMenu, hInst, lpParam);
}

static void HookCreateWindow()
{
    HMODULE hUser32 = GetModuleHandleA("user32.dll");
    if (!hUser32) { RevLog("hook: user32.dll not found"); return; }

    oCreateWindowExA = (tCreateWindowExA)GetProcAddress(hUser32, "CreateWindowExA");
    oCreateWindowExW = (tCreateWindowExW)GetProcAddress(hUser32, "CreateWindowExW");

    if (oCreateWindowExA)
        oCreateWindowExA = (tCreateWindowExA)DetourFunction((PBYTE)oCreateWindowExA, (PBYTE)hkCreateWindowExA);
    if (oCreateWindowExW)
        oCreateWindowExW = (tCreateWindowExW)DetourFunction((PBYTE)oCreateWindowExW, (PBYTE)hkCreateWindowExW);

    RevLog("hook: CreateWindowExA/W hooked");
}

// ============================================================================
// MessageBoxA/W hook — bos dialog tespiti
// Oyunun actigi dialog'larin text + tip bilgisini logla
// ============================================================================
typedef int(WINAPI* tMessageBoxA)(HWND, LPCSTR,  LPCSTR,  UINT);
typedef int(WINAPI* tMessageBoxW)(HWND, LPCWSTR, LPCWSTR, UINT);

static tMessageBoxA oMessageBoxA = nullptr;
static tMessageBoxW oMessageBoxW = nullptr;

// Bos text + bos caption → launcher check dialog → otomatik cevap don, gostermeden gec
static int AutoReply(UINT uType)
{
    if (uType & MB_YESNO)  return IDYES;
    if (uType & MB_OKCANCEL) return IDOK;
    return IDOK;
}

static bool IsEmptyDialog(LPCSTR lpText, LPCSTR lpCaption)
{
    bool textEmpty    = (!lpText    || lpText[0]    == '\0');
    bool captionEmpty = (!lpCaption || lpCaption[0] == '\0');
    return textEmpty && captionEmpty;
}

static int WINAPI hkMessageBoxA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType)
{
    RevLog("MessageBoxA: caption='%s' text='%s' type=0x%X",
        lpCaption ? lpCaption : "(null)",
        lpText    ? lpText    : "(null)",
        uType);

    if (IsEmptyDialog(lpText, lpCaption))
    {
        int reply = AutoReply(uType);
        RevLog("MessageBoxA: BLOCKED (empty dialog) -> reply=%d", reply);
        return reply;
    }

    return oMessageBoxA(hWnd, lpText, lpCaption, uType);
}

static int WINAPI hkMessageBoxW(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType)
{
    char textA[512]    = "(null)";
    char captionA[256] = "(null)";
    if (lpText)    WideCharToMultiByte(CP_ACP, 0, lpText,    -1, textA,    sizeof(textA),    NULL, NULL);
    if (lpCaption) WideCharToMultiByte(CP_ACP, 0, lpCaption, -1, captionA, sizeof(captionA), NULL, NULL);

    RevLog("MessageBoxW: caption='%s' text='%s' type=0x%X", captionA, textA, uType);

    if (IsEmptyDialog(lpText ? textA : nullptr, lpCaption ? captionA : nullptr))
    {
        int reply = AutoReply(uType);
        RevLog("MessageBoxW: BLOCKED (empty dialog) -> reply=%d", reply);
        return reply;
    }

    return oMessageBoxW(hWnd, lpText, lpCaption, uType);
}

static void HookMessageBox()
{
    HMODULE hUser32 = GetModuleHandleA("user32.dll");
    if (!hUser32) return;

    oMessageBoxA = (tMessageBoxA)GetProcAddress(hUser32, "MessageBoxA");
    oMessageBoxW = (tMessageBoxW)GetProcAddress(hUser32, "MessageBoxW");

    if (oMessageBoxA)
        oMessageBoxA = (tMessageBoxA)DetourFunction((PBYTE)oMessageBoxA, (PBYTE)hkMessageBoxA);
    if (oMessageBoxW)
        oMessageBoxW = (tMessageBoxW)DetourFunction((PBYTE)oMessageBoxW, (PBYTE)hkMessageBoxW);

    RevLog("hook: MessageBoxA/W hooked");
}

// ============================================================================
// ExitProcess / TerminateProcess / VEH — Kim kapatıyor tespiti
//
// ExitProcess    : normal cikis (ExitProcess(0) veya return from main)
// TerminateProcess : zorla kapatma (XIGNCODE, anti-cheat vs.)
// VEH            : unhandled exception / crash
// ============================================================================
typedef VOID  (WINAPI* tExitProcess)(UINT);
typedef BOOL  (WINAPI* tTerminateProcess)(HANDLE, UINT);

static tExitProcess       oExitProcess       = nullptr;
static tTerminateProcess  oTerminateProcess  = nullptr;

// Caller adresini oku (hook icinde esp+4 = return address)
static DWORD GetReturnAddr()
{
    DWORD ret = 0;
    __try { ret = ((DWORD*)_AddressOfReturnAddress())[0]; }
    __except (EXCEPTION_EXECUTE_HANDLER) {}
    return ret;
}

static VOID WINAPI hkExitProcess(UINT uExitCode)
{
    RevLog("EXIT: ExitProcess(code=%u) T=%lu caller=0x%08lX",
        uExitCode, GetCurrentThreadId(), GetReturnAddr());
    Sleep(100); // log flush icin
    oExitProcess(uExitCode);
}

static BOOL WINAPI hkTerminateProcess(HANDLE hProc, UINT uExitCode)
{
    RevLog("EXIT: TerminateProcess(pid=%lu code=%u) T=%lu caller=0x%08lX",
        GetProcessId(hProc), uExitCode, GetCurrentThreadId(), GetReturnAddr());
    Sleep(100);
    return oTerminateProcess(hProc, uExitCode);
}

// Unhandled exception / crash yakalayici
static LONG WINAPI VehCrashHandler(EXCEPTION_POINTERS* ep)
{
    EXCEPTION_RECORD* er = ep->ExceptionRecord;
    CONTEXT*          ctx = ep->ContextRecord;
    RevLog("CRASH: code=0x%08lX addr=0x%08lX EIP=0x%08lX ESP=0x%08lX EAX=0x%08lX T=%lu",
        er->ExceptionCode,
        (DWORD)er->ExceptionAddress,
        ctx->Eip, ctx->Esp, ctx->Eax,
        GetCurrentThreadId());
    Sleep(100);
    return EXCEPTION_CONTINUE_SEARCH; // normal crash flow'a devam
}

static void HookExit()
{
    HMODULE hK32 = GetModuleHandleA("kernel32.dll");
    if (!hK32) return;

    oExitProcess      = (tExitProcess)     GetProcAddress(hK32, "ExitProcess");
    oTerminateProcess = (tTerminateProcess)GetProcAddress(hK32, "TerminateProcess");

    if (oExitProcess)
        oExitProcess = (tExitProcess)DetourFunction((PBYTE)oExitProcess, (PBYTE)hkExitProcess);
    if (oTerminateProcess)
        oTerminateProcess = (tTerminateProcess)DetourFunction((PBYTE)oTerminateProcess, (PBYTE)hkTerminateProcess);

    AddVectoredExceptionHandler(0, VehCrashHandler); // 0 = last handler

    RevLog("hook: ExitProcess/TerminateProcess/VEH hooked");
}

// ============================================================================
// Hook kurulum noktasi
// TODO: Yeni client adresleri dogrulaninca asagidaki hook'lar tek tek acilacak.
// ============================================================================
void REVOLTEACSHook(HANDLE hProcess)
{
    // g_GameHooks.InitAllHooks(hProcess);  // EndGame 0x00E76BD9, Tick 0x006CE830
    // g_UIManager.Init();
    // g_PacketHandler.InitSendHook();      // 0x006FC190
    // g_PacketHandler.InitRecvHook();      // 0x0082C7D0
    // RenderSystem::Init();
    // Engine = new PearlEngine(); Engine->Init();

    HookExit();
    HookCreateWindow();
    HookMessageBox();
}

// ============================================================================
// Anti-AntiDebug
// PEB flag temizle + NtQueryInformationProcess / IsDebuggerPresent hookla
// ============================================================================
typedef NTSTATUS(NTAPI* tNtQIP)(HANDLE, UINT, PVOID, ULONG, PULONG);
typedef BOOL(WINAPI* tIsDbgPresent)();

static tNtQIP       oNtQIP          = nullptr;
static tIsDbgPresent oIsDbgPresent  = nullptr;

static NTSTATUS NTAPI hkNtQIP(HANDLE hProc, UINT cls, PVOID info, ULONG len, PULONG retLen)
{
    NTSTATUS st = oNtQIP(hProc, cls, info, len, retLen);
    if (st >= 0) {
        if (cls == 7 || cls == 0x1E)   // ProcessDebugPort / ProcessDebugObjectHandle
            *(DWORD*)info = 0;
    }
    return st;
}

static BOOL WINAPI hkIsDbgPresent() { return FALSE; }

static void AntiAntiDebug()
{
    // 1. PEB.BeingDebugged = 0
    // BYTE* peb = (BYTE*)__readfsdword(0x30);
    // peb[2] = 0;

    // 2. PEB.NtGlobalFlag heap debug bitleri temizle
    // *(DWORD*)(peb + 0x68) &= ~0x70u;

    // 3. NtQueryInformationProcess hook — ProcessDebugPort / ProcessDebugObjectHandle
    // HMODULE hNtdll = GetModuleHandleA("ntdll.dll");
    // if (hNtdll) {
    //     oNtQIP = (tNtQIP)GetProcAddress(hNtdll, "NtQueryInformationProcess");
    //     if (oNtQIP)
    //         oNtQIP = (tNtQIP)DetourFunction((PBYTE)oNtQIP, (PBYTE)hkNtQIP);
    // }

    // 4. IsDebuggerPresent hook
    // HMODULE hK32 = GetModuleHandleA("kernel32.dll");
    // if (hK32) {
    //     oIsDbgPresent = (tIsDbgPresent)GetProcAddress(hK32, "IsDebuggerPresent");
    //     if (oIsDbgPresent)
    //         oIsDbgPresent = (tIsDbgPresent)DetourFunction((PBYTE)oIsDbgPresent, (PBYTE)hkIsDbgPresent);
    // }

    RevLog("antidebug: all writes disabled (debug-observe mode)");
}

// ============================================================================
// Remap + XIGNCODE bypass baslangici
// TODO: Section remap Themida RWX crash riski nedeniyle devre disi.
//       Yeni client adresler dogrulaninca RemapProcess buraya eklenecek.
// ============================================================================
// ============================================================================
// WritePatch — adrese byte dizisi yazar, VirtualProtect ile RWX yapar
// ============================================================================
static void WritePatch(DWORD addr, const BYTE* bytes, SIZE_T len)
{
    DWORD oldProt;
    if (!VirtualProtect((LPVOID)addr, len, PAGE_EXECUTE_READWRITE, &oldProt)) {
        RevLog("patch: VirtualProtect failed @ 0x%08lX (err=%lu)", addr, GetLastError());
        return;
    }
    memcpy((void*)addr, bytes, len);
    VirtualProtect((LPVOID)addr, len, oldProt, &oldProt);
    RevLog("patch: wrote %zu bytes @ 0x%08lX", len, addr);
}

// ============================================================================
// PatchXigncode — XIGNCODE init bloğunu atla
//
// sub_E73C73 içinde loc_E73CD5 (0x00E73CD5) adresinden başlayan __try bloğu
// sub_E9263B'yi (XIGNCODE init) çağırıyor.
//
// Patch: 0x00E73CD5'e JMP 0x00E73D20 yaz → init çağrısını atla, başarı path'ine git.
//
// Offset hesabı:
//   Hedef (0x00E73D20) - (Patch adresi (0x00E73CD5) + 5) = 0x46
//   CPU: IP = 0x00E73CD5 + 5 = 0x00E73CDA, sonra +0x46 = 0x00E73D20
// ============================================================================
static void PatchXigncode()
{
    // --- Patch 1: XIGNCODE init dispatcher skip ---
    // sub_E73C73 / loc_E73CD5: __try blogu sub_E9263B'yi (XIGNCODE init) cagiriyor.
    // JMP ile atlayip basari path'ine (0x00E73D20) gidiyoruz.
    // Offset = 0x00E73D20 - (0x00E73CD5 + 5) = 0x46
    BYTE patch1[] = { 0xE9, 0x46, 0x00, 0x00, 0x00 };
    WritePatch(0x00E73CD5, patch1, sizeof(patch1));
    RevLog("xigncode: [1] dispatcher init skip (0x00E73CD5 -> JMP 0x00E73D20)");

    // --- Patch 2: CRC32 integrity check bypass ---
    // 0x005753B9: sub_510F30 CRC32 hesapliyor, 0x005753C1'de 0D922F8F5h ile karsilastiriyor.
    // 0x005753C6: jnz loc_5753DD → CRC eslesmediyse basari path'ini atla.
    // 90 90 (NOP NOP): kosula bakilmaksizin her zaman basari path'ine gir.
    BYTE patch2[] = { 0x90, 0x90 };
    WritePatch(0x005753C6, patch2, sizeof(patch2));
    RevLog("xigncode: [2] CRC32 check NOP (0x005753C6)");
}

void REVOLTEACSRemap()
{
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId());
    if (!hProcess) { RevLog("remap: OpenProcess failed"); return; }

    AntiAntiDebug();
    PatchXigncode();

    HANDLE hWatchdog = CreateThread(NULL, 0, XignDispatcherWatchdog, NULL, 0, NULL);
    if (hWatchdog) { RevLog("remap: watchdog started"); CloseHandle(hWatchdog); }

    REVOLTEACSHook(hProcess);
    CloseHandle(hProcess);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
#if CONSOLE_MODE
        AllocConsole();
        freopen(xorstr("CONOUT$"), xorstr("w"), stdout);
#endif
        // PID ekrani: Binary Ninja'yi attach et, sonra OK'e bas
        DWORD pid = GetCurrentProcessId();
        char pidMsg[128];
        sprintf_s(pidMsg, "KnightOnline.exe PID: %lu (0x%lX)\n\nBinary Ninja ile attach et, sonra OK'e bas.", pid, pid);
        MessageBoxA(NULL, pidMsg, "Attach Debugger", MB_ICONINFORMATION | MB_OK);
        REVOLTEACSRemap();
    }
    return TRUE;
}
