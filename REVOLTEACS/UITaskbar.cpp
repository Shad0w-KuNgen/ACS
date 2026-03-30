#include "pch.h"
#include "UITaskbar.h"

// =============================================================================
// UITaskbar — Taskbar ReceiveMessage Hook Implementasyonu
// Referans: Pearl Guard 2369 UITaskbarMain.cpp / UITaskbarSub.cpp
// =============================================================================

// Global UIManager referansi (dllmain.cpp'de tanimli)
extern CUIManager g_UIManager;
extern PearlEngine* Engine;

// =============================================================================
// TaskbarMain — Static hook degiskenleri (file-scope, __asm icin)
// =============================================================================
static DWORD s_uiTaskbarMainVTable = 0;
static DWORD s_uiTaskbarMainOrigFunc = 0;

// =============================================================================
// TaskbarSub — Static hook degiskenleri
// =============================================================================
static DWORD s_uiTaskbarSubVTable = 0;
static DWORD s_uiTaskbarSubOrigFunc = 0;

// Forward declarations — hook fonksiyonlari constructor'dan once tanimlanmali
void __stdcall UITaskbarMainReceiveMessage_Hook(DWORD* pSender, uint32_t dwMsg);
void __stdcall UITaskbarSubReceiveMessage_Hook(DWORD* pSender, uint32_t dwMsg);

// =============================================================================
// CUITaskbarMain — Constructor
// =============================================================================
CUITaskbarMain::CUITaskbarMain()
    : m_dVTableAddr(0)
    , m_baseTaskBar(0), m_btn00Stand(0), m_btn01Sit(0), m_btn02Seek(0)
    , m_btn03Trade(0), m_btn04Skill(0), m_btn05Character(0), m_btn06Inventory(0)
    , m_baseMenu(0), m_btnMenu(0), m_btnRank(0)
{
    DWORD dlgBase = *(DWORD*)KO_PTR_DLG;
    if (dlgBase == 0)
    {
        printf("[UITaskbarMain] UYARI: KO_PTR_DLG NULL, hook atlaniyor\n");
        return;
    }

    m_dVTableAddr = *(DWORD*)(dlgBase + KO_OFF_DLG_TASKBAR_MAIN);
    if (m_dVTableAddr == 0)
    {
        printf("[UITaskbarMain] UYARI: vTable adresi 0, hook atlaniyor (offset: 0x%03X)\n", KO_OFF_DLG_TASKBAR_MAIN);
        return;
    }

    printf("[UITaskbarMain] vTable: 0x%08X (DLG+0x%03X)\n", m_dVTableAddr, KO_OFF_DLG_TASKBAR_MAIN);

    ParseUIElements();
    InitReceiveMessage();
}

CUITaskbarMain::~CUITaskbarMain() {}

// =============================================================================
// CUITaskbarMain::ParseUIElements — GetChildByID ile buton pointer'larini coz
// =============================================================================
void CUITaskbarMain::ParseUIElements()
{
    if (m_dVTableAddr == 0) return;

    // base_TaskBar container
    m_baseTaskBar = g_UIManager.GetChildByID(m_dVTableAddr, "base_TaskBar");
    if (m_baseTaskBar)
    {
        m_btn00Stand     = g_UIManager.GetChildByID(m_baseTaskBar, "btn_00");
        m_btn01Sit       = g_UIManager.GetChildByID(m_baseTaskBar, "btn_01");
        m_btn02Seek      = g_UIManager.GetChildByID(m_baseTaskBar, "btn_02");
        m_btn03Trade     = g_UIManager.GetChildByID(m_baseTaskBar, "btn_03");
        m_btn04Skill     = g_UIManager.GetChildByID(m_baseTaskBar, "btn_04");
        m_btn05Character = g_UIManager.GetChildByID(m_baseTaskBar, "btn_05");
        m_btn06Inventory = g_UIManager.GetChildByID(m_baseTaskBar, "btn_06");
    }
    else
    {
        printf("[UITaskbarMain] UYARI: base_TaskBar bulunamadi\n");
    }

    // base_menu container
    m_baseMenu = g_UIManager.GetChildByID(m_dVTableAddr, "base_menu");
    if (m_baseMenu)
    {
        m_btnMenu = g_UIManager.GetChildByID(m_baseMenu, "btn_menu");
        m_btnRank = g_UIManager.GetChildByID(m_baseMenu, "btn_rank");
    }
    else
    {
        printf("[UITaskbarMain] UYARI: base_menu bulunamadi\n");
    }

    printf("[UITaskbarMain] ParseUIElements tamamlandi — btn_00:0x%08X btn_06:0x%08X\n",
        m_btn00Stand, m_btn06Inventory);

    // btn_powerup TaskbarMain'de de olabilir — engelle
    DWORD btnPowerupMain = g_UIManager.GetChildByID(m_dVTableAddr, "btn_powerup");
    if (btnPowerupMain != 0)
    {
        RegisterButtonHandler(btnPowerupMain, []() -> bool {
            printf("[UITaskbarMain] PUS (btn_powerup) engellendi\n");
            return true;
        });
    }
}

