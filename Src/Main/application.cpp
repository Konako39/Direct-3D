#include "application.h"
#include "../../cube.h"
#include "direct3d.h"
#include "../../Shader3D.h"

bool Application_Initialize(HWND window)
{
    if (!Direct3D_Initialize(window))
    {
        return false;
    }

    if (!Shader3D_Initialize(Direct3D_GetDevice(), Direct3D_GetDeviceContext()))
    {
        Shader3D_Finalize();
        Direct3D_Finalize();
        return false;
    }

    if (!Cube_Initialize())
    {
        Cube_Finalize();
        Shader3D_Finalize();
        Direct3D_Finalize();
        return false;
    }

    return true;
}

void Application_Update(float delta_time)
{
    // 预留给后续 3D 场景更新。
    (void)delta_time;
}

void Application_Draw()
{
    Direct3D_DrawBegin();

    Cube_Draw();
    // 预留给后续 3D 渲染。


    Direct3D_Present();
}

void Application_Finalize()
{
	Cube_Finalize();

    Shader3D_Finalize();
    Direct3D_Finalize();
}
