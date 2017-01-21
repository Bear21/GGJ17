// Copyright (c) 2014 All Right Reserved, http://8bitbear.com/
//
// THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
// KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// <author>Jim Wheeler</author>
// <email>bear@8bitbear.com</email>
// <date>2017-01-21</date>
#include "shader.fx"
cbuffer CBEXPLODE : register(b2)
{
   float explodeRangeSetting;
}

float4 explode(PS_INPUT_TEX input) : SV_Target
{

	float4 ref = tx2Data.Sample(samLinear, input.Tex);//locs[idx[0]][idx[1]];
	float2 loc = ref.zw; //get location
	float2 vel = ref.xy;
	float4 output;

	float2 detPoint = Box.zw / 2.f;

	float2 relvec = detPoint - loc;
	float dist = sqrt(relvec.x*relvec.x + relvec.y*relvec.y);//get distance for normalisation
   float rangeSetting = 50; //explodeRangeSetting;
	float forceSetting = 10;
   float rangeFalloffSetting = 150;

	if (dist > rangeFalloffSetting)
	{
		return ref;
	}

   float multiplier = clamp((rangeFalloffSetting - dist) / (rangeFalloffSetting - rangeSetting), 0, 1);
   float force = forceSetting * (multiplier * multiplier);

	vel = -relvec * force;

	return float4(float2(vel), float2(loc));
}