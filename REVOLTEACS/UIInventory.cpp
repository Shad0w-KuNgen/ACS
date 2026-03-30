#include "pch.h"
#include "UIInventory.h"

// =============================================================================
// UIInventory — Envanter UI hooklari (stub)
// =============================================================================

UIInventory::UIInventory()
    : m_pEngine(nullptr)
    , m_pUiMgr(nullptr)
{
}

UIInventory::~UIInventory()
{
}

void UIInventory::Init(PearlEngine* engine, CUIManager* uiMgr)
{
    m_pEngine = engine;
    m_pUiMgr = uiMgr;
    // TODO: Envanter hook kurulumu
}