// =============================================================================
// UITaskbarMainReceiveMessage_Hook — Global __stdcall hook fonksiyonu
// vTable pointer swap ile ReceiveMessage yerine cagirilir
// =============================================================================
void __stdcall UITaskbarMainReceiveMessage_Hook(DWORD* pSender, uint32_t dwMsg)
{
    // Ozel handler'lari kontrol et
    if (Engine && Engine->m_UITaskbarMain)
    {
        bool handled = Engine->m_UITaskbarMain->ReceiveMessage(pSender, dwMsg);
        if (handled)
            return; // Engellendi, orijinal cagirilmaz
    }

    // Orijinal fonksiyonu __thiscall olarak cagir
    __asm
    {
        MOV ECX, s_uiTaskbarMainVTable
        PUSH dwMsg
        PUSH pSender
        MOV EAX, s_uiTaskbarMainOrigFunc
        CALL EAX
    }
}

// =============================================================================
// CUITaskbarMain::InitReceiveMessage — vTable hook kurulumu
// =============================================================================
void CUITaskbarMain::InitReceiveMessage()
{
    if (m_dVTableAddr == 0) return;

    // GetRecvMessagePtr: (*(DWORD*)vTableAddr) + 0x7C (25xx ReceiveMessage offset)
    DWORD ptrMsg = (*(DWORD*)m_dVTableAddr) + 0x7C;
    if (ptrMsg == 0x7C) // vTable[0] sifir ise
    {
        printf("[UITaskbarMain] HATA: vTable[0] sifir, hook kurulamadi\n");
        return;
    }

    // Orijinal fonksiyon pointer'ini kaydet
    s_uiTaskbarMainVTable = m_dVTableAddr;
    s_uiTaskbarMainOrigFunc = *(DWORD*)ptrMsg;

    printf("[UITaskbarMain] Hook oncesi: ptrMsg=0x%08X, *ptrMsg=0x%08X\n", ptrMsg, s_uiTaskbarMainOrigFunc);

    // VirtualProtect ile yazma izni al — vTable read-only olabilir
    DWORD oldProtect = 0;
    BOOL vpResult = VirtualProtect((LPVOID)ptrMsg, sizeof(DWORD), PAGE_EXECUTE_READWRITE, &oldProtect);
    if (!vpResult)
    {
        printf("[UITaskbarMain] UYARI: VirtualProtect basarisiz (err: %d), dogrudan yazma deneniyor\n", GetLastError());
    }

    // Hook fonksiyonuyla degistir
    *(DWORD*)ptrMsg = (DWORD)UITaskbarMainReceiveMessage_Hook;

    // Eski koruma geri yukle
    if (vpResult)
    {
        DWORD tmp;
        VirtualProtect((LPVOID)ptrMsg, sizeof(DWORD), oldProtect, &tmp);
    }

    // Dogrulama — gercekten degisti mi?
    DWORD verify = *(DWORD*)ptrMsg;
    printf("[UITaskbarMain] Hook sonrasi: *ptrMsg=0x%08X (beklenen: 0x%08X) %s\n",
        verify, (DWORD)UITaskbarMainReceiveMessage_Hook,
        (verify == (DWORD)UITaskbarMainReceiveMessage_Hook) ? "OK" : "BASARISIZ");

    printf("[UITaskbarMain] ReceiveMessage hook kuruldu (orig: 0x%08X)\n", s_uiTaskbarMainOrigFunc);
}

// =============================================================================
// CUITaskbarMain::ReceiveMessage — Handler dispatch
// =============================================================================
bool CUITaskbarMain::ReceiveMessage(DWORD* pSender, uint32_t dwMsg)
{
    if (!pSender)
        return false;

    DWORD senderAddr = (DWORD)pSender;

    if (dwMsg == UIMSG_BUTTON_CLICK)
    {
        char btnName[64] = {0};
        extern bool ReadStdString(DWORD base, DWORD offset, char* outBuf, int maxLen);
        if (ReadStdString(senderAddr, 0x054, btnName, 64))
        {
            printf("[TaskbarMain] CLICK: sender=0x%08X ID=\"%s\"\n", senderAddr, btnName);

            // String bazli engelleme — btn_powerup her zaman engelle
            if (lstrcmpiA(btnName, "btn_powerup") == 0)
            {
                printf("[TaskbarMain] PUS (btn_powerup) engellendi\n");
                return true;
            }
        }
        else
        {
            printf("[TaskbarMain] CLICK: sender=0x%08X (ID okunamadi)\n", senderAddr);
        }
    }

    // Adres bazli handler map (diger butonlar icin)
    auto it = m_handlers.find(senderAddr);
    if (it != m_handlers.end())
    {
        __try
        {
            return it->second();
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            return false;
        }
    }

    return false;
}

