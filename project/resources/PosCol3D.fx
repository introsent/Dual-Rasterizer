//------------------------------------------------
// Global variables
//------------------------------------------------

//Matrices
float4x4 gWorldViewProjectionMatrix : WorldViewProjection;
float4x4 gWorldMatrix : WorldMatrix;

//Maps
Texture2D gDiffuseMap : DiffuseMap;
Texture2D gNormalMap : NormalMap;
Texture2D gSpecularMap : SpecularMap;
Texture2D gGlossinessMap : GlosinessMap;

//Lights / Materials
float3 gLightDirection : LightDirection = float3(0.577f, -0.577f, 0.577f);
float  gLightIntensity = 7.0f;
float  gShininess = 25.0f;

//Camera
float3 gCameraPosition : CameraPosition;

//Math consts
float gPI = 3.14159265358979323846f;

//Sampler state
SamplerState gSamplerState : register(s0);

//Update states
RasterizerState gRasterizerState
{
    CullMode = back;
    FrontCounterClockwise = false;
};


BlendState gBlendState
{
    BlendEnable[0] = true;
    SrcBlend = src_alpha;
    DestBlend = inv_src_alpha;
    BlendOp = add;
    SrcBlendAlpha = zero;
    DestBlendAlpha = zero;
    BlendOpAlpha = add;
    RenderTargetWriteMask[0] = 0x0F;
};

DepthStencilState gDepthStencilState
{
    DepthEnable = true;
    DepthWriteMask = true;
    DepthFunc = less;
    StencilEnable = false;
    
    StencilReadMask = 0x0F;
    StencilWriteMask = 0x0F;
    
    FrontFaceStencilFunc = always;
    BackFaceStencilFunc = always;

    FrontFaceStencilDepthFail = keep;
    BackFaceStencilDepthFail = keep;
    
    FrontFaceStencilPass = keep;
    BackFaceStencilPass = keep;
    
    FrontFaceStencilFail = keep;
    BackFaceStencilFail = keep;
};


//------------------------------------------------
// Input/Output Struct
//------------------------------------------------
struct VS_INPUT
{
	float3 Position : POSITION;
    float2 UV		: TEXCOORD;
    float3 Normal	: NORMAL;
    float3 Tangent	: TANGENT;
};

struct VS_OUTPUT
{
    float4 Position      : SV_POSITION0;
    float4 WorldPosition : WORLD;
    float2 UV            : TEXCOORD;
    float3 Normal        : NORMAL;
    float3 Tangent       : TANGENT;
};

//------------------------------------------------
// Vertex Shader
//------------------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
    
    output.Position      = mul(float4(input.Position, 1.0f), gWorldViewProjectionMatrix);
    output.WorldPosition = mul(float4(input.Position, 1.0f), gWorldMatrix);
    output.UV            = input.UV;
    output.Normal        = mul(normalize(input.Normal), (float3x3) gWorldMatrix);
    output.Tangent       = mul(normalize(input.Tangent), (float3x3) gWorldMatrix);
    
	return output;
}

//------------------------------------------------
// Pixel Shader Functions
//------------------------------------------------
float3 Phong(float3 ks, const float exp, const float3 l, const float3 v, const float3 n)
{
    float3 reflect = l - (2 * max(dot(n, l), 0.f) * n);
    const float cosAlpha = max(dot(reflect, v), 0.f);
    return ks * pow(cosAlpha, exp);
}

float3 Lambert(const float3 cd, const float kd = 1)
{
    const float3 rho = kd * cd;
    return rho / gPI;
}

float3 Shade(VS_OUTPUT input)
{
    //Normal map
    float3   binormal = normalize(cross(input.Normal, input.Tangent));
    float3x3 tangentSpaceAxis = float3x3(normalize(input.Tangent), binormal, input.Normal);
    float3   normalMapSample = gNormalMap.Sample(gSamplerState, input.UV);
    input.Normal = normalize(mul(2.f * normalMapSample - 1.f, tangentSpaceAxis));
    
    //Observed area
    float cosOfAngle = dot(input.Normal, -gLightDirection);
    if (cosOfAngle < 0.f)
        return (0.f, 0.f, 0.f);
    
    float3 observedArea = saturate(cosOfAngle);

    //Diffuse map
    float3 diffuse = Lambert(gDiffuseMap.Sample(gSamplerState, input.UV).rgb);
    
    //Glosiness map
    float3 gloss = gGlossinessMap.Sample(gSamplerState, input.UV);
    float exp = gloss.r * gShininess;
    
    //Specular map
    float3 viewDirection = normalize(input.WorldPosition.rgb - gCameraPosition);
    float3 specular = Phong(gSpecularMap.Sample(gSamplerState, input.UV).rgb, exp, -gLightDirection, viewDirection, input.Normal);
    

    //Return combined
    return float3(specular + diffuse * observedArea * gLightIntensity);

}

//------------------------------------------------
// Pixel Shader
//------------------------------------------------
float4 PS(VS_OUTPUT input) : SV_TARGET
{
    return float4(Shade(input), 1.f);
}

//------------------------------------------------
// Technique
//------------------------------------------------
technique11 DefaultTechnique
{
	pass P0
	{
        SetRasterizerState(gRasterizerState);
        SetDepthStencilState(gDepthStencilState, 0);
        SetBlendState(gBlendState, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}
