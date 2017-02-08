#pragma once
#include "platform.h"
#include "utils.h"

class CTilesGeometry
{
public:
	static void CreateBlock( CArray<float> &v, CArray<float> &vtex )
	{
		float V[] =  { 
			0,0,1,	
				0,0,0,
				1,0,0,
				1,0,1,
				0,1,1,	
				0,1,0,
				1,1,0,
				1,1,1,
		};

		float VTex[] =  {
				4,0,0,	7,1,0,	0,0,1,	0,		// front
				0,0,1,	7,1,0,	3,1,1,	0,

				6,0,0,	5,1,0,	1,1,1,	1,		// back
				6,0,0,	1,1,1,	2,0,1,	1,

				5,0,0,	4,1,0,	0,1,1,	2,		// left
				5,0,0,	0,1,1,	1,0,1,	2,

				7,0,0,	6,1,0,	2,1,1,	3,		// right
				7,0,0,	2,1,1,	3,0,1,	3,

				5,0,0,	6,1,0,	7,1,1,	4,		// top
				5,0,0,	7,1,1,	4,0,1,	4,

				3,1,0,	2,1,1,	1,0,1,	5,		// bottom
				3,1,0,	1,0,1,	0,0,0,	5
		};

		v.CopyArray(V, ArraySize(V));
		vtex.CopyArray(VTex, ArraySize(VTex));
	}

	static void CreateLowerBlock( CArray<float> &v, CArray<float> &vtex )
	{
		float V[] =  { 
			0,0,1,	
				0,0,0,
				1,0,0,
				1,0,1,
				0,0.5f,1,	
				0,0.5f,0,
				1,0.5f,0,
				1,0.5f,1,
		};

		float VTex[] =  {
			4,0,0,		7,1,0,	0,0,0.5f,	0,		// front
				0,0,0.5f,	7,1,0,	3,1,0.5f,	0,

				6,0,0,	5,1,0,		1,1,0.5f,	1,		// back
				6,0,0,	1,1,0.5f,	2,0,0.5f,	1,

				5,0,0,	4,1,0,		0,1,0.5f,	2,		// left
				5,0,0,	0,1,0.5f,	1,0,0.5f,	2,

				7,0,0,	6,1,0,		2,1,0.5f,	3,		// right
				7,0,0,	2,1,0.5f,	3,0,0.5f,	3,

				5,0,0,	6,1,0,	7,1,1,	4,		// top
				5,0,0,	7,1,1,	4,0,1,	4,

				3,1,0,	2,1,1,	1,0,1,	5,		// bottom
				3,1,0,	1,0,1,	0,0,0,	5
		};

		v.CopyArray(V, ArraySize(V));
		vtex.CopyArray(VTex, ArraySize(VTex));
	}

	static void CreateHigherBlock( CArray<float> &v, CArray<float> &vtex )
	{
		float V[] =  { 
			0,0.5f,1,	
				0,0.5f,0,
				1,0.5f,0,
				1,0.5f,1,
				0,1,1,	
				0,1,0,
				1,1,0,
				1,1,1,
		};

		float VTex[] =  {
			4,0,0,		7,1,0,	0,0,0.5f,	0,		// front
				0,0,0.5f,	7,1,0,	3,1,0.5f,	0,

				6,0,0,	5,1,0,		1,1,0.5f,	1,		// back
				6,0,0,	1,1,0.5f,	2,0,0.5f,	1,

				5,0,0,	4,1,0,		0,1,0.5f,	2,		// left
				5,0,0,	0,1,0.5f,	1,0,0.5f,	2,

				7,0,0,	6,1,0,		2,1,0.5f,	3,		// right
				7,0,0,	2,1,0.5f,	3,0,0.5f,	3,

				5,0,0,	6,1,0,	7,1,1,	4,		// top
				5,0,0,	7,1,1,	4,0,1,	4,

				3,1,0,	2,1,1,	1,0,1,	5,		// bottom
				3,1,0,	1,0,1,	0,0,0,	5
		};

		v.CopyArray(V, ArraySize(V));
		vtex.CopyArray(VTex, ArraySize(VTex));
	}

