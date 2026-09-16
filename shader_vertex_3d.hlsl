cbuffer WorldBuffer : register(b0)
{
    float4x4 world;
};

cbuffer ViewBuffer : register(b1)
{
    float4x4 view;
};

cbuffer ProjectionBuffer : register(b2)
{
    float4x4 proj;
};

struct VS_IN
{
    float4 pos : POSITION;
    float4 color : COLOR0;
};

struct VS_OUT
{
    float4 posH : SV_POSITION;
    float4 color : COLOR0;
};

VS_OUT main(VS_IN input)
{
    VS_OUT output;
    output.posH = mul(mul(mul(input.pos, world), view), proj);
    output.color = input.color;
    return output;
}
