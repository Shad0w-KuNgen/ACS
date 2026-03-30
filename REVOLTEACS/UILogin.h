#pragma once

// =============================================================================
// UILogin — Login ekrani UI hooklari (stub)
// UIManager ve PearlEngine ile entegre
// TODO: Login ekrani hook implementasyonu
// =============================================================================

class PearlEngine;
class CUIManager;

class UILogin
{
public:
    UILogin();
    ~UILogin();

    void Init(PearlEngine* engine, CUIManager* uiMgr);

    // TODO: Login ekrani hook fonksiyonlari

private:
    PearlEngine* m_pEngine;
    CUIManager*  m_pUiMgr;
};
