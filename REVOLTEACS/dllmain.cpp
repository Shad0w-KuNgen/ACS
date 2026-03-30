// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#define CONSOLE_MODE 1 // 0 OLUNCA CONSOL KAPALI - 1 OLUNCA KONSOL ACIK OLUYOR.
std::unordered_map<std::string, DWORD> m_mapAddress;

// --- Packet Hook --- PacketHandler sinifina tasindi
// Global PacketHandler instance - Send/Recv hook yonetimi
PacketHandler g_PacketHandler;

// Global UIManager instance - UI hook yonetimi
CUIManager g_UIManager;

// Global GameHooks instance - Tick, EndGame ve diger oyun hooklari
CGameHooks g_GameHooks;

DWORD GetAddress(std::string szAddressName) { return m_mapAddress[szAddressName]; }

void RemapArrayInsert(std::string szAddressName, DWORD iAddress = 0x00)
{
    if (iAddress == NULL)
        return;

    m_mapAddress.insert(std::make_pair(szAddressName, iAddress));
}

inline static DWORD Read4Byte(HANDLE hProcess, DWORD dwAddress)
{
    DWORD nValue = 0;
    ReadProcessMemory(hProcess, (LPVOID)dwAddress, &nValue, 4, 0);
    return nValue;
}

// Bu fonksiyon, thread'in i�inde �al��acak olan fonksiyon.
// Bu fonksiyon, thread'in i�inde �al��acak olan fonksiyon.
DWORD WINAPI RandomYazdir(LPVOID lpParam) {
	// Adres olarak al�nan DWORD de�erini adres de�i�kenine aktar.
	DWORD adres = reinterpret_cast<DWORD>(lpParam);

	// Rastgele say�lar� �retebilmek i�in adresi kullanarak zaman� ba�lat.
	srand(time(NULL) + adres);

	while (true) {
		// 0 ile 4294967295 (2^32 - 1) aras�nda rastgele bir say� olu�tur (4 byte).
		unsigned int randomSayi = rand();

		// Belirtilen adrese rastgele say�y� yazd�r.
		*(unsigned int*)adres = randomSayi;

		// Adresi ve i�eri�i hexadecimal olarak yazd�r.
		std::cout << "Adres: 0x" << std::hex << std::setw(8) << std::setfill('0') << adres << ", Icerik: 0x" << std::setw(8) << std::setfill('0') << randomSayi << "\n" << std::endl;

		// 10 saniye beklet.
		Sleep(10000);
	}
	return 0;
}


