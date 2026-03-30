#pragma once

// =============================================================================
// UIFramework — Ozel UI Eleman Sistemi
// RenderSystem uzerinden cizim yapar, oyunun N3UIBase sistemiyle cakismaz.
// =============================================================================

#include <functional>

class RenderSystem; // forward declaration

// =============================================================================
// UIElement — Temel UI eleman sinifi (soyut)
// =============================================================================
class UIElement
{
public:
    UIElement();
    virtual ~UIElement();

    virtual void Render(RenderSystem* renderer) = 0;
    virtual bool HandleMouse(uint32 flags, POINT pt) = 0;

    void SetPosition(int x, int y);
    void SetSize(int w, int h);
    void SetVisible(bool visible);
    bool IsVisible() const;
    bool IsPointInside(POINT pt) const;

    int GetX() const { return m_x; }
    int GetY() const { return m_y; }
    int GetWidth() const { return m_w; }
    int GetHeight() const { return m_h; }

    void SetZOrder(int z) { m_iZOrder = z; }
    int  GetZOrder() const { return m_iZOrder; }

    bool m_bDraggable;

protected:
    int  m_x, m_y;
    int  m_w, m_h;
    bool m_bVisible;
    int  m_iZOrder;
};

// =============================================================================
// UIPanel — Arka plan dikdortgeni
// =============================================================================
class UIPanel : public UIElement
{
public:
    UIPanel();
    virtual ~UIPanel();

    void Render(RenderSystem* renderer) override;
    bool HandleMouse(uint32 flags, POINT pt) override;

    void SetBackgroundColor(DWORD color) { m_dwBgColor = color; }
    void SetBorderColor(DWORD color)     { m_dwBorderColor = color; }
    void SetDrawBorder(bool draw)        { m_bDrawBorder = draw; }

private:
    DWORD m_dwBgColor;
    DWORD m_dwBorderColor;
    bool  m_bDrawBorder;
};

// =============================================================================
// UIButton — Tiklanabilir buton (onClick callback)
// =============================================================================
class UIButton : public UIElement
{
public:
    UIButton();
    virtual ~UIButton();

    void Render(RenderSystem* renderer) override;
    bool HandleMouse(uint32 flags, POINT pt) override;

    void SetText(const char* text)       { m_strText = text ? text : ""; }
    void SetColor(DWORD normal, DWORD hover, DWORD pressed);
    void SetTextColor(DWORD color)       { m_dwTextColor = color; }
    void SetOnClick(std::function<void()> fn) { m_onClick = fn; }

private:
    std::string m_strText;
    DWORD m_dwColorNormal;
    DWORD m_dwColorHover;
    DWORD m_dwColorPressed;
    DWORD m_dwTextColor;
    bool  m_bHovered;
    bool  m_bPressed;
    std::function<void()> m_onClick;
};

// =============================================================================
// UILabel — Metin etiketi
// =============================================================================
class UILabel : public UIElement
{
public:
    UILabel();
    virtual ~UILabel();

    void Render(RenderSystem* renderer) override;
    bool HandleMouse(uint32 flags, POINT pt) override;

    void SetText(const char* text)   { m_strText = text ? text : ""; }
    void SetTextColor(DWORD color)   { m_dwTextColor = color; }
    void SetFontSize(int size)       { m_iFontSize = size; }
    const std::string& GetText() const { return m_strText; }

private:
    std::string m_strText;
    DWORD m_dwTextColor;
    int   m_iFontSize;
};

// =============================================================================
// UIEditBox — Metin giris alani (klavye input destegi)
// =============================================================================
class UIEditBox : public UIElement
{
public:
    UIEditBox();
    virtual ~UIEditBox();

    void Render(RenderSystem* renderer) override;
    bool HandleMouse(uint32 flags, POINT pt) override;

    void OnChar(char c);
    void OnKeyDown(uint32 vk);

    void SetText(const char* text)   { m_strText = text ? text : ""; }
    const std::string& GetText() const { return m_strText; }
    void SetMaxLength(int len)       { m_iMaxLength = len; }
    void SetFocused(bool f)          { m_bFocused = f; }
    bool IsFocused() const           { return m_bFocused; }

private:
    std::string m_strText;
    DWORD m_dwBgColor;
    DWORD m_dwTextColor;
    DWORD m_dwBorderColor;
    int   m_iMaxLength;
    bool  m_bFocused;
};

// =============================================================================
// UIListBox — Kaydirilabilir liste
// =============================================================================
class UIListBox : public UIElement
{
public:
    UIListBox();
    virtual ~UIListBox();

    void Render(RenderSystem* renderer) override;
    bool HandleMouse(uint32 flags, POINT pt) override;

    void AddItem(const std::string& text, DWORD color = 0xFFFFFFFF);
    void ClearItems();
    int  GetSelectedIndex() const { return m_iSelectedIndex; }
    int  GetItemCount() const     { return (int)m_items.size(); }

private:
    struct ListItem { std::string text; DWORD color; };
    std::vector<ListItem> m_items;
    int   m_iScrollOffset;
    int   m_iSelectedIndex;
    int   m_iItemHeight;
    DWORD m_dwBgColor;
    DWORD m_dwSelColor;
};

// =============================================================================
// UIProgressBar — Ilerleme cubugu
// =============================================================================
class UIProgressBar : public UIElement
{
public:
    UIProgressBar();
    virtual ~UIProgressBar();

    void Render(RenderSystem* renderer) override;
    bool HandleMouse(uint32 flags, POINT pt) override;

    void SetRange(float min, float max) { m_fMin = min; m_fMax = max; }
    void SetValue(float val)            { m_fCurrent = val; }
    float GetValue() const              { return m_fCurrent; }
    float GetPercent() const;

    void SetBarColor(DWORD color)       { m_dwBarColor = color; }
    void SetBgColor(DWORD color)        { m_dwBgColor = color; }

private:
    float m_fMin;
    float m_fMax;
    float m_fCurrent;
    DWORD m_dwBarColor;
    DWORD m_dwBgColor;
};

// =============================================================================
// UIFramework — Ana yonetici sinifi
// =============================================================================
class UIFramework
{
public:
    UIFramework();
    ~UIFramework();

    void Render(RenderSystem* renderer);
    bool MouseProc(uint32 flags, POINT pt);

    void AddElement(UIElement* element);
    void RemoveElement(UIElement* element);

private:
    // Z-order'a gore siralanmis eleman listesi (en yuksek z = en ustte)
    std::vector<UIElement*> m_elements;

    // Drag state
    UIElement* m_pDragElement;
    POINT      m_ptDragStart;
    int        m_iDragOrigX;
    int        m_iDragOrigY;

    UIElement* FindTopElementAt(POINT pt);
    void SortByZOrder();
};

// Global UIFramework instance
extern UIFramework g_UIFramework;
