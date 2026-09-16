#include <d3d11.h>
#include <DirectXMath.h>
#include "shader3d.h"
#include "cube.h"
#include "Src/Core/direct3d.h"
#include <Windows.h>
using namespace DirectX;

static ID3D11Buffer* g_VertexBuffer = nullptr;

// 顶点构造体
struct Vertex
{
	XMFLOAT4 position; // 位置
	XMFLOAT4 color;    // 顶点颜色（RGBA）
};

static constexpr int GRID_COUNT_X{ 10 };
static constexpr int GRID_COUNT_Z{ 10 };
static constexpr int GRID_LINE_COUNT_X{ GRID_COUNT_X + 1 };
static constexpr int GRID_LINE_COUNT_Z{ GRID_COUNT_Z + 1 };
static constexpr int NUM_VERTEX = { GRID_LINE_COUNT_X * 2 + GRID_LINE_COUNT_Z * 2}; // 表格

bool Grid_Initialize() {

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

	Vertex v[NUM_VERTEX]{};
	
	constexpr float GRID_SIZE_X{ 1.0f * GRID_COUNT_X };
	constexpr float GRID_SIZE_Z{ 1.0f * GRID_COUNT_Z };
	constexpr float START_X{ GRID_SIZE_X * -0.5f };
	constexpr float START_Z{ GRID_SIZE_Z * -0.5f };

	int index = 0;
	for (int i = 0 ; i < GRID_LINE_COUNT_X ; i++) {
		float x = START_X + i * 1.0f;
		v[index] = { {x,0.0f,START_Z,1.0f},{0.5f,1.0f,0.0f,1.0f} };
		v[index+1] = { {x,0.0f,START_Z + GRID_SIZE_Z,1.0f},{0.5f,1.0f,0.0f,1.0f} };
		index += 2;
	}
	for (int i = 0; i < GRID_LINE_COUNT_Z; i++) {
		float z = START_Z + i * 1.0f;
		v[index] = { {START_X,0.0f,z,1.0f},{0.5f,1.0f,0.0f,1.0f} };
		v[index+1] = { {START_X + GRID_SIZE_X,0.0f,z,1.0f},{0.5f,1.0f,0.0f,1.0f} };
		index += 2;
	}



	D3D11_SUBRESOURCE_DATA sd{};
	sd.pSysMem = v;
	// 要传给 GPU 的数据就是 v
	//“初始化 GPU 资源时，初始数据在哪里”的说明结构体。

	// 安全检查，并且这里创建顶点缓冲区 bd规则 sd数据 放到这个地址
	HRESULT hr = device->CreateBuffer(&bd, &sd, &g_VertexBuffer);
	if (FAILED(hr))
	{
		MessageBoxW(nullptr, L"创建顶点缓冲区失败。", L"Direct3D 错误", MB_OK | MB_ICONERROR);
		return false;
	}
	return true;

}

void Grid_Finalize()
{
	SAFE_RELEASE(g_VertexBuffer);
}

void Grid_Draw() {
	// 世界坐标变换行列 用Shader设置
	Shader3D_SetWorldMatrix(XMMatrixIdentity());

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	// stride是每个顶点的大小 offset是从第几个顶点开始
	Direct3D_GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);
	// 把顶点缓冲区绑定到GPU上 0是槽位 1是数量 &g_VertexBuffer是顶点缓冲区的地址
	// &stride是每个顶点的大小 &offset是从第几个顶点开始 就是刚刚算出来的

	Direct3D_GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	// 告诉GPU 这些顶点是三角形列表

	Direct3D_GetDeviceContext()->Draw(NUM_VERTEX, 0);
	// 告诉GPU 画多少个顶点 NUM_VERTEX是数量 0是从第几个顶点开始


}