// =============================================================================
// CUITaskbarMain::RegisterButtonHandler
// =============================================================================
void CUITaskbarMain::RegisterButtonHandler(DWORD btnPtr, std::function<bool()> handler)
{
    if (btnPtr == 0)
    {
        printf("[UITaskbarMain] RegisterButtonHandler: btnPtr 0, reddedildi\n");
        return;
    }
    m_handlers[btnPtr] = handler;
    printf("[UITaskbarMain] Handler kayitlandi: btn 0x%08X\n", btnPtr);
}

// =============================================================================
// CUITaskbarSub — Constructor
// =============================================================================
CUITaskbarSub::CUITaskbarSub()
    : m_dVTableAddr(0)
    , m_btnPowerUPStore(0), m_btnHotkey(0), m_btnGlobalMap(0)
    , m_btnParty(0), m_btnExit(0)
{
    DWORD dlgBase = *(DWORD*)KO_PTR_DLG;
    if (dlgBase == 0)
    {
        printf("[UITaskbarSub] UYARI: KO_PTR_DLG NULL, hook atlaniyor\n");
        return;
    }

    m_dVTableAddr = *(DWORD*)(dlgBase + KO_OFF_DLG_TASKBAR_SUB);
    if (m_dVTableAddr == 0)
    {
        printf("[UITaskbarSub] UYARI: vTable adresi 0, hook atlaniyor (offset: 0x%03X)\n", KO_OFF_DLG_TASKBAR_SUB);
        return;
    }

    printf("[UITaskbarSub] vTable: 0x%08X\n", m_dVTableAddr);
    ParseUIElements();
    InitReceiveMessage();

    // PUS butonu handler'ini kaydet — tiklandiginda engelle
    if (m_btnPowerUPStore != 0)
    {
        RegisterButtonHandler(m_btnPowerUPStore, []() -> bool {
            printf("[UITaskbarSub] PUS (btn_powerup) engellendi\n");
            return true; // Orijinal davranis engellendi
        });
    }
}

CUITaskbarSub::~CUITaskbarSub() {}

// =============================================================================
// CUITaskbarSub::ParseUIElements
// =============================================================================
void CUITaskbarSub::ParseUIElements()
{
    if (m_dVTableAddr == 0) return;

    // Once tum child'lari listele — PUS butonunun gercek ID'sini bulmak icin
    printf("[UITaskbarSub] Tum child'lar listeleniyor...\n");
    g_UIManager.DumpChildren(m_dVTableAddr);

    m_btnPowerUPStore = g_UIManager.GetChildByID(m_dVTableAddr, "btns_pus");
    // btns_pus bulunamazsa alternatif ID'leri dene
    if (m_btnPowerUPStore == 0)
        m_btnPowerUPStore = g_UIManager.GetChildByID(m_dVTableAddr, "btn_pus");
    if (m_btnPowerUPStore == 0)
        m_btnPowerUPStore = g_UIManager.GetChildByID(m_dVTableAddr, "btn_powerup");

    m_btnHotkey       = g_UIManager.GetChildByID(m_dVTableAddr, "btn_hotkey");
    m_btnGlobalMap    = g_UIManager.GetChildByID(m_dVTableAddr, "btn_globalmap");
    m_btnParty        = g_UIManager.GetChildByID(m_dVTableAddr, "btn_party");
    m_btnExit         = g_UIManager.GetChildByID(m_dVTableAddr, "btn_exit");

    if (m_btnPowerUPStore == 0) printf("[UITaskbarSub] UYARI: btns_pus bulunamadi (tum alternatifler denendi)\n");
    if (m_btnHotkey == 0)       printf("[UITaskbarSub] UYARI: btn_hotkey bulunamadi\n");
    if (m_btnGlobalMap == 0)    printf("[UITaskbarSub] UYARI: btn_globalmap bulunamadi\n");
    if (m_btnParty == 0)        printf("[UITaskbarSub] UYARI: btn_party bulunamadi\n");
    if (m_btnExit == 0)         printf("[UITaskbarSub] UYARI: btn_exit bulunamadi\n");

    printf("[UITaskbarSub] ParseUIElements tamamlandi — PUS:0x%08X Hotkey:0x%08X\n",
        m_btnPowerUPStore, m_btnHotkey);
}

