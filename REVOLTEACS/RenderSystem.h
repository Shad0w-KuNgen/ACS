#pragma once

// =============================================================================
// RenderSystem — DX9/DX11 Overlay Render Sistemi
// 25xx istemcisi DX9 kullanir. DX11 stub olarak eklenmistir.
// EndScene hook ile oyun uzerine ozel cizim yapilir.
// =============================================================================

// d3d9.h pch.h uzerinden dahil edilir

// EndScene vtable index (IDirect3DDevice9 vtable'da 42. slot)
#define D3D9_ENDSCENE_VTABLE_INDEX 42

// EndScene fonksiyon tipi
typedef HRESULT(__stdcall* tEndScene)(LPDIRECT3DDEVICE9 pDevice);

// Basit vertex yapisi — DrawRect ve DrawLine icin
struct RenderVertex
{
    float x, y, z, rhw;
    DWORD color;
};
#define RENDER_VERTEX_FVF (D3DFVF_XYZRHW | D3DFVF_DIFFUSE)

// DX11 Present vtable index (IDXGISwapChain vtable'da 8. slot)
#define DXGI_PRESENT_VTABLE_INDEX 8

// Present fonksiyon tipi
typedef HRESULT(__stdcall* tPresent)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);

class RenderSystem
{
public:
    RenderSystem();
    ~RenderSystem();

    bool Init();
    bool IsDX11() const;

    // --- DX9 ---
    void InitDX9(LPDIRECT3DDEVICE9 pDevice);
    static HRESULT __stdcall hkEndScene(LPDIRECT3DDEVICE9 pDevice);

    // --- DX11 ---
    void InitDX11Device(IDXGISwapChain* pSwapChain);
    static HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);

    // --- Cizim ---
    void DrawText(const char* text, int x, int y, DWORD color);
    void DrawRect(int x, int y, int w, int h, DWORD color);
    void DrawFilledRect(int x, int y, int w, int h, DWORD color);
    void DrawLine(int x1, int y1, int x2, int y2, DWORD color, int thickness = 1);
    void DrawTexture(int x, int y, int w, int h);

    void OnDeviceLost();
    void OnDeviceReset();

    bool IsInitialized() const { return m_bInitialized; }
    LPDIRECT3DDEVICE9 GetDevice() const { return m_pDX9Device; }

private:
    bool m_bInitialized;
    bool m_bDX11Mode;
    bool m_bDeviceLost;

    // DX9
    LPDIRECT3DDEVICE9 m_pDX9Device;
    static tEndScene s_oEndScene;

    // DX11
    IDXGISwapChain* m_pSwapChain;
    ID3D11Device* m_pDX11Device;
    ID3D11DeviceContext* m_pDX11Context;
    static tPresent s_oPresent;

    static RenderSystem* s_pInstance;
    void DrawTextGDI(HDC hdc, const char* text, int x, int y, DWORD color);
};

// Global RenderSystem instance (dllmain.cpp'de tanimlanir)
extern RenderSystem g_RenderSystem;
