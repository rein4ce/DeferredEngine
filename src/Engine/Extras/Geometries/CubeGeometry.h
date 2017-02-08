#pragma once
#include "platform.h"
#include "Geometry.h"
#include "Array.h"
#include "Vector.h"
#include "Face.h"

class CCubeGeometry : public CGeometry
{
public:
	CCubeGeometry(
		float width = 1.0f, float height = 1.0f, float depth = 1.0f, 
		float segmentsWidth = 1.0f, float segmentsHeight = 1.0f, float segmentsDepth = 1.0f,
		CArray<CMaterial*> &materials = CArray<CMaterial*>(), bool *sides = NULL ) : CGeometry()
	{
		this->materials.Purge();

		// copy the materials
		int materialsNum = materials.Size();
		for (int i=0; i<materials.Size(); i++)
			this->materials.AddToTail(materials[i]);
		
		// copy sides
		for (int i=0; i<6; i++)
			this->sides[i] = sides ? sides[i] : true;

		if (this->sides[0]) BuildPlane(2, 1, -1, -1, depth, height, width/2.0f,		0, segmentsWidth, segmentsHeight, segmentsDepth);
		if (this->sides[1]) BuildPlane(2, 1,  1, -1, depth, height, -width/2.0f,	materialsNum == 6 ? 1 : 0, segmentsWidth, segmentsHeight, segmentsDepth);
		if (this->sides[2]) BuildPlane(0, 2,  1,  1, width, depth, height/2.0f,		materialsNum == 6 ? 2 : 0, segmentsWidth, segmentsHeight, segmentsDepth);
		if (this->sides[3]) BuildPlane(0, 2,  1, -1, width, depth, -height/2.0f,	materialsNum == 6 ? 3 : 0, segmentsWidth, segmentsHeight, segmentsDepth);
		if (this->sides[4]) BuildPlane(0, 1,  1, -1, width, height, depth/2.0f,		materialsNum == 6 ? 4 : 0, segmentsWidth, segmentsHeight, segmentsDepth);
		if (this->sides[5]) BuildPlane(0, 1, -1, -1, width, height, -depth/2.0f,	materialsNum == 6 ? 5 : 0, segmentsWidth, segmentsHeight, segmentsDepth);

		ComputeCentroids();		
		MergeVertices();
		ComputeBoudingSphere();
		ComputeBoundingBox();	
		ComputeTangents();
	}

protected:
	void BuildPlane(int u, int v, int udir, int vdir, float width, float height, float depth, int material, float segmentsWidth, float segmentsHeight, float segmentsDepth )
	{
		int w = 0;
		int gridX = segmentsWidth;
		int gridY = segmentsHeight;
		int offset = vertices.Size();
		
		if ((u == 0 && v == 1) || (u == 1 && v == 0))
		{
			w = 2;
		}
		else if ((u == 0 && v == 2) || (u == 2 && v == 0))
		{
			w = 1;
			gridY = segmentsDepth;
		}
		else if ((u == 2 && v == 1) || (u == 1 && v == 2))
		{
			w = 0;
			gridX = segmentsDepth;
		}

		Vector3 normal;
		normal[w] = depth > 0 ? 1 : -1;
		float segWidth = width / gridX;
		float segHeight = height / gridY;

	
		for (int y=0; y<gridY+1; y++)
			for (int x=0; x<gridX+1; x++)
			{
				Vector3 vec;
				vec[u] = (x * segWidth - width/2) * udir;
				vec[v] = (y * segHeight - height/2) * vdir;
				vec[w] = depth;

				vertices.AddToTail(vec);
			}

		for (int y=0; y<gridY; y++ ) 
			for (int x=0; x<gridX; x++)
			{
				int a = x + (gridX+1)*y;
				int b = x + (gridX+1)*(y+1);
				int c = (x+1) + (gridX+1)*(y+1);
				int d = (x+1) + (gridX+1) * y;

				Vector2 uv[] = {
					 Vector2( (float)x/gridX, (float)y/gridY),
					 Vector2( (float)x/gridX, (float)(y+1)/gridY),
					 Vector2( (float)(x+1)/gridX, (float)(y+1)/gridY),
					 Vector2( (float)(x+1)/gridX, (float)y/gridY)
				};

				Face3 face[] = 
				{
					Face3(a + offset, b + offset, c + offset),
					Face3(c + offset, d + offset, a + offset)
				};

				face[0].texcoord[0] = uv[0]; 
				face[0].texcoord[1] = uv[1]; 
				face[0].texcoord[2] = uv[2]; 

				face[1].texcoord[0] = uv[2]; 
				face[1].texcoord[1] = uv[3]; 
				face[1].texcoord[2] = uv[0]; 

				for (int j=0; j<2; j++) 
				{
					face[j].normal = normal;
					face[j].vertexNormals[0] = 
					face[j].vertexNormals[1] = 
					face[j].vertexNormals[2] = normal;
					face[j].materialIndex = material;
					faces.AddToTail(face[j]);
				}	
			}			
	}

public:
	bool		sides[6];			// which sides are visible
};