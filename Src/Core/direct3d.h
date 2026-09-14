#pragma once

#include <Windows.h>
#include <d3d11.h>

bool Direct3D_Initialize(HWND window);
void Direct3D_Finalize();

void Direct3D_DrawBegin();
void Direct3D_Present();
void Direct3D_Resize(unsigned int width, unsigned int height);

ID3D11Device* Direct3D_GetDevice();
ID3D11DeviceContext* Direct3D_GetDeviceContext();