void REVOLTEACSRemapProcess(HANDLE hProcess)
{
	if (GetAddress(skCryptDec("KO_PATCH_ADDRESS1")) > 0)
	{
		Remap::PatchSection(
			hProcess,
			(LPVOID*)GetAddress(skCryptDec("KO_PATCH_ADDRESS1")),
			GetAddress(skCryptDec("KO_PATCH_ADDRESS1_SIZE")), PAGE_EXECUTE_READWRITE);
	}

	if (GetAddress(skCryptDec("KO_PATCH_ADDRESS2")) > 0)
	{
		Remap::PatchSection(
			hProcess,
			(LPVOID*)GetAddress(skCryptDec("KO_PATCH_ADDRESS2")),
			GetAddress(skCryptDec("KO_PATCH_ADDRESS2_SIZE")), PAGE_EXECUTE_READWRITE);
	}

	if (GetAddress(skCryptDec("KO_PATCH_ADDRESS3")) > 0)
	{
		Remap::PatchSection(
			hProcess,
			(LPVOID*)GetAddress(skCryptDec("KO_PATCH_ADDRESS3")),
			GetAddress(skCryptDec("KO_PATCH_ADDRESS3_SIZE")), PAGE_EXECUTE_READWRITE);
	}

	/*XINGCODE*/

	BYTE byPatch[] = { 0xE9, 0x7A, 0x06, 0x00, 0x00, 0x90, 0x90 };
	WriteProcessMemory(hProcess, (LPVOID*)0x00E73CD7, &byPatch, sizeof(byPatch), 0);

	/*00FC7644 | 4B | dec ebx |*/
	char buff[50];
	sprintf_s(buff, ("OPSGUARD[%d]"), GetCurrentProcessId());
	WriteProcessMemory(hProcess, (LPVOID*)0x00FCB66C, &buff, sizeof(buff), 0);

	// Adresler.
	DWORD adresler[] = { 0x01161AF4, 0x01161A5C, 0x01161648, 0x11619D4, 0x11619CC, 0x011619DC, 0x011619D8 };

	// Her adres i�in bir thread olu�tur.
	for (int i = 0; i < sizeof(adresler) / sizeof(adresler[0]); ++i)
	{
		HANDLE hThread = CreateThread(NULL, 0, RandomYazdir, (LPVOID)adresler[i], 0, NULL);
		if (hThread == NULL)
		{
			std::cerr << ("Thread olu�turulamad� ! Hata kodu : ") << GetLastError() << std::endl;
			return;
		}
		CloseHandle(hThread);
	}



	*(uint8_t*)0x006FCC93 = 0xEB;
	*(uint8_t*)0x0079FB0F = 0xEB;

	/*XIGNCODE*/
	*(uint8_t*)0x004F35DF = 0x25;
	*(uint8_t*)0x004F35F9 = 0x0B;

	*(uint8_t*)0x0079E44A = 0x29;
	*(uint8_t*)0x0079E464 = 0x0F;

	*(uint8_t*)0x0079DE63 = 0xC9;
	*(uint8_t*)0x0079DE81 = 0xAB;

	*(uint8_t*)0x006FDC92 = 0x2C;
	*(uint8_t*)0x006FDCAC = 0x12;

	*(uint8_t*)0x00795AA7 = 0x25;
	*(uint8_t*)0x00795AC1 = 0x0B;
}


