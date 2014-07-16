cbuffer CBufferPerFrame
{
	float4 AmbientColor;
	float4 LightColor;
	float3 CameraPosition;
}

cbuffer CBufferPerObject
{
	float3 SpecularColor;
	float SpecularPower;
}

Texture2D ColorTexture;
Texture2D NormalMap;
SamplerState TrilinearSampler;

struct VS_OUTPUT
{
	float4 Position: SV_Position;
	float3 WorldPosition : WORLDPOS;
	float2 TextureCoordinate : TEXCOORD;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float3 Binormal : BINORMAL;
	float3 LightDirection : LIGHTDIR;
};

float4 main(VS_OUTPUT IN) : SV_TARGET
{
	float4 OUT = (float4)0;

	float3 sampledNormal = (2 * NormalMap.Sample(TrilinearSampler, IN.TextureCoordinate).xyz) - 1.0; // Map normal from [0..1] to [-1..1]
	float3x3 tbn = float3x3(IN.Tangent, IN.Binormal, IN.Normal);
	sampledNormal = mul(sampledNormal, tbn); // Transform normal to world space

	float3 lightDirection = normalize(IN.LightDirection);
	float3 viewDirection = normalize(CameraPosition - IN.WorldPosition);
	float n_dot_l = dot(sampledNormal, lightDirection);
	float3 halfVector = normalize(lightDirection + viewDirection);
	float n_dot_h = dot(sampledNormal, halfVector);

	float4 color = ColorTexture.Sample(TrilinearSampler, IN.TextureCoordinate);
	float4 lightCoefficients = lit(n_dot_l, n_dot_h, SpecularPower);

	float3 ambient = AmbientColor.rgb * color.rgb;
	float3 diffuse = LightColor.rgb * lightCoefficients.y * color.rgb;
	float3 specular = SpecularColor * min(lightCoefficients.z, color.w);

	OUT.rgb = ambient + diffuse + specular;
	OUT.a = 1.0f;

	return OUT;
}