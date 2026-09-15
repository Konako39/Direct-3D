struct PS_IN
{
    float4 posH : SV_POSITION;
    float2 uv : TEXCOORD0;
};

float4 main(PS_IN input) : SV_TARGET
{
    return float4(input.uv.x, input.uv.y, 0.8f, 1.0f);
}