//void REVOLTEACSRemapProcess(HANDLE hProcess)
//{
//	if (GetAddress(skCryptDec("KO_PATCH_ADDRESS1")) > 0)
//	{
//		Remap::PatchSection(
//			hProcess,
//			(LPVOID*)GetAddress(skCryptDec("KO_PATCH_ADDRESS1")),
//			GetAddress(skCryptDec("KO_PATCH_ADDRESS1_SIZE")), PAGE_EXECUTE_READWRITE);
//	}
//
//	if (GetAddress(skCryptDec("KO_PATCH_ADDRESS2")) > 0)
//	{
//		Remap::PatchSection(
//			hProcess,
//			(LPVOID*)GetAddress(skCryptDec("KO_PATCH_ADDRESS2")),
//			GetAddress(skCryptDec("KO_PATCH_ADDRESS2_SIZE")), PAGE_EXECUTE_READWRITE);
//	}
//
//	if (GetAddress(skCryptDec("KO_PATCH_ADDRESS3")) > 0)
//	{
//		Remap::PatchSection(
//			hProcess,
//			(LPVOID*)GetAddress(skCryptDec("KO_PATCH_ADDRESS3")),
//			GetAddress(skCryptDec("KO_PATCH_ADDRESS3_SIZE")), PAGE_EXECUTE_READWRITE);
//	}
//
//	/*XINGCODE*/
//	// Hedef bellekte yazmak istedi�iniz adresi belirtin (bu �rnekte varsay�lan bir adres kullan�lm��t�r)
//	LPVOID baseAddress = (LPVOID)0x00E73CD7; // De�i�tirin: hedef bellek adresi
//	BYTE byPatch[] = { 0xE9, 0x7A, 0x06, 0x00, 0x00, 0x90, 0x90 }; // Yazmak istedi�iniz veri
//	WriteProcessMemory(hProcess, baseAddress, &byPatch, sizeof(byPatch), 0);
//
//	char buff[21];
//	sprintf_s(buff, "OPSGUARD v2524[%d]", GetCurrentProcessId());
//	WriteProcessMemory(hProcess, (LPVOID*)0x00FCB66C, &buff, sizeof(buff), 0);
//
//	// Adresler.
//	DWORD adresler[] = { 0x01161AF4, 0x01161A5C, 0x01161648, 0x11619D4, 0x11619CC, 0x011619DC, 0x011619D8 };
//
//	// Her adres i�in bir thread olu�tur.
//	for (int i = 0; i < sizeof(adresler) / sizeof(adresler[0]); ++i) {
//		HANDLE hThread = CreateThread(NULL, 0, RandomYazdir, (LPVOID)adresler[i], 0, NULL);
//		if (hThread == NULL) {
//			std::cerr << "Thread olu�turulamad�! Hata kodu: " << GetLastError() << std::endl;
//			return;
//		}
//		CloseHandle(hThread);
//	}
//
//	/**(uint8_t*)0x006FCC93 = 0xEB;
//	*(uint8_t*)0x0079FB0F = 0xEB;*/
//
//	///*XIGNCODE*/
//	*(uint8_t*)0x005ADD48 = 0x25;
//	*(uint8_t*)0x005ADD62 = 0x0B;
//
//	*(uint8_t*)0x0079E44A = 0x29;
//	*(uint8_t*)0x0079E464 = 0x0F;
//
//	*(uint8_t*)0x004F35DF = 0x25;
//	*(uint8_t*)0x004F35F9 = 0x0B;
//
//	memcpy((void*)0x007955B6, (char*)"\x75\x2D\x90\x90\x90\x90", 6);
//	memcpy((void*)0x007955D4, (char*)"\x75\x0F\x90\x90\x90\x90", 6);
//
//	memcpy((void*)0x0079DE61, (char*)"\x75\x2D\x90\x90\x90\x90", 6);
//	memcpy((void*)0x0079DE7F, (char*)"\x75\x0F\x90\x90\x90\x90", 6);
//
//	*(uint8_t*)0x006FDC92 = 0x2C;
//	*(uint8_t*)0x006FDCAC = 0x12;
//
//	*(uint8_t*)0x00562B14 = 0x18;
//	*(uint8_t*)0x00562BE7 = 0x0B;
//
//	*(uint8_t*)0x00795AA8 = 0x25; // ��pheli
//	*(uint8_t*)0x00795AC2 = 0x0B;// ��pheli
//}

