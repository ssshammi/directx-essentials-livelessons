cbuffer CBufferPerFrame
{
	float4 AmbientColor;
	float4 LightColor;
	float3 LightPosition;
	float SpotLightInnerAngle;
	float SpotLightOuterAngle;
	float3 CameraPosition;
}

cbuffer CBufferPerObject
{
	float3 SpecularColor;
	float SpecularPower;
}

Texture2D ColorTexture;
SamplerState ColorSampler;

struct VS_OUTPUT
{
	float4 Position: SV_Position;
	float3 WorldPosition : WORLDPOS;
	float2 TextureCoordinate : TEXCOORD;
	float3 Normal : NORMAL;
	float Attenuation : ATTENUATION;
	float3 LightLookAt : LOOKAT;
};

float4 main(VS_OUTPUT IN) : SV_TARGET
{
	float4 OUT = (float)0;

	float3 lightDirection = normalize(LightPosition - IN.WorldPosition);
	float3 viewDirection = normalize(CameraPosition - IN.WorldPosition);

	float3 normal = normalize(IN.Normal);
	float n_dot_l = dot(normal, lightDirection);
	float3 halfVector = normalize(lightDirection + viewDirection);
	float n_dot_h = dot(normal, halfVector);
	float3 lightLookAt = normalize(IN.LightLookAt);

	float4 color = ColorTexture.Sample(ColorSampler, IN.TextureCoordinate);
	float4 lightCoefficients = lit(n_dot_l, n_dot_h, SpecularPower);

	float3 ambient = AmbientColor.rgb * color.rgb;
	float3 diffuse = LightColor.rgb * lightCoefficients.y * color.rgb * IN.Attenuation;
	float3 specular = SpecularColor * min(lightCoefficients.z, color.w) * IN.Attenuation;
		
	float lightAngle = dot(lightLookAt, lightDirection);
	float spotFactor = (lightAngle > 0.0f ? smoothstep(SpotLightOuterAngle, SpotLightInnerAngle, lightAngle) : 0.0);

	OUT.rgb = ambient + (spotFactor * (diffuse + specular));
	OUT.a = 1.0f;

	return OUT;
}