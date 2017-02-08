#pragma once
#include "platform.h"
#include "Vector.h"
#include "Utils.h"

class Face3
{
public:
	TYPE("Face3");

	Face3() 
	{
		a = b = c = -1;
	}
	Face3(int a, int b, int c, Vector3 normal = Vector3(), SRGBA color = SRGBA(), int index = -1)
	{
		ZeroMemory(this, 0, sizeof(this));
		this->a = a;
		this->b = b;
		this->c = c;
		this->normal = normal;
		this->color = color;
		this->materialIndex = index;
		this->texcoord[0] = 
		this->texcoord[1] = 
		this->texcoord[2] = Vector2();
		this->vertexNormals[0] = 
		this->vertexNormals[1] = 
		this->vertexNormals[2] = Vector3();
	}

	int operator[](int index) 
	{ 
		if (index == 0) return a;
		if (index == 1) return b;
		if (index == 2) return c;
		return -1;
	}

public:
	union 
	{
		struct 
		{
			int			a, b, c;				// indexes of vertices
		};
		struct 
		{
			int			index[3];
		};
	};

	Vector3		normal;
	Vector2		texcoord[3];
	Vector3		vertexNormals[3];
	SRGBA		color;
	SRGBA		vertexColors[3];
	Vector3		vertexTangets[3];
	int			materialIndex;
	Vector3		centroid;
};