//void REVOLTEACSRemapProcess(HANDLE hProcess)
//{
//	if (GetAddress(skCryptDec("KO_PATCH_ADDRESS1")) > 0)
//	{
//		Remap::PatchSection(
//			hProcess,
//			(LPVOID*)GetAddress(skCryptDec("KO_PATCH_ADDRESS1")),
//			GetAddress(skCryptDec("KO_PATCH_ADDRESS1_SIZE")), PAGE_EXECUTE_READWRITE);
//	}
//
//	if (GetAddress(skCryptDec("KO_PATCH_ADDRESS2")) > 0)
//	{
//		Remap::PatchSection(
//			hProcess,
//			(LPVOID*)GetAddress(skCryptDec("KO_PATCH_ADDRESS2")),
//			GetAddress(skCryptDec("KO_PATCH_ADDRESS2_SIZE")), PAGE_EXECUTE_READWRITE);
//	}
//
//	if (GetAddress(skCryptDec("KO_PATCH_ADDRESS3")) > 0)
//	{
//		Remap::PatchSection(
//			hProcess,
//			(LPVOID*)GetAddress(skCryptDec("KO_PATCH_ADDRESS3")),
//			GetAddress(skCryptDec("KO_PATCH_ADDRESS3_SIZE")), PAGE_EXECUTE_READWRITE);
//	}
//
//	/*XINGCODE*/
//	memcpy((void*)0x00E73DF8, (char*)"\xE9\x4C\x03\x00\x00\x90\x90\x90\x90\x90", 10);
//	memcpy((void*)0x004F353D, (char*)"\xEB\x6B", 2);
//	/*memcpy((void*)0x0079DE61, (char*)"\xE9\x98\x00\x00\x00\x90", 6);
//	//memcpy((void*)0x00562BF3, (char*)"\x33\xC0", 2);
//	memcpy((void*)0x00562B52, (char*)"\xE9\x9C\x00\x00\x00\x90", 6);
//	/*memcpy((void*)0x0062FD55, (char*)"\xE9\x9B\x00\x00\x00\x90", 6);
//	memcpy((void*)0x0079DE61, (char*)"\xE9\x98\x00\x00\x00\x90", 6);
//	memcpy((void*)0x00562BF3, (char*)"\x33\xC0", 2);
//	memcpy((void*)0x00562BF5, (char*)"\xE9\x9E\x02\x00\x00\x90", 6);*/
//	//// Hedef bellekte yazmak istedi�iniz adresi belirtin (bu �rnekte varsay�lan bir adres kullan�lm��t�r)
//	//LPVOID baseAddress = (LPVOID)0x00E73CD7; // De�i�tirin: hedef bellek adresi
//	//BYTE byPatch[] = { 0xE9, 0x7A, 0x06, 0x00, 0x00, 0x90, 0x90 }; // Yazmak istedi�iniz veri
//	//WriteProcessMemory(hProcess, baseAddress, &byPatch, sizeof(byPatch), 0);
//
//	char buff[21];
//	sprintf_s(buff, "REVOLTEACS Client[%d]", GetCurrentProcessId());
//	WriteProcessMemory(hProcess, (LPVOID*)0x00FCB66C, &buff, sizeof(buff), 0);
//
//	//// Adresler.
//	//DWORD adresler[] = { 0x01161AF4, 0x01161A5C, 0x01161648, 0x11619D4, 0x11619CC, 0x011619DC, 0x011619D8 };
//
//	//// Her adres i�in bir thread olu�tur.
//	//for (int i = 0; i < sizeof(adresler) / sizeof(adresler[0]); ++i) {
//	//	HANDLE hThread = CreateThread(NULL, 0, RandomYazdir, (LPVOID)adresler[i], 0, NULL);
//	//	if (hThread == NULL) {
//	//		std::cerr << "Thread olu�turulamad�! Hata kodu: " << GetLastError() << std::endl;
//	//		return;
//	//	}
//	//	CloseHandle(hThread);
//	//}
//
//	///**(uint8_t*)0x006FCC93 = 0xEB;
//	//*(uint8_t*)0x0079FB0F = 0xEB;*/
//
//	/////*XIGNCODE*/
//	//*(uint8_t*)0x005ADD48 = 0x25;
//	//*(uint8_t*)0x005ADD62 = 0x0B;
//
//	//*(uint8_t*)0x0079E44A = 0x29;
//	//*(uint8_t*)0x0079E464 = 0x0F;
//
//	//*(uint8_t*)0x004F35DF = 0x25;
//	//*(uint8_t*)0x004F35F9 = 0x0B;
//
//	//memcpy((void*)0x007955B6, (char*)"\x75\x2D\x90\x90\x90\x90", 6);
//	//memcpy((void*)0x007955D4, (char*)"\x75\x0F\x90\x90\x90\x90", 6);
//
//	//memcpy((void*)0x0079DE61, (char*)"\x75\x2D\x90\x90\x90\x90", 6);
//	//memcpy((void*)0x0079DE7F, (char*)"\x75\x0F\x90\x90\x90\x90", 6);
//
//	//*(uint8_t*)0x006FDC92 = 0x2C;
//	//*(uint8_t*)0x006FDCAC = 0x12;
//
//	//*(uint8_t*)0x00562B14 = 0x18;
//	//*(uint8_t*)0x00562BE7 = 0x0B;
//
//	//*(uint8_t*)0x00795AA8 = 0x25;
//	//*(uint8_t*)0x00795AC2 = 0x0B;
//}

