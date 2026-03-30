#pragma once

// =============================================================================
// CGameHooks — Oyun Fonksiyon Hook Sinifi
// Tick, EndGame, kamera, nesne dongusu ve UI ReceiveMessage hooklari
// Mevcut dllmain.cpp'deki hook mantigi buraya tasinmistir
// =============================================================================

class CGameHooks
{
public:
    CGameHooks();
    ~CGameHooks();

    // Tum hooklari kur
    void InitAllHooks(HANDLE hProcess);

    // --- Tick Hook ---
    static void __fastcall myTick();

    // --- EndGame Hook ---
    // Not: hkEndGames ve hkTick naked fonksiyonlar — sinif disi tanimlanir

    // --- Kamera / Nesne Dongusu / Mouse Hook Stub'lari ---
    // TODO: 25xx icin adresler henuz kesfedilmedi
    // IDA/x32dbg ile bulundugunda aktif edilecek
    void InitCameraHook();
    void InitObjectLoopHooks();
    void InitMouseHook();

    // --- UI ReceiveMessage Hook Stub'lari ---
    // TODO: 25xx icin adresler henuz kesfedilmedi
    void InitUIReceiveMessageHooks();

private:
    // Tick hook icin orijinal fonksiyon adresi (public — naked fonksiyondan erisilebilir)
public:
    static DWORD s_TICK_ORG;
private:

    // Tick zamanlayici
    static time_t s_sTimers;
};
