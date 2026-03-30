#include "pch.h"
#include "UILogin.h"

// =============================================================================
// UILogin — Login ekrani UI hooklari (stub)
// =============================================================================

UILogin::UILogin()
    : m_pEngine(nullptr)
    , m_pUiMgr(nullptr)
{
}

UILogin::~UILogin()
{
}

void UILogin::Init(PearlEngine* engine, CUIManager* uiMgr)
{
    m_pEngine = engine;
    m_pUiMgr = uiMgr;
    // TODO: Login ekrani hook kurulumu
}
