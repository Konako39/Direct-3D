#include <d3d11.h>
#include <DirectXMath.h>
using namespace DirectX;
#include "direct3d.h"
#include "Shader3D.h"
#include <array>
#include <filesystem>
#include <fstream>
#include <vector>


static ID3D11VertexShader* g_pVertexShader = nullptr;
static ID3D11InputLayout* g_pInputLayout = nullptr;
static ID3D11Buffer* g_pVSConstantBuffer0 = nullptr;
static ID3D11Buffer* g_pVSConstantBuffer1 = nullptr;
static ID3D11Buffer* g_pVSConstantBuffer2 = nullptr;
static ID3D11PixelShader* g_pPixelShader = nullptr;

static ID3D11DeviceContext* g_pContext = nullptr;

namespace
{
bool ReadCompiledShader(const wchar_t* file_name, std::vector<char>& bytecode)
{
	std::array<wchar_t, 32768> executable_path{};
	const DWORD path_length = GetModuleFileNameW(
		nullptr,
		executable_path.data(),
		static_cast<DWORD>(executable_path.size()));
	if (path_length == 0 || path_length >= executable_path.size())
	{
		return false;
	}

	const std::filesystem::path shader_path =
		std::filesystem::path(executable_path.data()).parent_path() / file_name;
	std::ifstream file(shader_path, std::ios::binary | std::ios::ate);
	if (!file)
	{
		return false;
	}

	const std::streampos end = file.tellg();
	if (end <= std::streampos(0))
	{
		return false;
	}

	const std::streamsize byte_count = static_cast<std::streamsize>(end);
	bytecode.resize(static_cast<size_t>(byte_count));
	file.seekg(0, std::ios::beg);
	return static_cast<bool>(file.read(bytecode.data(), byte_count));
}

void ShowShaderError(const wchar_t* message)
{
	MessageBoxW(nullptr, message, L"着色器错误", MB_OK | MB_ICONERROR);
}

void SetMatrix(ID3D11Buffer* buffer, const DirectX::XMMATRIX& matrix)
{
	if (g_pContext == nullptr || buffer == nullptr)
	{
		return;
	}

	XMFLOAT4X4 transposed{};
	XMStoreFloat4x4(&transposed, XMMatrixTranspose(matrix));
	g_pContext->UpdateSubresource(buffer, 0, nullptr, &transposed, 0, 0);
}
}

bool Shader3D_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	Shader3D_Finalize();
	if (pDevice == nullptr || pContext == nullptr)
	{
		ShowShaderError(L"Direct3D 设备或设备上下文为空。");
		return false;
	}

	std::vector<char> vertex_bytecode;
	if (!ReadCompiledShader(L"shader_vertex_3d.cso", vertex_bytecode))
	{
		ShowShaderError(L"无法读取顶点着色器。请先成功构建项目，确认 .cso 文件已生成。");
		return false;
	}

	HRESULT result = pDevice->CreateVertexShader(
		vertex_bytecode.data(), vertex_bytecode.size(), nullptr, &g_pVertexShader);
	if (FAILED(result))
	{
		ShowShaderError(L"创建顶点着色器失败。");
		Shader3D_Finalize();
		return false;
	}

	const D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	result = pDevice->CreateInputLayout(
		layout, ARRAYSIZE(layout), vertex_bytecode.data(), vertex_bytecode.size(), &g_pInputLayout);
	if (FAILED(result))
	{
		ShowShaderError(L"创建顶点输入布局失败。");
		Shader3D_Finalize();
		return false;
	}

	D3D11_BUFFER_DESC buffer_description{};
	buffer_description.ByteWidth = sizeof(XMFLOAT4X4);
	buffer_description.Usage = D3D11_USAGE_DEFAULT;
	buffer_description.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	ID3D11Buffer** matrix_buffers[] = {
		&g_pVSConstantBuffer0,
		&g_pVSConstantBuffer1,
		&g_pVSConstantBuffer2,
	};
	for (ID3D11Buffer** buffer : matrix_buffers)
	{
		result = pDevice->CreateBuffer(&buffer_description, nullptr, buffer);
		if (FAILED(result))
		{
			ShowShaderError(L"创建矩阵常量缓冲区失败。");
			Shader3D_Finalize();
			return false;
		}
	}

	std::vector<char> pixel_bytecode;
	if (!ReadCompiledShader(L"shader_pixel_3d.cso", pixel_bytecode))
	{
		ShowShaderError(L"无法读取像素着色器。请先成功构建项目，确认 .cso 文件已生成。");
		Shader3D_Finalize();
		return false;
	}

	result = pDevice->CreatePixelShader(
		pixel_bytecode.data(), pixel_bytecode.size(), nullptr, &g_pPixelShader);
	if (FAILED(result))
	{
		ShowShaderError(L"创建像素着色器失败。");
		Shader3D_Finalize();
		return false;
	}

	g_pContext = pContext;
	g_pContext->AddRef();
	return true;
}

void Shader3D_Finalize()
{
	SAFE_RELEASE(g_pPixelShader);
	SAFE_RELEASE(g_pVSConstantBuffer0);
	SAFE_RELEASE(g_pVSConstantBuffer1);
	SAFE_RELEASE(g_pVSConstantBuffer2);
	SAFE_RELEASE(g_pInputLayout);
	SAFE_RELEASE(g_pVertexShader);
	SAFE_RELEASE(g_pContext);
}

// 这三个函数用于设置世界矩阵、视图矩阵和投影矩阵
// 并将它们传递给顶点着色器的常量缓冲区 也就是UpdateSubresource函数 传递transpose矩阵到GPU
void Shader3D_SetWorldMatrix(const DirectX::XMMATRIX& matrix)
{
	SetMatrix(g_pVSConstantBuffer0, matrix);
}
void Shader3D_SetViewMatrix(const DirectX::XMMATRIX& matrix)
{
	SetMatrix(g_pVSConstantBuffer1, matrix);
}

void Shader3D_SetProjectionMatrix(const DirectX::XMMATRIX& matrix)
{
	SetMatrix(g_pVSConstantBuffer2, matrix);
}


void Shader3D_Begin()
{
	if (g_pContext == nullptr || g_pVertexShader == nullptr || g_pPixelShader == nullptr ||
		g_pInputLayout == nullptr || g_pVSConstantBuffer0 == nullptr ||
		g_pVSConstantBuffer1 == nullptr || g_pVSConstantBuffer2 == nullptr)
	{
		return;
	}

	g_pContext->VSSetShader(g_pVertexShader, nullptr, 0);
	g_pContext->PSSetShader(g_pPixelShader, nullptr, 0);
	// 设置顶点着色器和像素着色器

	g_pContext->IASetInputLayout(g_pInputLayout);
	// 设置输入布局

	g_pContext->VSSetConstantBuffers(0, 1, &g_pVSConstantBuffer0);
	g_pContext->VSSetConstantBuffers(1, 1, &g_pVSConstantBuffer1);
	g_pContext->VSSetConstantBuffers(2, 1, &g_pVSConstantBuffer2);
	// 设置常量缓冲区
}
