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
cbuffer SimInput : register(b2)
{
   float timeP;
};
float4 sandpit(PS_INPUT_TEX input) : SV_Target
{

	float4 ref = tx2Data.Sample(samLinear, input.Tex);//locs[idx[0]][idx[1]];
	float2 loc = ref.zw; //get location
	float2 vel = ref.xy;
	float4 output;

	if (loc.x > 200 || loc.x < 100)
	{
		return ref;
	}

   if (vel.x > 0)
   {
      vel.x = -vel.x;
   }
   else
   {
      vel -= vel*3.5*timeP;
   }
	return float4(float2(vel), float2(loc));
}