// =============================================================================
// OFFSET DOGRULAMA - Oyuna girdikten sonra tum offsetleri kontrol eder
// =============================================================================
DWORD WINAPI OffsetVerifyThread(LPVOID lpParam)
{
	printf("\n[OFFSET] 15 saniye bekleniyor (oyuna gir)...\n");
	Sleep(15000);

	printf("\n========== OFFSET DOGRULAMA ==========\n");

	DWORD chrBase = *(DWORD*)KO_PTR_CHR;
	printf("[PTR] KO_PTR_CHR (0x%08X) -> 0x%08X\n", KO_PTR_CHR, chrBase);

	if (chrBase == 0) {
		printf("[!] KO_PTR_CHR NULL - oyuna henuz girilmemis!\n");
		printf("[!] Oyuna gir, 30 sn sonra tekrar denenecek...\n");
		Sleep(30000);
		chrBase = *(DWORD*)KO_PTR_CHR;
		printf("[PTR] KO_PTR_CHR -> 0x%08X\n", chrBase);
	}

	if (chrBase == 0) {
		printf("[!] KO_PTR_CHR hala NULL, offset dogrulama iptal.\n");
		return 0;
	}

	DWORD dlgBase = *(DWORD*)KO_PTR_DLG;
	DWORD pktBase = *(DWORD*)KO_PTR_PKT;
	DWORD fldbBase = *(DWORD*)KO_FLDB;

	printf("[PTR] KO_PTR_DLG (0x%08X) -> 0x%08X\n", KO_PTR_DLG, dlgBase);
	printf("[PTR] KO_PTR_PKT (0x%08X) -> 0x%08X\n", KO_PTR_PKT, pktBase);
	printf("[PTR] KO_FLDB    (0x%08X) -> 0x%08X\n", KO_FLDB, fldbBase);

	printf("\n--- Oyuncu Verileri (chrBase: 0x%08X) ---\n", chrBase);

	// ID
	uint16_t id = *(uint16_t*)(chrBase + KO_OFF_ID);
	printf("[OFF] ID       (+0x%03X) = %d\n", KO_OFF_ID, id);

	// Name - string pointer okuma
	char* namePtr = (char*)(chrBase + KO_OFF_NAME);
	printf("[OFF] NAME     (+0x%03X) = %.20s\n", KO_OFF_NAME, namePtr);

	// Nation
	uint8_t nation = *(uint8_t*)(chrBase + KO_OFF_NATION);
	printf("[OFF] NATION   (+0x%03X) = %d (1=Karus, 2=ElMorad)\n", KO_OFF_NATION, nation);

	// Class
	short cls = *(short*)(chrBase + KO_OFF_CLASS);
	printf("[OFF] CLASS    (+0x%03X) = %d\n", KO_OFF_CLASS, cls);

	// Level
	uint8_t level = *(uint8_t*)(chrBase + KO_OFF_LEVEL);
	printf("[OFF] LEVEL    (+0x%03X) = %d\n", KO_OFF_LEVEL, level);

	// HP / MaxHP
	int32_t hp = *(int32_t*)(chrBase + KO_OFF_HP);
	int32_t maxhp = *(int32_t*)(chrBase + KO_OFF_MAXHP);
	printf("[OFF] HP       (+0x%03X) = %d\n", KO_OFF_HP, hp);
	printf("[OFF] MAXHP    (+0x%03X) = %d\n", KO_OFF_MAXHP, maxhp);

	// MP / MaxMP
	int32_t mp = *(int32_t*)(chrBase + KO_OFF_MP);
	int32_t maxmp = *(int32_t*)(chrBase + KO_OFF_MAXMP);
	printf("[OFF] MP       (+0x%03X) = %d\n", KO_OFF_MP, mp);
	printf("[OFF] MAXMP    (+0x%03X) = %d\n", KO_OFF_MAXMP, maxmp);

	// Attack
	uint16_t attack = *(uint16_t*)(chrBase + KO_OFF_ATTACK);
	printf("[OFF] ATTACK   (+0x%03X) = %d\n", KO_OFF_ATTACK, attack);

	// Gold
	uint32_t gold = *(uint32_t*)(chrBase + KO_OFF_GOLD);
	printf("[OFF] GOLD     (+0x%03X) = %u\n", KO_OFF_GOLD, gold);

	// Position
	float posX = *(float*)(chrBase + KO_OFF_X);
	float posY = *(float*)(chrBase + KO_OFF_Y);
	float posZ = *(float*)(chrBase + KO_OFF_Z);
	printf("[OFF] POS X    (+0x%03X) = %.2f\n", KO_OFF_X, posX);
	printf("[OFF] POS Y    (+0x%03X) = %.2f\n", KO_OFF_Y, posY);
	printf("[OFF] POS Z    (+0x%03X) = %.2f\n", KO_OFF_Z, posZ);

	// Target
	uint16_t target = *(uint16_t*)(chrBase + KO_OFF_TARGET);
	printf("[OFF] TARGET   (+0x%03X) = %d\n", KO_OFF_TARGET, target);

	printf("\n========== DOGRULAMA TAMAMLANDI ==========\n");
	printf("Yukaridaki degerleri oyundaki degerlerle karsilastir!\n");
	printf("Yanlis olan offsetler framework.h'de duzeltilecek.\n\n");

	// --- OFFSET SCANNER ---
	// Bilinen degerleri bellekte ara
	printf("\n========== OFFSET SCANNER ==========\n");
	printf("chrBase etrafinda bilinen degerleri ariyorum...\n\n");

	// HP arama (uint16 olarak)
	printf("--- uint16 olarak HP (4807 = 0x12C7) araniyor ---\n");
	for (int off = 0; off < 0x1500; off += 2) {
		uint16_t val = *(uint16_t*)(chrBase + off);
		if (val == 4807) {
			printf("  [BULUNDU] HP? offset +0x%03X = %d\n", off, val);
		}
	}

	// HP arama (uint32 olarak)
	printf("--- uint32 olarak HP araniyor ---\n");
	for (int off = 0; off < 0x1500; off += 4) {
		uint32_t val = *(uint32_t*)(chrBase + off);
		if (val == 4807) {
			printf("  [BULUNDU] HP? offset +0x%03X = %d\n", off, val);
		}
	}

	// Level arama - oyundaki seviyeni yaz
	printf("--- Level araniyor (1-83 arasi tum degerler) ---\n");
	for (int off = 0x600; off < 0xD00; off += 1) {
		uint8_t val = *(uint8_t*)(chrBase + off);
		// Sadece makul seviye degerleri (60-83 arasi, yuksek seviye karakterler icin)
		if (val >= 60 && val <= 83) {
			// Civarindaki degerler de mantikli mi kontrol et
			uint8_t prev = *(uint8_t*)(chrBase + off - 1);
			uint8_t next = *(uint8_t*)(chrBase + off + 1);
			if (prev < 200 && next < 200) {
				printf("  [ADAY] Level? offset +0x%03X = %d\n", off, val);
			}
		}
	}

	// String arama - karakter adi
	printf("--- String araniyor (ilk 20 karakter gosteriliyor) ---\n");
	for (int off = 0x600; off < 0xD00; off += 4) {
		char* ptr = (char*)(chrBase + off);
		// Okunabilir ASCII mi kontrol et
		bool readable = true;
		int len = 0;
		for (int i = 0; i < 16; i++) {
			if (ptr[i] == 0) { len = i; break; }
			if (ptr[i] < 32 || ptr[i] > 126) { readable = false; break; }
			len = i + 1;
		}
		if (readable && len >= 3 && len <= 16) {
			printf("  [STRING] offset +0x%03X = \"%.16s\" (len=%d)\n", off, ptr, len);
		}
	}

	// Nation arama (1 veya 2)
	printf("--- Nation araniyor (1=Karus, 2=ElMorad) ---\n");
	for (int off = 0x600; off < 0xD00; off += 1) {
		uint8_t val = *(uint8_t*)(chrBase + off);
		if (val == 1 || val == 2) {
			// Civarinda class (1-15) var mi?
			for (int d = -16; d <= 16; d += 4) {
				short cls2 = *(short*)(chrBase + off + d);
				if (cls2 >= 1 && cls2 <= 15 && d != 0) {
					printf("  [ADAY] Nation? +0x%03X=%d, Class? +0x%03X=%d\n",
						off, val, off + d, cls2);
				}
			}
		}
	}

	printf("\n========== SCANNER TAMAMLANDI ==========\n");
	printf("Sonuclari kontrol et ve framework.h'yi guncelle!\n\n");

	return 0;
}

