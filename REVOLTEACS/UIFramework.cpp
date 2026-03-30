#include "pch.h"

// =============================================================================
// UIFramework — Ozel UI Eleman Sistemi Implementasyonu
// RenderSystem uzerinden cizim yapar, oyunun N3UIBase sistemiyle cakismaz.
// =============================================================================

// Global instance
UIFramework g_UIFramework;

// =============================================================================
// UIElement — Temel sinif
// =============================================================================
UIElement::UIElement()
    : m_x(0), m_y(0), m_w(100), m_h(30)
    , m_bVisible(true)
    , m_bDraggable(false)
    , m_iZOrder(0)
{
}

UIElement::~UIElement() {}

void UIElement::SetPosition(int x, int y)
{
    m_x = x;
    m_y = y;
}

void UIElement::SetSize(int w, int h)
{
    m_w = w;
    m_h = h;
}

void UIElement::SetVisible(bool visible)
{
    m_bVisible = visible;
}

bool UIElement::IsVisible() const
{
    return m_bVisible;
}

bool UIElement::IsPointInside(POINT pt) const
{
    return (pt.x >= m_x && pt.x < m_x + m_w &&
            pt.y >= m_y && pt.y < m_y + m_h);
}

// =============================================================================
// UIPanel — Arka plan dikdortgeni
// =============================================================================
UIPanel::UIPanel()
    : m_dwBgColor(0xCC222222)
    , m_dwBorderColor(0xFF888888)
    , m_bDrawBorder(true)
{
}

UIPanel::~UIPanel() {}

void UIPanel::Render(RenderSystem* renderer)
{
    if (!renderer || !m_bVisible)
        return;

    renderer->DrawFilledRect(m_x, m_y, m_w, m_h, m_dwBgColor);

    if (m_bDrawBorder)
        renderer->DrawRect(m_x, m_y, m_w, m_h, m_dwBorderColor);
}

bool UIPanel::HandleMouse(uint32 flags, POINT pt)
{
    // Panel sadece mouse olayini tuketir (arka plandaki elemanlara gecmez)
    if (!m_bVisible || !IsPointInside(pt))
        return false;
    return true;
}

// =============================================================================
// UIButton — Tiklanabilir buton
// =============================================================================
UIButton::UIButton()
    : m_dwColorNormal(0xFF444444)
    , m_dwColorHover(0xFF666666)
    , m_dwColorPressed(0xFF333333)
    , m_dwTextColor(0xFFFFFFFF)
    , m_bHovered(false)
    , m_bPressed(false)
{
}

UIButton::~UIButton() {}

void UIButton::SetColor(DWORD normal, DWORD hover, DWORD pressed)
{
    m_dwColorNormal  = normal;
    m_dwColorHover   = hover;
    m_dwColorPressed = pressed;
}

void UIButton::Render(RenderSystem* renderer)
{
    if (!renderer || !m_bVisible)
        return;

    DWORD bgColor = m_dwColorNormal;
    if (m_bPressed)
        bgColor = m_dwColorPressed;
    else if (m_bHovered)
        bgColor = m_dwColorHover;

    renderer->DrawFilledRect(m_x, m_y, m_w, m_h, bgColor);
    renderer->DrawRect(m_x, m_y, m_w, m_h, 0xFF888888);

    if (!m_strText.empty())
    {
        // Metni butonun ortasina yakın yerlestir
        int textX = m_x + 4;
        int textY = m_y + (m_h - 14) / 2;
        renderer->DrawText(m_strText.c_str(), textX, textY, m_dwTextColor);
    }
}

bool UIButton::HandleMouse(uint32 flags, POINT pt)
{
    if (!m_bVisible)
        return false;

    bool inside = IsPointInside(pt);

    if (flags == WM_MOUSEMOVE)
    {
        m_bHovered = inside;
        if (!inside)
            m_bPressed = false;
        return inside;
    }

    if (flags == WM_LBUTTONDOWN && inside)
    {
        m_bPressed = true;
        return true;
    }

    if (flags == WM_LBUTTONUP)
    {
        if (m_bPressed && inside)
        {
            m_bPressed = false;
            if (m_onClick)
                m_onClick();
            return true;
        }
        m_bPressed = false;
    }

    return inside;
}

// =============================================================================
// UILabel — Metin etiketi
// =============================================================================
UILabel::UILabel()
    : m_dwTextColor(0xFFFFFFFF)
    , m_iFontSize(14)
{
}

UILabel::~UILabel() {}

