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
cbuffer CBEXPLODE : register(b3)
{
   float2 detPoint;
   float explodeRangeSetting;
}

float4 explode(PS_INPUT_TEX input) : SV_Target
{

	float4 ref = tx2Data.Sample(samLinear, input.Tex);//locs[idx[0]][idx[1]];
	float2 loc = ref.zw; //get location
	float2 vel = ref.xy;
	float4 output;

	float2 relvec = detPoint - loc;
	float dist = length(relvec);//get distance for normalisation

   relvec = normalize(relvec);
   float rangeSetting = 50; //explodeRangeSetting;
	float forceSetting = 600;
   float rangeFalloffSetting = 150;

	if (dist > rangeFalloffSetting)
	{
		return ref;
	}

   float multiplier = clamp((rangeFalloffSetting - dist) / (rangeFalloffSetting - rangeSetting), 0, 1);
   
   float force = forceSetting * (multiplier * multiplier);

	vel = vel + -relvec * force;

   /*float multiplier = (rangeFalloffSetting - dist) / rangeFalloffSetting;

   float force = forceSetting * (multiplier * multiplier);

   vel = vel + -normalize(relvec) * force;*/

	return float4(float2(vel), float2(loc));
}