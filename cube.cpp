
#include <d3d11.h>
#include <DirectXMath.h>
#include "shader3d.h"
#include "cube.h"
#include "Src/Core/direct3d.h"
using namespace DirectX;

static ID3D11Buffer* g_VertexBuffer = nullptr;
// 顶点缓冲区 顶点放到这里传给GPU
static ID3D11RasterizerState* g_RasterizerState = nullptr;
// 光栅化状态 画实心还是线？
static ID3D11DepthStencilState* g_DepthStencilState = nullptr;
// 深度测试谁在前谁在后

// 顶点构造体
struct Vertex
{
	XMFLOAT4 position; // 位置
	XMFLOAT2 uv;       // 纹理坐标
};

static constexpr int NUM_VERTEX = 6; // 立方体的顶点数量

bool Cube_Initialize()
{
	ID3D11Device* device = Direct3D_GetDevice();
	if (device == nullptr)
	{
		return false;
	}

	D3D11_BUFFER_DESC bd{
		.ByteWidth = sizeof(Vertex) * NUM_VERTEX,
		// 大小
		.Usage = D3D11_USAGE_DEFAULT,
		// 如何使用 主要交给 GPU 使用这里 DEFAULT
		.BindFlags = D3D11_BIND_VERTEX_BUFFER,
		// 这块 Buffer 拿来当顶点缓冲区
		.CPUAccessFlags = 0,
		// CPU不访问
	};

	Vertex v[NUM_VERTEX]{
		{{-0.5f, 0.5f, -0.5f, 1.0f}, {0.0f, 0.0f}},
		{{ 0.5f, 0.5f, -0.5f, 1.0f}, {1.0f, 0.0f}},
		{{-0.5f,-0.5f, -0.5f, 1.0f}, {0.0f, 1.0f}},

		{{-0.5f,-0.5f, -0.5f, 1.0f}, {0.0f, 1.0f}},
		{{ 0.5f,-0.5f, -0.5f, 1.0f}, {1.0f, 1.0f}},
		{{ 0.5f, 0.5f, -0.5f, 1.0f}, {1.0f, 0.0f}},
	};
	D3D11_SUBRESOURCE_DATA sd{};
	sd.pSysMem = v;
	// 要传给 GPU 的数据就是 v
	//“初始化 GPU 资源时，初始数据在哪里”的说明结构体。

	// 安全检查，并且这里创建顶点缓冲区 bd规则 sd数据 放到这个地址
	HRESULT hr = device->CreateBuffer(&bd, &sd, &g_VertexBuffer);
	if(FAILED(hr))
	{
		MessageBoxW(nullptr, L"创建顶点缓冲区失败。", L"Direct3D 错误", MB_OK | MB_ICONERROR);
		return false;
	}


	D3D11_DEPTH_STENCIL_DESC depth_stencil_desc{};// 深度测试规则
	depth_stencil_desc.DepthEnable = TRUE; // 开启深度测试
	depth_stencil_desc.DepthFunc = D3D11_COMPARISON_LESS; // 小的在前面
	depth_stencil_desc.StencilEnable = FALSE;// 不使用模板测试
	depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;// 深度写入全部
	hr = device->CreateDepthStencilState(&depth_stencil_desc, &g_DepthStencilState);
	if (FAILED(hr))
	{
		MessageBoxW(nullptr, L"创建深度状态失败。", L"Direct3D 错误", MB_OK | MB_ICONERROR);
		Cube_Finalize();
		return false;
	}
	// 把上述规则创建成 D3D 对象

	D3D11_RASTERIZER_DESC rasterizer_desc{}; // 光栅化规则
	rasterizer_desc.FillMode = D3D11_FILL_SOLID; // 实心
	rasterizer_desc.CullMode = D3D11_CULL_BACK; // 背面剔除
	hr = device->CreateRasterizerState(&rasterizer_desc, &g_RasterizerState);
	if (FAILED(hr))
	{
		MessageBoxW(nullptr, L"创建光栅化状态失败。", L"Direct3D 错误", MB_OK | MB_ICONERROR);
		Cube_Finalize();
		return false;
	}
	// 把上述规则创建成 D3D 对象

	return true;
}
void Cube_Finalize()
{
	SAFE_RELEASE(g_DepthStencilState);
	SAFE_RELEASE(g_RasterizerState);
	SAFE_RELEASE(g_VertexBuffer);
}
void Cube_Draw()
{
	if (g_VertexBuffer == nullptr || g_RasterizerState == nullptr ||
		g_DepthStencilState == nullptr || Direct3D_GetDeviceContext() == nullptr)
	{
		return;
	}

	// Shader设定
	Shader3D_Begin();

	Direct3D_GetDeviceContext()->OMSetDepthStencilState(g_DepthStencilState, 0);
	Direct3D_GetDeviceContext()->RSSetState(g_RasterizerState);
	// 设置GPU的深度测试状态 让GPU现在就用这套规则

	// 世界坐标变换行列 用Shader设置
	Shader3D_SetWorldMatrix(XMMatrixIdentity());
	// View矩阵变换行列
	XMMATRIX view = XMMatrixLookAtLH(
		XMVectorSet(0.0f, 0.0f, -3.0f, 1.0f), // 相机位置
		XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), // 相机看向的目标点
		XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f)  // 相机的上方向
	);
	Shader3D_SetViewMatrix(view);
	// projection矩阵变换行列 投影矩阵是透视投影矩阵 60度视角
	XMMATRIX proj = XMMatrixPerspectiveFovLH(XMConvertToRadians(60.0f), (float)(SCREEN_WIDTH / SCREEN_HEIGHT), 0.1f, 100.0f);
	Shader3D_SetProjectionMatrix(proj);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	// stride是每个顶点的大小 offset是从第几个顶点开始
	Direct3D_GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);
	// 把顶点缓冲区绑定到GPU上 0是槽位 1是数量 &g_VertexBuffer是顶点缓冲区的地址
	// &stride是每个顶点的大小 &offset是从第几个顶点开始 就是刚刚算出来的

	Direct3D_GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// 告诉GPU 这些顶点是三角形列表

	Direct3D_GetDeviceContext()->Draw(NUM_VERTEX, 0);
	// 告诉GPU 画多少个顶点 NUM_VERTEX是数量 0是从第几个顶点开始
}