void UILabel::Render(RenderSystem* renderer)
{
    if (!renderer || !m_bVisible || m_strText.empty())
        return;

    renderer->DrawText(m_strText.c_str(), m_x, m_y, m_dwTextColor);
}

bool UILabel::HandleMouse(uint32 flags, POINT pt)
{
    // Label fare olaylarini islemez
    return false;
}

// =============================================================================
// UIEditBox — Metin giris alani (stub — temel klavye destegi)
// =============================================================================
UIEditBox::UIEditBox()
    : m_dwBgColor(0xFF1A1A1A)
    , m_dwTextColor(0xFFFFFFFF)
    , m_dwBorderColor(0xFF888888)
    , m_iMaxLength(256)
    , m_bFocused(false)
{
}

UIEditBox::~UIEditBox() {}

void UIEditBox::Render(RenderSystem* renderer)
{
    if (!renderer || !m_bVisible)
        return;

    renderer->DrawFilledRect(m_x, m_y, m_w, m_h, m_dwBgColor);
    DWORD borderColor = m_bFocused ? 0xFFAADDFF : m_dwBorderColor;
    renderer->DrawRect(m_x, m_y, m_w, m_h, borderColor);

    if (!m_strText.empty())
        renderer->DrawText(m_strText.c_str(), m_x + 4, m_y + (m_h - 14) / 2, m_dwTextColor);
}

bool UIEditBox::HandleMouse(uint32 flags, POINT pt)
{
    if (!m_bVisible)
        return false;

    if (flags == WM_LBUTTONDOWN)
    {
        m_bFocused = IsPointInside(pt);
        return m_bFocused;
    }
    return IsPointInside(pt);
}

void UIEditBox::OnChar(char c)
{
    if (!m_bFocused)
        return;
    if (c >= 32 && c < 127 && (int)m_strText.length() < m_iMaxLength)
        m_strText += c;
}

void UIEditBox::OnKeyDown(uint32 vk)
{
    if (!m_bFocused)
        return;
    if (vk == VK_BACK && !m_strText.empty())
        m_strText.pop_back();
}

// =============================================================================
// UIListBox — Kaydirilabilir liste (stub)
// =============================================================================
UIListBox::UIListBox()
    : m_iScrollOffset(0)
    , m_iSelectedIndex(-1)
    , m_iItemHeight(18)
    , m_dwBgColor(0xFF1A1A1A)
    , m_dwSelColor(0xFF3366AA)
{
}

UIListBox::~UIListBox() {}

void UIListBox::AddItem(const std::string& text, DWORD color)
{
    m_items.push_back({ text, color });
}

void UIListBox::ClearItems()
{
    m_items.clear();
    m_iScrollOffset = 0;
    m_iSelectedIndex = -1;
}

void UIListBox::Render(RenderSystem* renderer)
{
    if (!renderer || !m_bVisible)
        return;

    renderer->DrawFilledRect(m_x, m_y, m_w, m_h, m_dwBgColor);
    renderer->DrawRect(m_x, m_y, m_w, m_h, 0xFF888888);

    int visibleCount = m_h / m_iItemHeight;
    for (int i = 0; i < visibleCount && (i + m_iScrollOffset) < (int)m_items.size(); ++i)
    {
        int idx = i + m_iScrollOffset;
        int itemY = m_y + i * m_iItemHeight;

        if (idx == m_iSelectedIndex)
            renderer->DrawFilledRect(m_x + 1, itemY, m_w - 2, m_iItemHeight, m_dwSelColor);

        renderer->DrawText(m_items[idx].text.c_str(), m_x + 4, itemY + 2, m_items[idx].color);
    }
}

bool UIListBox::HandleMouse(uint32 flags, POINT pt)
{
    if (!m_bVisible || !IsPointInside(pt))
        return false;

    if (flags == WM_LBUTTONDOWN)
    {
        int relY = pt.y - m_y;
        int clickedIdx = m_iScrollOffset + relY / m_iItemHeight;
        if (clickedIdx >= 0 && clickedIdx < (int)m_items.size())
            m_iSelectedIndex = clickedIdx;
        return true;
    }
    return true;
}

// =============================================================================
// UIProgressBar — Ilerleme cubugu
// =============================================================================
UIProgressBar::UIProgressBar()
    : m_fMin(0.0f)
    , m_fMax(100.0f)
    , m_fCurrent(0.0f)
    , m_dwBarColor(0xFF44AA44)
    , m_dwBgColor(0xFF222222)
{
}

UIProgressBar::~UIProgressBar() {}

