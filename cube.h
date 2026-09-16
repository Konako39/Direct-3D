#pragma once
#ifndef CUBE_H
#define CUBE_H
#include <DirectXMath.h>

bool Cube_Initialize();
void Cube_Finalize();
void Cube_Draw(const DirectX::XMMATRIX& world);

#endif // CUBE_H
