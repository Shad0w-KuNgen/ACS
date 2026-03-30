#pragma once

// =============================================================================
// CUIManager — UI Hook ve Kontrol Sinifi
// Oyunun N3UIBase tabanli UI fonksiyonlarini hooklar ve kontrol eder
// GetChildByID hook'u dllmain.cpp'den buraya tasinmistir
// =============================================================================

#include <string>

// GetChildByID orijinal fonksiyon typedef'i
typedef void(WINAPI* tGetChild)(const std::string& szString, DWORD nUnknown);

class CUIManager
{
public:
    CUIManager();
    ~CUIManager();

    // Hook kurulumu — GetChildByID hook'unu kurar
    void Init();

    // --- UI Eleman Kontrolu ---

    // Gorunurluk
    void SetVisible(DWORD pElement, bool bVisible);
    bool IsVisible(DWORD pElement);

    // Metin
    void SetString(DWORD pElement, const std::string& str);
    std::string GetString(DWORD pElement);

    // Durum
    void SetState(DWORD pElement, DWORD dwState);

    // Alt eleman bulma — hook uzerinden calisan mevcut mekanizma
    DWORD GetChildByID(DWORD pParent, const std::string& szID);

    // Tum child'lari konsola dump et (debug)
    void DumpChildren(DWORD pParent);

    // --- Pozisyon ve Boyut ---
    void  SetUIPos(DWORD pElement, POINT pt);
    POINT GetUiPos(DWORD pElement);
    LONG  GetUiWidth(DWORD pElement);
    LONG  GetUiHeight(DWORD pElement);

    // --- Region ---
    void SetUiRegion(DWORD pElement, RECT rc);
    RECT GetUiRegion(DWORD pElement);

    // --- Liste Islemleri ---
    void AddListString(DWORD pElement, const std::string& str, DWORD dwColor = 0xFFFFFFFF);
    void ClearListString(DWORD pElement);

    // --- Hook Callback (static) ---
    static void __stdcall GetChildByID_Hook(const std::string& szString, DWORD nUnknown);

    // Orijinal fonksiyon pointer'i
    static tGetChild s_oGetChild;
};