float UIProgressBar::GetPercent() const
{
    if (m_fMax <= m_fMin)
        return 0.0f;
    float pct = (m_fCurrent - m_fMin) / (m_fMax - m_fMin);
    if (pct < 0.0f) pct = 0.0f;
    if (pct > 1.0f) pct = 1.0f;
    return pct;
}

void UIProgressBar::Render(RenderSystem* renderer)
{
    if (!renderer || !m_bVisible)
        return;

    // Arka plan
    renderer->DrawFilledRect(m_x, m_y, m_w, m_h, m_dwBgColor);

    // Dolu kisim
    float pct = GetPercent();
    int fillW = (int)(m_w * pct);
    if (fillW > 0)
        renderer->DrawFilledRect(m_x, m_y, fillW, m_h, m_dwBarColor);

    // Cerceve
    renderer->DrawRect(m_x, m_y, m_w, m_h, 0xFF888888);
}

bool UIProgressBar::HandleMouse(uint32 flags, POINT pt)
{
    // ProgressBar fare olaylarini islemez
    return false;
}

// =============================================================================
// UIFramework — Ana yonetici sinifi
// Z-order tabanli render ve fare yonlendirme
// Oyunun N3UIBase sistemiyle cakismaz (kendi eleman listesini yonetir)
// =============================================================================
UIFramework::UIFramework()
    : m_pDragElement(nullptr)
    , m_iDragOrigX(0)
    , m_iDragOrigY(0)
{
    m_ptDragStart.x = 0;
    m_ptDragStart.y = 0;
}

UIFramework::~UIFramework()
{
    // Elemanlarin sahipligi disarida — burada silmiyoruz
    m_elements.clear();
}

void UIFramework::AddElement(UIElement* element)
{
    if (!element)
        return;
    m_elements.push_back(element);
    SortByZOrder();
}

void UIFramework::RemoveElement(UIElement* element)
{
    if (!element)
        return;
    for (auto it = m_elements.begin(); it != m_elements.end(); ++it)
    {
        if (*it == element)
        {
            if (m_pDragElement == element)
                m_pDragElement = nullptr;
            m_elements.erase(it);
            return;
        }
    }
}

void UIFramework::SortByZOrder()
{
    // Dusuk z-order once ciziilir (arka plan), yuksek z-order en ustte
    std::sort(m_elements.begin(), m_elements.end(),
        [](const UIElement* a, const UIElement* b) {
            return a->GetZOrder() < b->GetZOrder();
        });
}

// Render: dusuk z-order'dan yuksege dogru ciz (en ustteki en son ciziilir)
void UIFramework::Render(RenderSystem* renderer)
{
    if (!renderer)
        return;

    for (auto* elem : m_elements)
    {
        if (elem && elem->IsVisible())
            elem->Render(renderer);
    }
}

// En ustteki gorunur elemani bul (yuksek z-order oncelikli)
UIElement* UIFramework::FindTopElementAt(POINT pt)
{
    // Ters sira — en yuksek z-order'dan basla
    for (int i = (int)m_elements.size() - 1; i >= 0; --i)
    {
        UIElement* elem = m_elements[i];
        if (elem && elem->IsVisible() && elem->IsPointInside(pt))
            return elem;
    }
    return nullptr;
}

// MouseProc: fare olaylarini z-order'a gore en ustteki elemana yonlendir
// Drag destegi: m_bDraggable olan elemanlar suruklenir
// Return: true = olay islendi (oyunun N3UIBase'ine gecme), false = islenmedii
bool UIFramework::MouseProc(uint32 flags, POINT pt)
{
    // --- Drag devam ediyor mu? ---
    if (m_pDragElement && flags == WM_MOUSEMOVE)
    {
        int dx = pt.x - m_ptDragStart.x;
        int dy = pt.y - m_ptDragStart.y;
        m_pDragElement->SetPosition(m_iDragOrigX + dx, m_iDragOrigY + dy);
        return true;
    }

    if (m_pDragElement && flags == WM_LBUTTONUP)
    {
        m_pDragElement = nullptr;
        return true;
    }

    // --- Yeni olay: en ustteki elemani bul ---
    UIElement* target = FindTopElementAt(pt);

    if (!target)
        return false;

    // Drag baslat
    if (flags == WM_LBUTTONDOWN && target->m_bDraggable)
    {
        m_pDragElement = target;
        m_ptDragStart  = pt;
        m_iDragOrigX   = target->GetX();
        m_iDragOrigY   = target->GetY();
        // Drag baslarken elemana da haber ver
        target->HandleMouse(flags, pt);
        return true;
    }

    return target->HandleMouse(flags, pt);
}