void REVOLTEACSHook(HANDLE hProcess)
{
	// --- Game Hooks (EndGame, Tick, Login Intro, patches) ---
	g_GameHooks.InitAllHooks(hProcess);

	// --- UI Hook (UIManager) ---
	g_UIManager.Init();

	// --- Packet Send/Recv Hook (PacketHandler) ---
	g_PacketHandler.InitSendHook();
	g_PacketHandler.InitRecvHook();

	printf("[+] All hooks installed.\n");

	// --- RenderSystem — DX9 EndScene hook (ayri thread'de) ---
	CreateThread(NULL, 0, [](LPVOID) -> DWORD {
		Sleep(3000); // Oyun DX9 device'i olustursun diye 3sn bekle
		g_RenderSystem.Init();
		return 0;
	}, NULL, 0, NULL);

	// --- PearlEngine — merkezi engine baslatma ---
	Engine = new PearlEngine();
	Engine->Init();

	// --- Offset Dogrulama Thread ---
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)OffsetVerifyThread, NULL, 0, NULL);
}
void ExitSystem()
{
    ExitProcess(NULL);
    FreeLibrary(GetModuleHandle(NULL));
    TerminateProcess(GetCurrentProcess(), 0);
}

void REVOLTEACSRemap()
{
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId());
    if (hProcess == NULL) {
        MessageBoxA(NULL, "OPSGUARD Image Map ReadWrite Error\n", "OPSGUARD Security System", 0);
        ExitSystem();
    }
    RemapArrayInsert("KO_PATCH_ADDRESS1", 0x00400000);
    RemapArrayInsert("KO_PATCH_ADDRESS1_SIZE", 0x00B30000);
    RemapArrayInsert("KO_PATCH_ADDRESS2", 0x00F30000);
    RemapArrayInsert("KO_PATCH_ADDRESS2_SIZE", 0x00130000);
    RemapArrayInsert("KO_PATCH_ADDRESS3", 0x01060000);
    RemapArrayInsert("KO_PATCH_ADDRESS3_SIZE", 0x00010000);
	REVOLTEACSRemapProcess(hProcess);
	REVOLTEACSHook(hProcess);
}

void REVOLTEACSLoad()
{
#if CONSOLE_MODE 1
    AllocConsole();
    freopen(xorstr("CONOUT$"), xorstr("w"), stdout);
#endif

	// packet_tool.dll varsa yukle
	HMODULE hPacketTool = LoadLibraryA("packet_tool.dll");
	if (hPacketTool) {
		printf("[+] packet_tool.dll yuklendi!\n");
	}

	REVOLTEACSRemap();
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        REVOLTEACSLoad();
        break;
    }
    return TRUE;
}

