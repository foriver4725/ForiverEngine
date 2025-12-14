cbuffer _0 : register(b0)
{
    float4x4 _Matrix_M_IT;
    float4x4 _Matrix_MVP;
}

Texture2D<float4> _AlbedoTexture : register(t0);
SamplerState _Sampler : register(s0);

struct VSInput
{
    float4 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

struct V2P
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

struct PSOutput
{
    float4 color : SV_TARGET;
};

V2P VSMain(VSInput input)
{
    V2P output;
    
    output.pos = mul(_Matrix_MVP, input.pos);
    output.normal = mul((float3x3) _Matrix_M_IT, input.normal);
    output.uv = input.uv;
    
    return output;
}

PSOutput PSMain(V2P input)
{
    PSOutput output;
    
    output.color = _AlbedoTexture.Sample(_Sampler, input.uv);
    
    return output;
}
