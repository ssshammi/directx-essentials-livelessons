cbuffer CBufferPerFrame
{
	float3 AmbientColor;
	float3 LightDirection;
	float3 LightColor;
}

Texture2D ColorMap;
Texture2D NormalMap;
SamplerState TextureSampler;

struct VS_OUTPUT
{
	float4 Position: SV_Position;
	float2 TextureCoordinates : TEXCOORD;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float3 Binormal : BINORMAL;
};

float4 main(VS_OUTPUT IN) : SV_TARGET
{
	float3 sampledNormal = (2 * NormalMap.Sample(TextureSampler, IN.TextureCoordinates).xyz) - 1.0; // Map normal from [0..1] to [-1..1]
	float3x3 tbn = float3x3(IN.Tangent, IN.Binormal, IN.Normal);
	sampledNormal = mul(sampledNormal, tbn); // Transform normal to world space

	float4 color = ColorMap.Sample(TextureSampler, IN.TextureCoordinates);
	float3 ambient = color.rgb * AmbientColor;
	float n_dot_l = dot(sampledNormal, LightDirection);
	float3 diffuse = (n_dot_l > 0 ? color.rgb * n_dot_l * LightColor : (float3)0);

	return float4(saturate(ambient + diffuse), color.a);
}