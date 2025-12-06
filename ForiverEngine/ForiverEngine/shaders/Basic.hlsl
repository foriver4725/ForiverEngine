struct VSInput
{
    float4 pos : POSITION;
};

struct V2P
{
    float4 pos : SV_POSITION;
};

struct PSOutput
{
    float4 color : SV_TARGET;
};

V2P VSMain(VSInput input)
{
    V2P output;
    
    output.pos = input.pos;
    
    return output;
}

PSOutput PSMain(V2P input)
{
    PSOutput output;
    
    output.color = 1;
    
    return output;
}
