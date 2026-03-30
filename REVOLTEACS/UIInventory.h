#pragma once

// =============================================================================
// UIInventory — Envanter UI hooklari (stub)
// UIManager ve PearlEngine ile entegre
// TODO: Envanter hook implementasyonu
// =============================================================================

class PearlEngine;
class CUIManager;

class UIInventory
{
public:
    UIInventory();
    ~UIInventory();

    void Init(PearlEngine* engine, CUIManager* uiMgr);

    // TODO: Envanter hook fonksiyonlari

private:
    PearlEngine* m_pEngine;
    CUIManager*  m_pUiMgr;
};
