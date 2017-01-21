

//cbuffer SimDetails : register( b1 )
//{
//	float4 Box;
//	float tscale;
//	int2	dimensions;
//}

cbuffer SimDetails : register(b3)
{
	int offset;
	int limit;
}
cbuffer PSCB : register( b0 )
{
	int height;
}


Texture2D<float4> tx2Data : register( t0);
RWTexture2D<float4> tx2DataOut : register( u0); 

[numthreads(8, 8, 1)]
void CSSortH(uint3 DTid : SV_DispatchThreadID)
{
	//tx2DataOut[DTid.xy] = (float4)0;
	uint2 a1 = uint2(DTid.x * 2 + offset, DTid.y);
	uint2 a2 = a1;
	a2.x += 1;

	float4 ref1 = tx2Data.Load(uint3(a1, 0));
	float4 ref2 = tx2Data.Load(uint3(a2, 0));
	if ((ref1.z > ref2.z) && (a2.x != limit))
	{
		tx2DataOut[a1] = ref2;
		tx2DataOut[a2] = ref1;
	}
	else
	{
		tx2DataOut[a1] = ref1;
		tx2DataOut[a2] = ref2;
	}
}