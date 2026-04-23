// dllmain.cpp
// Injection noktasi: REVOLTELAUNCHER/GDIHelper.cpp InjectDLL()
//   CreateProcessA("KnightOnLine.exe") -> CreateRemoteThread(LoadLibraryA, "REVOLTEACS.dll")
//   -> Sleep(2000) -> ResumeThread()
//   -> DllMain(DLL_PROCESS_ATTACH) -> REVOLTEACSLoad() -> REVOLTEACSRemap()
#include "pch.h"

PacketHandler g_PacketHandler;
CUIManager g_UIManager;
CGameHooks g_GameHooks;

// ============================================================================
// RevLog — Console + C:\REVOLTEACS.log + OutputDebugStringA
// ============================================================================
static CRITICAL_SECTION g_LogCs;
static BOOL g_LogInit = FALSE;
static HANDLE g_hConsole = INVALID_HANDLE_VALUE;

static void InitConsole()
{
	AllocConsole();
	SetConsoleTitleA("REVOLTEACS Debug");
	g_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	// console buffer buyut — kayip olmasin
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (GetConsoleScreenBufferInfo(g_hConsole, &csbi))
	{
		COORD size = {csbi.dwSize.X, 3000};
		SetConsoleScreenBufferSize(g_hConsole, size);
	}
}

static void RevLog(const char *fmt, ...)
{
	if (!g_LogInit)
	{
		InitializeCriticalSection(&g_LogCs);
		g_LogInit = TRUE;
	}
	EnterCriticalSection(&g_LogCs);

	char msg[1024];
	SYSTEMTIME st;
	GetLocalTime(&st);
	int hdr = sprintf_s(msg, sizeof(msg),
						"[%02d:%02d:%02d.%03d T=%05lu] ",
						st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
						GetCurrentThreadId());

	va_list ap;
	va_start(ap, fmt);
	vsnprintf(msg + hdr, sizeof(msg) - hdr - 2, fmt, ap);
	va_end(ap);
	strcat_s(msg, sizeof(msg), "\n");

	// Console
	if (g_hConsole != INVALID_HANDLE_VALUE)
	{
		DWORD written;
		WriteConsoleA(g_hConsole, msg, (DWORD)strlen(msg), &written, NULL);
	}

	// Dosya
	FILE *fp = nullptr;
	if (fopen_s(&fp, "C:\\REVOLTEACS.log", "a") == 0 && fp)
	{
		fputs(msg, fp);
		fflush(fp);
		fclose(fp);
	}

	OutputDebugStringA(msg);
	LeaveCriticalSection(&g_LogCs);
}

