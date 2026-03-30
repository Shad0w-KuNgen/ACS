#pragma once

// =============================================================================
// UIChatBar — Chat bar UI hooklari (stub)
// UIManager ve PearlEngine ile entegre
// TODO: Chat bar hook implementasyonu
// =============================================================================

class PearlEngine;
class CUIManager;

class UIChatBar
{
public:
    UIChatBar();
    ~UIChatBar();

    void Init(PearlEngine* engine, CUIManager* uiMgr);

    // TODO: Chat bar hook fonksiyonlari

private:
    PearlEngine* m_pEngine;
    CUIManager*  m_pUiMgr;
};
