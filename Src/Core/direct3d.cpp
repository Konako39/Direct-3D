#include "direct3d.h"

#include <dxgi.h>

#pragma comment(lib, "d3d11.lib")

namespace
{
ID3D11Device* g_device = nullptr;
ID3D11DeviceContext* g_context = nullptr;
IDXGISwapChain* g_swap_chain = nullptr;
ID3D11RenderTargetView* g_render_target = nullptr;
ID3D11Texture2D* g_depth_buffer = nullptr;
ID3D11DepthStencilView* g_depth_view = nullptr;

template <typename T>
void Release(T*& object)
{
    if (object)
    {
        object->Release();
        object = nullptr;
    }
}

void ReleaseBackBuffer()
{
    Release(g_depth_view);
    Release(g_depth_buffer);
    Release(g_render_target);
}

bool CreateBackBuffer()
{
    ID3D11Texture2D* back_buffer = nullptr;
    HRESULT result = g_swap_chain->GetBuffer(0, IID_PPV_ARGS(&back_buffer));
    if (FAILED(result))
    {
        return false;
    }

    result = g_device->CreateRenderTargetView(back_buffer, nullptr, &g_render_target);

    D3D11_TEXTURE2D_DESC back_buffer_description{};
    back_buffer->GetDesc(&back_buffer_description);
    Release(back_buffer);

    if (FAILED(result))
    {
        return false;
    }

    D3D11_TEXTURE2D_DESC depth_description{};
    depth_description.Width = back_buffer_description.Width;
    depth_description.Height = back_buffer_description.Height;
    depth_description.MipLevels = 1;
    depth_description.ArraySize = 1;
    depth_description.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depth_description.SampleDesc.Count = 1;
    depth_description.Usage = D3D11_USAGE_DEFAULT;
    depth_description.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    result = g_device->CreateTexture2D(&depth_description, nullptr, &g_depth_buffer);
    if (FAILED(result))
    {
        ReleaseBackBuffer();
        return false;
    }

    result = g_device->CreateDepthStencilView(g_depth_buffer, nullptr, &g_depth_view);
    if (FAILED(result))
    {
        ReleaseBackBuffer();
        return false;
    }

    D3D11_VIEWPORT viewport{};
    viewport.Width = static_cast<float>(back_buffer_description.Width);
    viewport.Height = static_cast<float>(back_buffer_description.Height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    g_context->RSSetViewports(1, &viewport);

    return true;
}
}

bool Direct3D_Initialize(HWND window)
{
    DXGI_SWAP_CHAIN_DESC swap_chain_description{};
    swap_chain_description.BufferCount = 2;
    swap_chain_description.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swap_chain_description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_description.OutputWindow = window;
    swap_chain_description.SampleDesc.Count = 1;
    swap_chain_description.Windowed = TRUE;
    swap_chain_description.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    UINT device_flags = 0;
#if defined(_DEBUG)
    device_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    constexpr D3D_FEATURE_LEVEL feature_levels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };

    D3D_FEATURE_LEVEL created_feature_level{};
    const D3D_FEATURE_LEVEL* requested_levels = feature_levels;
    UINT requested_level_count = ARRAYSIZE(feature_levels);
    HRESULT result = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        device_flags,
        requested_levels,
        requested_level_count,
        D3D11_SDK_VERSION,
        &swap_chain_description,
        &g_swap_chain,
        &g_device,
        &created_feature_level,
        &g_context);

    // Windows 7 等旧运行时不认识 11_1，改用 11_0 再试一次。
    if (result == E_INVALIDARG)
    {
        requested_levels = &feature_levels[1];
        requested_level_count = 1;
        result = D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            device_flags,
            requested_levels,
            requested_level_count,
            D3D11_SDK_VERSION,
            &swap_chain_description,
            &g_swap_chain,
            &g_device,
            &created_feature_level,
            &g_context);
    }

#if defined(_DEBUG)
    if (result == DXGI_ERROR_SDK_COMPONENT_MISSING)
    {
        device_flags &= ~D3D11_CREATE_DEVICE_DEBUG;
        result = D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            device_flags,
            requested_levels,
            requested_level_count,
            D3D11_SDK_VERSION,
            &swap_chain_description,
            &g_swap_chain,
            &g_device,
            &created_feature_level,
            &g_context);
    }
#endif

    if (FAILED(result) || !CreateBackBuffer())
    {
        MessageBoxW(window, L"Direct3D 11 初始化失败。", L"Direct3D 错误", MB_OK | MB_ICONERROR);
        Direct3D_Finalize();
        return false;
    }

    return true;
}

void Direct3D_Finalize()
{
    if (g_context)
    {
        g_context->ClearState();
    }

    ReleaseBackBuffer();
    Release(g_swap_chain);
    Release(g_context);
    Release(g_device);
}

void Direct3D_DrawBegin()
{
    constexpr float clear_color[] = {0.08f, 0.10f, 0.14f, 1.0f};
    g_context->OMSetRenderTargets(1, &g_render_target, g_depth_view);
    g_context->ClearRenderTargetView(g_render_target, clear_color);
    g_context->ClearDepthStencilView(g_depth_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void Direct3D_Present()
{
    g_swap_chain->Present(1, 0);
}

void Direct3D_Resize(unsigned int width, unsigned int height)
{
    if (!g_swap_chain || !g_context || width == 0 || height == 0)
    {
        return;
    }

    g_context->OMSetRenderTargets(0, nullptr, nullptr);
    ReleaseBackBuffer();

    if (SUCCEEDED(g_swap_chain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0)))
    {
        CreateBackBuffer();
    }
}

ID3D11Device* Direct3D_GetDevice()
{
    return g_device;
}

ID3D11DeviceContext* Direct3D_GetDeviceContext()
{
    return g_context;
}
