#pragma once
#include "Vector.h"

struct SPrimitive
{

};

struct STriangle : SPrimitive
{
	Vector3 v[3];
	Vector3 normal;

	void CalculateNormal()
	{
		normal = (v[1] - v[0]).Cross(v[2]-v[0]).Normalize();
	}
};

struct SIndexedTriangle : SPrimitive
{
	int v[3];
	Vector3 normal;
};

struct SBox : SPrimitive
{
	Vector3 pos;
	Vector3 size;
};

struct SRay : SPrimitive
{
	Vector3 origin;
	Vector3 dir;
};


