#include "shader.fx"

half4 sort(PS_INPUT_TEX input) : SV_Target
{
	float2 scale = float2((input.Tex.x / input.Pos.x) * 0.5f, 0);

	float4 s1 = tx2Data.Sample(samLinear, input.Tex);
	float4 s2 = tx2Data.Sample(samLinear, input.Tex + scale);

	if (s1.z > 

	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}