// Copyright (c) 2014 All Right Reserved, http://8bitbear.com/
//
// THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
// KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// <author>Stephen Wheeler</author>
// <email>bear@8bitbear.com</email>
// <date>2014-01-15</date>

#pragma once
#include <d3d11.h>
#include <DirectXMath.h>

#define MAX_EXPLOSIONS 6
enum InputMessages
{
	InputMessages_pull,
	InputMessages_push,
	InputMessages_middlemouse,
	InputMessages_reset,
	InputMessages_halt,
	InputMessages_explode,
};
#pragma pack(push, 1) // ensure no padding
__declspec(align(4)) struct SimControl
{
	int inputLow;
	int inputHigh;
	float mousePosX;
	float mousePosY;
};

struct SimInput
{
	float timeP;
	unsigned int numControl;
	int reserved1, reserved2;
	SimControl controlInput[16];
};

struct ExplosionDelayedData
{
   ExplosionDelayedData() : 
      timePDeadline(FLT_MAX)
   {
   }
   ExplosionDelayedData(float x, float y, float time) :
      detPoint(x, y), timePDeadline(time)
   {
   }
   void Reset()
   {
      timePDeadline = FLT_MAX;
   }
   DirectX::XMFLOAT2 detPoint;
	float timePDeadline;
};
#pragma pack(pop)