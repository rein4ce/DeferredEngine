#pragma once
#include "platform.h"
#include "Geometry.h"
#include "Vector.h"
#include "Face.h"

class CMaterial;

class CPlaneGeometry : public CGeometry
{
public:
	CPlaneGeometry(
		float width = 10.0f, float height = 10.0f, int segmentsWidth = 1, int segmentsHeight = 1,
		CMaterial* const material = NULL) : CGeometry()
	{
		this->materials.Purge();

		this->width = width;
		this->height = height;
		this->segmentsX = segmentsWidth;
		this->segmentsY = segmentsHeight;

		// copy the materials
		if (material)
			this->materials.AddToTail(material);

		for (int y=0; y<segmentsHeight+1; y++)
			for (int x=0; x<segmentsWidth+1; x++)
			{
				Vector3 v;
				v.x = x * width / (float)segmentsWidth;
				v.y = 0;
				v.z = y * height / (float)segmentsHeight; 
				vertices.AddToTail(v);				
			}
		
		for (int y=0; y<segmentsHeight; y++)
			for (int x=0; x<segmentsWidth; x++)
			{
				float a = x + (segmentsWidth+1) * y;
				float b = x + (segmentsWidth+1) * (y+1);
				float c = (x+1) + (segmentsWidth+1) * (y+1);
				float d = (x+1) + (segmentsWidth+1) * y;

				Vector2 uv[] = {
					Vector2( (float)x/segmentsWidth, 1.0f - (float)y/segmentsHeight),
					Vector2( (float)x/segmentsWidth, 1.0f - (float)(y+1)/segmentsHeight),
					Vector2( (float)(x+1)/segmentsWidth, 1.0f - (float)(y+1)/segmentsHeight),
					Vector2( (float)(x+1)/segmentsWidth, 1.0f - (float)y/segmentsHeight) 	
				};
	
				Face3 face[] = 
				{
					Face3(a, b, c),
					Face3(c, d, a)
				};

				face[0].texcoord[0] = uv[0]; 
				face[0].texcoord[1] = uv[1]; 
				face[0].texcoord[2] = uv[2]; 

				face[1].texcoord[0] = uv[2]; 
				face[1].texcoord[1] = uv[3]; 
				face[1].texcoord[2] = uv[0]; 

				for (int i=0; i<2; i++) 
				{
					face[i].materialIndex = 0;
					face[i].vertexNormals[0] =
					face[i].vertexNormals[1] =
					face[i].vertexNormals[2] =
					face[i].normal = Vector3(0,1,0);					
					faces.AddToTail(face[i]);
				}						
			}
		
		ComputeCentroids();	
		ComputeTangents();
	}


	//////////////////////////////////////////////////////////////////////////
	// How many texture repetitions over the plane
	//////////////////////////////////////////////////////////////////////////
	void SetTextureScale(float x, float y)
	{
		for (int i=0; i<faces.Size(); i++)
		{
			Face3 &f = faces[i];
			for (int j=0; j<3; j++)
			{
				Vector3 &v = vertices[ f[j] ];

				float fx = v.x / width;		// 0 -> 1
				float fy = v.z / height;	// 0 -> 1

				f.texcoord[j].x = fx * x;
				f.texcoord[j].y = 1.0f - fy * y;
			}
		}
	}

public:
	float			width, height;			// size of the plane
	int				segmentsX, segmentsY;
};