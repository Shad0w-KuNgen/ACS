#pragma once

// =============================================================================
// PearlEngine — Merkezi Engine Sinifi
// Tum modulleri koordine eder: PlayerBase, UIManager, PacketHandler,
// RenderSystem, UIFramework, GameHooks
// =============================================================================

// Forward declarations
class CPlayerBase;
class CUIManager;
class PacketHandler;
class RenderSystem;
class UIFramework;
class CGameHooks;
class Packet;
class CUITaskbarMain;
class CUITaskbarSub;

class PearlEngine
{
public:
    PearlEngine();
    ~PearlEngine();

    // Tum modulleri baslat
    void Init();

    // Ana guncelleme dongusu — PlayerBase guncelleme, UI framework render tetikleme
    void Update();

    // PacketHandler uzerinden paket gonderme
    void Send(Packet* pkt);

    // Yardimci fonksiyonlar
    int16 GetTarget();
    uint8 GetNation();

    // vTable'dan ReceiveMessage pointer adresi dondur
    DWORD GetRecvMessagePtr(DWORD vTableAddr);

    // Engine ana dongusu (ayri thread)
    static DWORD WINAPI EngineMain(LPVOID lpParam);

    // Modul referanslari (mevcut global instance'lara pointer)
    CPlayerBase*   m_PlayerBase;
    CUIManager*    m_UiMgr;
    PacketHandler* m_PacketHandler;
    RenderSystem*  m_RenderSystem;
    UIFramework*   m_UIFramework;
    CGameHooks*    m_GameHooks;

    // Taskbar hook modulleri
    CUITaskbarMain* m_UITaskbarMain;
    CUITaskbarSub*  m_UITaskbarSub;

private:
    bool m_bInitialized;
    HANDLE m_hEngineThread;
};

// Global Engine pointer
extern PearlEngine* Engine;
