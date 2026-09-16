#include "Game.h"

#include <DirectXMath.h>

#include "Shader3D.h"
#include "Src/Core/direct3d.h"
#include "cube.h"
#include "grid.h"

using namespace DirectX;

static float Rotation{};
static float Scale{};
float dt;

bool Game_Initialize()
{
    // 初始化立方体。
    if (!Cube_Initialize())
    {
        return false;
    }

    // 初始化地面网格；失败时释放已创建的立方体资源。
    if (!Grid_Initialize())
    {
        Cube_Finalize();
        return false;
    }

    return true;
}

void Game_Update(float delta_time)
{
    // 预留给后续 3D 场景更新。
    (void)delta_time;
    dt += delta_time;
    Rotation += 2.0f;
    Scale = cosf(dt) + 2.0f;
}

void Game_Draw()
{
    // Shader设定。
    Shader3D_Begin();

    // View矩阵变换行列。
    const XMMATRIX view = XMMatrixLookAtLH(
        XMVectorSet(3.0f, 3.0f, -8.0f, 1.0f), // 相机位置
        XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f),  // 相机看向的目标点
        XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)); // 相机的上方向
    Shader3D_SetViewMatrix(view);

    // Projection矩阵变换行列，透视投影使用 60 度视角。
    const XMMATRIX projection = XMMatrixPerspectiveFovLH(
        XMConvertToRadians(60.0f),
        SCREEN_WIDTH / SCREEN_HEIGHT,
        0.1f,
        100.0f);
    Shader3D_SetProjectionMatrix(projection);

    // 世界坐标变换矩阵,将立方体平移到xx。
    // 运动、旋转、缩放都是通过矩阵相乘完成的。
    const XMMATRIX world = XMMatrixTranslation(0.5f, 0.5f, 0.0f);
    const XMMATRIX CubeRotation = XMMatrixRotationY(XMConvertToRadians(Rotation));
    const XMMATRIX CubeScale = XMMatrixScaling(5.0f,0.5f,0.5f);
    const XMMATRIX CubeMatrixA = world * CubeScale * CubeRotation;
    Cube_Draw(CubeMatrixA);

    // 网格使用自己的单位世界矩阵，共用上面的相机和投影矩阵。
    Grid_Draw();
}

void Game_Finalize()
{
    // 释放游戏场景中创建的 GPU 资源。
    Grid_Finalize();
    Cube_Finalize();
}