// =============================================================================
// UITaskbarSubReceiveMessage_Hook — Global __stdcall hook fonksiyonu
// =============================================================================
void __stdcall UITaskbarSubReceiveMessage_Hook(DWORD* pSender, uint32_t dwMsg)
{
    if (Engine && Engine->m_UITaskbarSub)
    {
        bool handled = Engine->m_UITaskbarSub->ReceiveMessage(pSender, dwMsg);
        if (handled)
            return;
    }

    __asm
    {
        MOV ECX, s_uiTaskbarSubVTable
        PUSH dwMsg
        PUSH pSender
        MOV EAX, s_uiTaskbarSubOrigFunc
        CALL EAX
    }
}

// =============================================================================
// CUITaskbarSub::InitReceiveMessage
// =============================================================================
void CUITaskbarSub::InitReceiveMessage()
{
    if (m_dVTableAddr == 0) return;

    DWORD ptrMsg = (*(DWORD*)m_dVTableAddr) + 0x7C;
    if (ptrMsg == 0x7C)
    {
        printf("[UITaskbarSub] HATA: vTable[0] sifir, hook kurulamadi\n");
        return;
    }

    s_uiTaskbarSubVTable = m_dVTableAddr;
    s_uiTaskbarSubOrigFunc = *(DWORD*)ptrMsg;

    printf("[UITaskbarSub] Hook oncesi: ptrMsg=0x%08X, *ptrMsg=0x%08X\n", ptrMsg, s_uiTaskbarSubOrigFunc);

    // VirtualProtect ile yazma izni al
    DWORD oldProtect = 0;
    BOOL vpResult = VirtualProtect((LPVOID)ptrMsg, sizeof(DWORD), PAGE_EXECUTE_READWRITE, &oldProtect);
    if (!vpResult)
    {
        printf("[UITaskbarSub] UYARI: VirtualProtect basarisiz (err: %d)\n", GetLastError());
    }

    *(DWORD*)ptrMsg = (DWORD)UITaskbarSubReceiveMessage_Hook;

    if (vpResult)
    {
        DWORD tmp;
        VirtualProtect((LPVOID)ptrMsg, sizeof(DWORD), oldProtect, &tmp);
    }

    // Dogrulama
    DWORD verify = *(DWORD*)ptrMsg;
    printf("[UITaskbarSub] Hook sonrasi: *ptrMsg=0x%08X (beklenen: 0x%08X) %s\n",
        verify, (DWORD)UITaskbarSubReceiveMessage_Hook,
        (verify == (DWORD)UITaskbarSubReceiveMessage_Hook) ? "OK" : "BASARISIZ");

    printf("[UITaskbarSub] ReceiveMessage hook kuruldu (orig: 0x%08X)\n", s_uiTaskbarSubOrigFunc);
}

// =============================================================================
// CUITaskbarSub::ReceiveMessage — Handler dispatch
// =============================================================================
bool CUITaskbarSub::ReceiveMessage(DWORD* pSender, uint32_t dwMsg)
{
    if (!pSender)
        return false;

    DWORD senderAddr = (DWORD)pSender;

    if (dwMsg == UIMSG_BUTTON_CLICK)
    {
        char btnName[64] = {0};
        extern bool ReadStdString(DWORD base, DWORD offset, char* outBuf, int maxLen);
        if (ReadStdString(senderAddr, 0x054, btnName, 64))
        {
            printf("[TaskbarSub] CLICK: sender=0x%08X ID=\"%s\"\n", senderAddr, btnName);

            // String bazli engelleme — btn_powerup her zaman engelle
            if (lstrcmpiA(btnName, "btn_powerup") == 0)
            {
                printf("[TaskbarSub] PUS (btn_powerup) engellendi\n");
                return true;
            }
        }
        else
        {
            printf("[TaskbarSub] CLICK: sender=0x%08X (ID okunamadi)\n", senderAddr);
        }
    }

    // Adres bazli handler map (diger butonlar icin)
    auto it = m_handlers.find(senderAddr);
    if (it != m_handlers.end())
    {
        __try
        {
            return it->second();
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            return false;
        }
    }

    return false;
}

// =============================================================================
// CUITaskbarSub::RegisterButtonHandler
// =============================================================================
void CUITaskbarSub::RegisterButtonHandler(DWORD btnPtr, std::function<bool()> handler)
{
    if (btnPtr == 0)
    {
        printf("[UITaskbarSub] RegisterButtonHandler: btnPtr 0, reddedildi\n");
        return;
    }
    m_handlers[btnPtr] = handler;
    printf("[UITaskbarSub] Handler kayitlandi: btn 0x%08X\n", btnPtr);
}
