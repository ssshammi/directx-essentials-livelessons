cbuffer CBufferPerFrame
{
	float4 AmbientColor;
	float4 LightColor;
}

Texture2D ColorTexture;
SamplerState ColorSampler;

struct VS_OUTPUT
{
	float4 Position: SV_Position;
	float2 TextureCoordinate : TEXCOORD;
	float3 Normal : NORMAL;
	float3 LightDirection : LIGHTDIR;
};

float4 main(VS_OUTPUT IN) : SV_TARGET
{
	float4 OUT = (float4)0;

	float3 normal = normalize(IN.Normal);
	float3 lightDirection = normalize(IN.LightDirection);
	float n_dot_l = dot(lightDirection, normal);

	float4 color = ColorTexture.Sample(ColorSampler, IN.TextureCoordinate);
	float3 ambient = AmbientColor.rgb * color.rgb;

	float3 diffuse = (n_dot_l > 0 ? LightColor.rgb * n_dot_l * color.rgb : (float3)0);

	OUT.rgb = ambient + diffuse;
	OUT.a = color.a;

	return OUT;
}