	static void CreateSlope( CArray<float> &v, CArray<float> &vtex )
	{
		float V[] =  { 
			0,0,1,	
				0,0,0,
				1,0,0,
				1,0,1,
				0,1,1,	
				0,1,0,
		};

		float VTex[] =  {
			0,0,1,	4,0,0,	3,1,1,	0,		// front

				5,0,0,	1,0,1,	2,1,1,	0,		// back

				5,0,0,	4,1,0,	0,1,1,	1,		// left
				5,0,0,	0,1,1,	1,0,1,	1,

				4,0,0,	5,1,0,	2,1,1,	2,		// right
				4,0,0,	2,1,1,	3,0,1,	2,

				3,1,0,	2,1,1,	1,0,1,	3,		// bottom
				3,1,0,	1,0,1,	0,0,0,	3,
		};

		v.CopyArray(V, ArraySize(V));
		vtex.CopyArray(VTex, ArraySize(VTex));
	}

	static void CreateUpperSlope( CArray<float> &v, CArray<float> &vtex )
	{
		float V[] =  { 
			0,1,1,
				0,1,0,
				1,1,0,
				1,1,1,
				0,0,1,
				0,0,0
		};


		float VTex[] =  {
			0,0,0,	3,1,0,	4,0,1,	0,		// front

				2,1,0,	1,0,0,	5,0,1,	0,		// back

				1,0,0,	0,1,0,	4,1,1,	1,		// left
				1,0,0,	4,1,1,	5,0,1,	1,

				3,0,0,	2,1,0,	5,1,1,	2,		// right
				3,0,0,	5,1,1,	4,0,1,	2,

				1,0,0,	2,1,0,	3,1,1,	3,		// top
				1,0,0,	3,1,1,	0,0,1,	3
		};

		v.CopyArray(V, ArraySize(V));
		vtex.CopyArray(VTex, ArraySize(VTex));
	}

	static void CreateStair( CArray<float> &v, CArray<float> &vtex )
	{
		float V[] =  { 
			0,0,1,
				0,0,0,
				1,0,0,
				1,0,1,
				0.5f,0.5f,1,
				0.5f,0.5f,0,
				1,0.5f,0,
				1,0.5f,1,
				0,1,1,
				0,1,0,
				0.5f,1,0,
				0.5f,1,1
		};

		float VTex[] =  {
				0,0,1,	8,0,0,			11,0.5f,0,		0,	// front
				0,0,1,	11,0.5f,0,		4,0.5f,0.5f,	0,
				0,0,1,	4,0.5f,0.5f,	7,1,0.5f,		0,
				0,0,1,	7,1,0.5f,		3,1,1,			0,

				1,0,1,	10,0.5f,0,		9,0,0,			0,	// back
				1,0,1,	5,0.5f,0.5f,	10,0.5f,0,		0,
				1,0,1,	6,1,0.5f,		5,0.5f,0.5f,	0,
				1,0,1,	2,1,1,			6,1,0.5f,		0,

				9,0,0,	8,1,0,	0,1,1,					1,	// left
				9,0,0,	0,1,1,	1,0,1,					1,

				11,0,0,		10,1,0,	5,1,0.5f,			2,	// right up
				11,0,0,		5,1,0.5f, 4,0,0.5f,			2,

				7,0,0.5f,	6,1,0.5f,	2,1,1,			2,	// right down
				7,0,0.5f,	2,1,1,		3,0,1,			2,	

				9,1,0,	10,1,0.5f,	11,0,0.5f,			3,	// top left
				9,1,0,	11,0,0.5f,	8,0,0,				3,

				5,1,0.5f,	6,1,1,	7,0,1,				3,	// top right
				5,1,0.5f,	7,0,1,	4,0,0.5f,			3,

				3,1,0,	2,1,1,	1,0,1,					4,	// bottom
				3,1,0,	1,0,1,	0,0,0,					4
		};

		v.CopyArray(V, ArraySize(V));
		vtex.CopyArray(VTex, ArraySize(VTex));
	}

};