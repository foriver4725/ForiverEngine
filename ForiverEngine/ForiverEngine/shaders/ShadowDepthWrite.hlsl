cbuffer _0 : register(b0)
{
    float4x4 _Matrix_MVP;
}

struct VSInput
{
    float4 pos : POSITION;
    float2 uv : TEXCOORD0;
};

struct V2P
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

struct PSOutput
{
    float depth : SV_TARGET;
};

V2P VSMain(VSInput input)
{
    V2P output;
    
    output.pos = mul(_Matrix_MVP, input.pos);
    output.uv = input.uv; // 必要ないけど、一応やっておく
    
    return output;
}

PSOutput PSMain(V2P input)
{
    PSOutput output;
    
    output.depth = input.pos.z;
    
    return output;
}
