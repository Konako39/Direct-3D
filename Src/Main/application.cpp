#include "application.h"

#include "direct3d.h"
#include "../../Game.h"
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

    if (!Game_Initialize())
    {
        Game_Finalize();
        Shader3D_Finalize();
        Direct3D_Finalize();
        return false;
    }

    return true;
}

void Application_Update(float delta_time)
{
    // 更新游戏场景。
    Game_Update(delta_time);
}

void Application_Draw()
{
    // 开始一帧、绘制游戏场景，然后显示到窗口。
    Direct3D_DrawBegin();
    Game_Draw();
    Direct3D_Present();
}

void Application_Finalize()
{
    // 先释放游戏资源，再释放 Shader 和 Direct3D。
    Game_Finalize();
    Shader3D_Finalize();
    Direct3D_Finalize();
}
