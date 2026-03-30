#include "pch.h"
#include "UIChatBar.h"

// =============================================================================
// UIChatBar — Chat bar UI hooklari (stub)
// =============================================================================

UIChatBar::UIChatBar()
    : m_pEngine(nullptr)
    , m_pUiMgr(nullptr)
{
}

UIChatBar::~UIChatBar()
{
}

void UIChatBar::Init(PearlEngine* engine, CUIManager* uiMgr)
{
    m_pEngine = engine;
    m_pUiMgr = uiMgr;
    // TODO: Chat bar hook kurulumu
}