// ============================================================================
// Remap + XIGNCODE bypass baslangici
// TODO: Section remap Themida RWX crash riski nedeniyle devre disi.
//       Yeni client adresler dogrulaninca RemapProcess buraya eklenecek.
// ============================================================================
// ============================================================================
// WritePatch — adrese byte dizisi yazar, VirtualProtect ile RWX yapar
// ============================================================================
static void WritePatch(DWORD addr, const BYTE *bytes, SIZE_T len)
{
	DWORD oldProt;
	if (!VirtualProtect((LPVOID)addr, len, PAGE_EXECUTE_READWRITE, &oldProt))
	{
		RevLog("patch: VirtualProtect failed @ 0x%08lX (err=%lu)", addr, GetLastError());
		return;
	}
	memcpy((void *)addr, bytes, len);
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
	BYTE patch1_success[] = {0xB8, 0x01, 0x00, 0x00, 0x00, 0xC3};
	WritePatch(0x00E73CD5, patch1_success, sizeof(patch1_success));
	RevLog("xigncode: [1] dispatcher init skip (0x00E73CD5 -> JMP 0x00E73D20)");

	// --- Patch 2: CRC32 integrity check bypass ---
	// 0x005753B9: sub_510F30 CRC32 hesapliyor, 0x005753C1'de 0D922F8F5h ile karsilastiriyor.
	// 0x005753C6: jnz loc_5753DD → CRC eslesmediyse basari path'ini atla.
	// 90 90 (NOP NOP): kosula bakilmaksizin her zaman basari path'ine gir.
	BYTE patch2[] = {0x90, 0x90};
	WritePatch(0x005753C6, patch2, sizeof(patch2));
	RevLog("xigncode: [2] CRC32 check NOP (0x005753C6)");

	// --- Patch 3: Runtime Polling Loop Bypass (Yeni Bulduğumuz) ---
	// 0x00459B64: jnz loc_459C41 -> Hata/Event tablosuna zıplatan komut.
	// Bu zıplamayı NOP'layarak (6 byte) oyunun hata yakaladığında
	// o bozuk Jump Table'a gitmesini engelliyoruz.
	// BYTE patch3[] = {0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
	BYTE patch3[] = {0xC3};
	WritePatch(0x00459B64, patch3, sizeof(patch3));
	RevLog("xigncode: [3] Runtime Polling Loop NOP (0x00459B64)");

	// Patch 4 (Revize): Fonksiyonu daha girmeden öldür
	// 0x00E11900 adresine RET koyuyoruz (Adresi IDA'dan teyit et, loc_A11960'ın üstündeki fonksiyon başı)
	BYTE retKill[] = {0xC3};
	WritePatch(0x00E11900, retKill, sizeof(retKill));
	RevLog("xigncode: [4] Critical Function 0x00E11900 KILLED before execution");
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

// Slot override yerine Detour hook — slot degeri degismez, XIGNCODE fark etmez
// F661D0 slot'unun gosterdigi adres runtime'da ogrenilip hook'laniyor.
typedef int(__stdcall *tXignCheck)(DWORD arg);
static tXignCheck oXignCheck = nullptr;

static int __stdcall hkXignCheck(DWORD arg)
{
	// Her cagriyi logla (ilk 20 kez)
	static int callCount = 0;
	if (callCount < 20)
	{
		RevLog("xign: check called arg=0x%08lX -> returning 1", arg);
		callCount++;
	}
	return 1; // success
}

DWORD WINAPI XignDispatcherWatchdog(LPVOID)
{
	RevLog("watchdog: started, waiting for F661D0 slot (Unpack Event)...");

	void *lastAddressInSlot = nullptr;
	bool patchesApplied = false;
	DWORD tick = 0;

	while (true)
	{
		__try
		{
			// 1. Slotun icindeki adresi kontrol et
			void **pSlot = (void **)0x00F661D0;
			void *curFuncAddr = *pSlot;

			// 2. UNPACK GERÇEKLEŞTİ Mİ?
			// Slot dolduysa (nullptr degilse) ve henüz biz müdahale etmediysek
			if (curFuncAddr != nullptr && curFuncAddr != (void *)hkXignCheck)
			{
				RevLog("watchdog: UNPACK DETECTED! F661D0 contains: %p", curFuncAddr);

				// --- A) POINTER SWAP (Sessiz Yönlendirme) ---
				// Orijinal fonksiyonu sakla (trambolin yerine)
				oXignCheck = (tXignCheck)curFuncAddr;

				DWORD oldProt;
				if (VirtualProtect(pSlot, sizeof(void *), PAGE_READWRITE, &oldProt))
				{
					// Slotun icindeki adresi bizim hook fonksiyonumuzla degistiriyoruz
					*pSlot = (void *)hkXignCheck;
					VirtualProtect(pSlot, sizeof(void *), oldProt, &oldProt);

					RevLog("watchdog: Pointer Swapped! Original: %p -> Hook: %p", oXignCheck, hkXignCheck);

					// --- B) GECİKMELİ YAMALAMA (Patching) ---
					// Kodlar artik acildigina gore CRC ve Dispatcher yamalarini yapabiliriz
					if (!patchesApplied)
					{
						PatchXigncode();
						patchesApplied = true;

						// YAMANIN GERÇEKTEN ORADA OLUP OLMADIĞINI KONTROL ET
						BYTE check[6];
						memcpy(check, (void *)0x00459B64, 6);
						RevLog("Verify Patch 3 at 0x00459B64: %02X %02X %02X %02X %02X %02X",
							   check[0], check[1], check[2], check[3], check[4], check[5]);
					}
				}
				else
				{
					RevLog("watchdog: VirtualProtect FAILED! Error: %lu", GetLastError());
				}
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			// Bellek sayfasi henüz hazir degilse (PAGE_NOACCESS gibi) buraya duser.
			// Sessizce beklemeye devam ediyoruz.
		}

		// Hayatta oldugumuzu logla (her 10 saniyede bir - 100ms * 100)
		if ((++tick % 100) == 0)
			RevLog("watchdog: alive, still waiting/monitoring...");

		Sleep(1); // 100ms bekleme (Themida'yi uyandirmamak icin ideal süre)
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
typedef HWND(WINAPI *tCreateWindowExA)(DWORD, LPCSTR, LPCSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID);
typedef HWND(WINAPI *tCreateWindowExW)(DWORD, LPCWSTR, LPCWSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID);

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
				if (!isalnum((unsigned char)lpCaption[i]))
				{
					allAlnum = false;
					break;
				}
			if (allAlnum)
				return true;
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
		   lpClass ? lpClass : "(null)",
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
	char classA[128] = "(null)";
	char captionA[256] = "(null)";
	if (lpClass)
		WideCharToMultiByte(CP_ACP, 0, lpClass, -1, classA, sizeof(classA), NULL, NULL);
	if (lpCaption)
		WideCharToMultiByte(CP_ACP, 0, lpCaption, -1, captionA, sizeof(captionA), NULL, NULL);

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
	if (!hUser32)
	{
		RevLog("hook: user32.dll not found");
		return;
	}

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
typedef int(WINAPI *tMessageBoxA)(HWND, LPCSTR, LPCSTR, UINT);
typedef int(WINAPI *tMessageBoxW)(HWND, LPCWSTR, LPCWSTR, UINT);

static tMessageBoxA oMessageBoxA = nullptr;
static tMessageBoxW oMessageBoxW = nullptr;

// Bos text + bos caption → launcher check dialog → otomatik cevap don, gostermeden gec
static int AutoReply(UINT uType)
{
	if (uType & MB_YESNO)
		return IDYES;
	if (uType & MB_OKCANCEL)
		return IDOK;
	return IDOK;
}

static bool IsEmptyDialog(LPCSTR lpText, LPCSTR lpCaption)
{
	bool textEmpty = (!lpText || lpText[0] == '\0');
	bool captionEmpty = (!lpCaption || lpCaption[0] == '\0');
	return textEmpty && captionEmpty;
}

static int WINAPI hkMessageBoxA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType)
{
	RevLog("MessageBoxA: caption='%s' text='%s' type=0x%X",
		   lpCaption ? lpCaption : "(null)",
		   lpText ? lpText : "(null)",
		   uType);

	void *retAddr = _ReturnAddress();

	// Detaylı loglama
	RevLog("MessageBoxA yakalandi!");
	RevLog("Cagiran Adres: %p", retAddr);
	RevLog("Baslik: %s | Metin: %s", lpCaption ? lpCaption : "null", lpText ? lpText : "null");

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
	char textA[512] = "(null)";
	char captionA[256] = "(null)";
	if (lpText)
		WideCharToMultiByte(CP_ACP, 0, lpText, -1, textA, sizeof(textA), NULL, NULL);
	if (lpCaption)
		WideCharToMultiByte(CP_ACP, 0, lpCaption, -1, captionA, sizeof(captionA), NULL, NULL);

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
	if (!hUser32)
		return;

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
typedef VOID(WINAPI *tExitProcess)(UINT);
typedef BOOL(WINAPI *tTerminateProcess)(HANDLE, UINT);

static tExitProcess oExitProcess = nullptr;
static tTerminateProcess oTerminateProcess = nullptr;

// Caller adresini oku (hook icinde esp+4 = return address)
static DWORD GetReturnAddr()
{
	DWORD ret = 0;
	__try
	{
		ret = ((DWORD *)_AddressOfReturnAddress())[0];
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}
	return ret;
}

static void PauseConsole(const char *reason, bool forceWait = false)
{
	if (g_hConsole == INVALID_HANDLE_VALUE)
		return;

	char buf[256];
	// "forceWait" true ise ENTER bekler, false ise sadece yazıp geçer
	if (forceWait)
	{
		sprintf_s(buf, "\n>>> %s -- devam etmek icin ENTER'a bas <<<\n", reason);
	}
	else
	{
		sprintf_s(buf, "\n>>> %s -- (Loglandi, devam ediliyor...) <<<\n", reason);
	}

	DWORD w;
	WriteConsoleA(g_hConsole, buf, (DWORD)strlen(buf), &w, NULL);

	// Eğer forceWait false ise, ENTER beklemeden hemen geri dön
	if (!forceWait)
	{
		return;
	}

	// YALNIZCA forceWait true olduğunda (Crash anı gibi) burası çalışır
	HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
	FlushConsoleInputBuffer(hIn);
	SetConsoleMode(hIn, ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT);
	char ch;
	DWORD rd;
	ReadConsoleA(hIn, &ch, 1, &rd, NULL);
}

static VOID WINAPI hkExitProcess(UINT uExitCode)
{
	RevLog("EXIT: ExitProcess(code=%u) T=%lu caller=0x%08lX",
		   uExitCode, GetCurrentThreadId(), GetReturnAddr());
	PauseConsole("ExitProcess");
	oExitProcess(uExitCode);
}

static BOOL WINAPI hkTerminateProcess(HANDLE hProc, UINT uExitCode)
{
	RevLog("EXIT: TerminateProcess(pid=%lu code=%u) T=%lu caller=0x%08lX",
		   GetProcessId(hProc), uExitCode, GetCurrentThreadId(), GetReturnAddr());
	if (GetProcessId(hProc) == GetCurrentProcessId())
		PauseConsole("TerminateProcess");
	return oTerminateProcess(hProc, uExitCode);
}

static LONG WINAPI VehCrashHandler(EXCEPTION_POINTERS *ep)
{
	DWORD code = ep->ExceptionRecord->ExceptionCode;

	// Themida'nın kasti hatalarını (0x96) ve Debugger kesmelerini (0x03) pas geç
	if (code == 0xC0000096 || code == 0x80000003)
		return EXCEPTION_CONTINUE_SEARCH;

	// Sadece GERÇEK Access Violation hatalarında dur
	if (code == 0xC0000005)
	{
		PEXCEPTION_RECORD er = ep->ExceptionRecord;
		PCONTEXT ctx = ep->ContextRecord;

		RevLog("========= !!! REAL CRASH DETECTED !!! =========");
		RevLog("Hata Kodu   : 0xC0000005 (Access Violation)");
		RevLog("Hata Adresi : 0x%p", er->ExceptionAddress);

		// 1. Hatanın Tipini Belirle (Okuma mı, Yazma mı?)
		uintptr_t accessType = er->ExceptionInformation[0]; // 0: Read, 1: Write, 8: DEP Violation
		uintptr_t faultAddr = er->ExceptionInformation[1];	// Erişilmeye çalışılan hatalı adres

		const char *typeStr = (accessType == 0) ? "READ" : (accessType == 1 ? "WRITE" : "EXECUTE (DEP)");
		RevLog("Islem Tipi  : %s", typeStr);
		RevLog("Hedef Adres : 0x%p", (void *)faultAddr);

		// 2. Register Durumlarını Dök (İşlemcinin o anki hali)
		RevLog("--- CPU REGISTERS ---");
		RevLog("EAX: %08X | EBX: %08X | ECX: %08X | EDX: %08X", ctx->Eax, ctx->Ebx, ctx->Ecx, ctx->Edx);
		RevLog("ESI: %08X | EDI: %08X | EBP: %08X | ESP: %08X", ctx->Esi, ctx->Edi, ctx->Ebp, ctx->Esp);
		RevLog("EIP: %08X (Hatalı komut konumu)", ctx->Eip);

		// 3. Crash Noktasındaki Byte'ları Oku (Disassembly için)
		unsigned char opcodes[16];
		if (ReadProcessMemory(GetCurrentProcess(), (LPCVOID)ctx->Eip, opcodes, 16, NULL))
		{
			char hexStr[64] = {0};
			for (int i = 0; i < 10; i++)
				sprintf(hexStr + strlen(hexStr), "%02X ", opcodes[i]);
			RevLog("Opcodes (EIP): %s", hexStr);
		}

		// 4. Hangi Modülde Patladı? (Oyun mu, DLL mi, XignCode mu?)
		HMODULE hMod;
		char modName[MAX_PATH];
		if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCSTR)er->ExceptionAddress, &hMod))
		{
			GetModuleFileNameA(hMod, modName, MAX_PATH);
			RevLog("Modul Ismi  : %s", modName);
		}
		else
		{
			RevLog("Modul Ismi  : BILINMIYOR (Dinamik bellek veya Shellcode)");
		}

		RevLog("===============================================");

		// Şimdi ENTER bekle ki biz bu verileri okuyabilelim
		PauseConsole("CRASH ANALIZI TAMAMLANDI", true);
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

static void HookExit()
{
	// HMODULE hK32 = GetModuleHandleA("kernel32.dll");
	// if (!hK32)
	// 	return;

	// oExitProcess = (tExitProcess)GetProcAddress(hK32, "ExitProcess");
	// oTerminateProcess = (tTerminateProcess)GetProcAddress(hK32, "TerminateProcess");

	// if (oExitProcess)
	// 	oExitProcess = (tExitProcess)DetourFunction((PBYTE)oExitProcess, (PBYTE)hkExitProcess);
	// if (oTerminateProcess)
	// 	oTerminateProcess = (tTerminateProcess)DetourFunction((PBYTE)oTerminateProcess, (PBYTE)hkTerminateProcess);

	// AddVectoredExceptionHandler(0, VehCrashHandler); // 0 = last handler

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

	HookCreateWindow();
	HookMessageBox();
	HookExit();
}

// ============================================================================
// Anti-AntiDebug
// PEB flag temizle + NtQueryInformationProcess / IsDebuggerPresent hookla
// ============================================================================
typedef NTSTATUS(NTAPI *tNtQIP)(HANDLE, UINT, PVOID, ULONG, PULONG);
typedef BOOL(WINAPI *tIsDbgPresent)();

static tNtQIP oNtQIP = nullptr;
static tIsDbgPresent oIsDbgPresent = nullptr;

static NTSTATUS NTAPI hkNtQIP(HANDLE hProc, UINT cls, PVOID info, ULONG len, PULONG retLen)
{
	NTSTATUS st = oNtQIP(hProc, cls, info, len, retLen);
	if (st >= 0)
	{
		if (cls == 7 || cls == 0x1E) // ProcessDebugPort / ProcessDebugObjectHandle
			*(DWORD *)info = 0;
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

	// RevLog("antidebug: all writes disabled (debug-observe mode)");
}

void REVOLTEACSRemap()
{
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId());
	if (!hProcess)
	{
		RevLog("remap: OpenProcess failed");
		return;
	}

	AntiAntiDebug();

	HANDLE hWatchdog = CreateThread(NULL, 0, XignDispatcherWatchdog, NULL, 0, NULL);
	if (hWatchdog)
	{
		RevLog("remap: watchdog started");
		CloseHandle(hWatchdog);
	}

	REVOLTEACSHook(hProcess);
	CloseHandle(hProcess);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		InitConsole();
		RevLog("REVOLTEACS loaded — PID=%lu", GetCurrentProcessId());
		REVOLTEACSRemap();
	}
	return TRUE;
}
