#pragma once

// =============================================================================
// UITaskbar — Taskbar ReceiveMessage Hook Sistemi
// UITaskbarMain: Alt gorev cubugu ana bolumu (Stand, Sit, Trade, Skill, vb.)
// UITaskbarSub:  Alt gorev cubugu yan bolumu (PUS, Hotkey, GlobalMap, vb.)
// Referans: Pearl Guard 2369 CUITaskbarMainPlug / CUITaskbarSubPlug
// =============================================================================

#include <functional>
#include <unordered_map>

// Forward declarations
class PearlEngine;
class CUIManager;

// =============================================================================
// CUITaskbarMain — TaskbarMain ReceiveMessage hook ve buton yonetimi
// =============================================================================
class CUITaskbarMain
{
public:
    CUITaskbarMain();
    ~CUITaskbarMain();

    // Buton pointer'larini GetChildByID ile coz
    void ParseUIElements();

    // vTable ReceiveMessage pointer'ini hook fonksiyonuyla degistir
    void InitReceiveMessage();

    // Hook'tan cagrilan handler — true donerse orijinal engellenir
    bool ReceiveMessage(DWORD* pSender, uint32_t dwMsg);

    // Buton handler kaydi — callback true donerse orijinal engellenir
    void RegisterButtonHandler(DWORD btnPtr, std::function<bool()> handler);

    // vTable adresi (public — hook fonksiyonundan erisim icin)
    DWORD m_dVTableAddr;

    // Buton pointer'lari — base_TaskBar altindan
    DWORD m_baseTaskBar;
    DWORD m_btn00Stand;     // "btn_00"
    DWORD m_btn01Sit;       // "btn_01"
    DWORD m_btn02Seek;      // "btn_02"
    DWORD m_btn03Trade;     // "btn_03"
    DWORD m_btn04Skill;     // "btn_04"
    DWORD m_btn05Character; // "btn_05"
    DWORD m_btn06Inventory; // "btn_06"

    // Buton pointer'lari — base_menu altindan
    DWORD m_baseMenu;
    DWORD m_btnMenu;        // "btn_menu"
    DWORD m_btnRank;        // "btn_rank"

private:
    std::unordered_map<DWORD, std::function<bool()>> m_handlers;
};

// =============================================================================
// CUITaskbarSub — TaskbarSub ReceiveMessage hook ve buton yonetimi
// =============================================================================
class CUITaskbarSub
{
public:
    CUITaskbarSub();
    ~CUITaskbarSub();

    void ParseUIElements();
    void InitReceiveMessage();
    bool ReceiveMessage(DWORD* pSender, uint32_t dwMsg);
    void RegisterButtonHandler(DWORD btnPtr, std::function<bool()> handler);

    DWORD m_dVTableAddr;

    // Buton pointer'lari
    DWORD m_btnPowerUPStore;  // "btns_pus"
    DWORD m_btnHotkey;        // "btn_hotkey"
    DWORD m_btnGlobalMap;     // "btn_globalmap"
    DWORD m_btnParty;         // "btn_party"
    DWORD m_btnExit;          // "btn_exit"

private:
    std::unordered_map<DWORD, std::function<bool()>> m_handlers;
};
