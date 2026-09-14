#include "application.h"

#include "direct3d.h"

bool Application_Initialize(HWND window)
{
    return Direct3D_Initialize(window);
}

void Application_Update(float delta_time)
{
    // 预留给后续 3D 场景更新。
    (void)delta_time;
}

void Application_Draw()
{
    Direct3D_DrawBegin();

    // 预留给后续 3D 渲染。

    Direct3D_Present();
}

void Application_Finalize()
{
    Direct3D_Finalize();
}
