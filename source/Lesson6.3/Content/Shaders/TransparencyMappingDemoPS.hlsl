cbuffer CBufferPerFrame
{
	float3 AmbientColor;
	float3 LightDirection;
	float3 LightColor;
	float3 FogColor;
	float3 CameraPosition;
}

cbuffer CBufferPerObject
{
	float3 SpecularColor;
	float SpecularPower;
}

Texture2D ColorMap;
Texture2D SpecularMap;
Texture2D TransparencyMap;
SamplerState TextureSampler;

struct VS_OUTPUT
{
	float4 Position: SV_Position;
	float3 WorldPosition : WORLDPOS;
	float2 TextureCoordinates : TEXCOORD;
	float3 Normal : NORMAL;
	float FogAmount : FOG;
};

float4 main(VS_OUTPUT IN) : SV_TARGET
{
	if (IN.FogAmount == 1.0f)
	{
		return float4(FogColor, 1.0f);
	}

	float sampledAlpha = TransparencyMap.Sample(TextureSampler, IN.TextureCoordinates).a;
	if (sampledAlpha == 0.0f)
	{
		return float4(1.0f, 1.0f, 1.0f, 0.0f);
	}

	float3 viewDirection = normalize(CameraPosition - IN.WorldPosition);

	float3 normal = normalize(IN.Normal);	
	float n_dot_l = dot(normal, LightDirection);
	float3 halfVector = normalize(LightDirection + viewDirection);
	float n_dot_h = dot(normal, halfVector);
	float2 lightCoefficients = lit(n_dot_l, n_dot_h, SpecularPower).yz;

	float4 color = ColorMap.Sample(TextureSampler, IN.TextureCoordinates);
	float specularClamp = SpecularMap.Sample(TextureSampler, IN.TextureCoordinates).x;	

	float3 ambient = AmbientColor * color.rgb;
	float3 diffuse = color.rgb * lightCoefficients.x * LightColor;
	float3 specular = min(lightCoefficients.y, specularClamp) * SpecularColor;

	return float4(lerp(saturate(ambient + diffuse + specular), FogColor, IN.FogAmount), sampledAlpha);
}