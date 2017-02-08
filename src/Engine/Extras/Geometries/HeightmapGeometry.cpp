#include "platform.h"
#include "Geometries.h"
#include "Geometry.h"
#include "Resources.h"


//////////////////////////////////////////////////////////////////////////
CHeightmapGeometry::CHeightmapGeometry(float width, float height, int segmentsWidth, int segmentsHeight, float *heightmap)
{
	Create(width, height, segmentsWidth, segmentsHeight, heightmap);
}

CHeightmapGeometry::CHeightmapGeometry(CImage *heightmap, float tileWidth, float tileHeight)
{
	Create(heightmap, tileWidth, tileHeight);
}


//////////////////////////////////////////////////////////////////////////
void CHeightmapGeometry::Create(CImage *heightmap, float tileWidth, float tileHeight)
{
	int tilesX = heightmap->width - 1;
	int tilesY = heightmap->height - 1;
	Create(tilesX * tileWidth, tilesY * tileHeight, tilesX, tilesY);

	for (int y=0; y<heightmap->height; y++)
		for (int x=0; x<heightmap->width; x++)
		{
			vertices[y * heightmap->width + x].y = heightmap->GetPixel(x, y).r;
		}

	ComputeFaceNormals();
	ComputeFaceNormals();
	ComputeTangents();
	ComputeCentroids();
}

//////////////////////////////////////////////////////////////////////////
void CHeightmapGeometry::Create(float width, float height, int segmentsWidth, int segmentsHeight, float *heightmap)
{
	this->width = width;
	this->height = height;
	this->segmentsX = segmentsWidth;
	this->segmentsY = segmentsHeight;

	// create vertices
	for (int y=0; y<segmentsHeight+1; y++)
		for (int x=0; x<segmentsWidth+1; x++)
		{
			Vector3 v;
			v.x = x * width / (float)segmentsWidth;
			v.y = 0;
			v.z = y * height / (float)segmentsHeight; 
			vertices.AddToTail(v);				
		}

	// create triangles
	for (int y=0; y<segmentsHeight; y++)
		for (int x=0; x<segmentsWidth; x++)
		{
			// node position
			float a = x + (segmentsWidth+1) * y;			// bottom-left
			float b = x + (segmentsWidth+1) * (y+1);		// top-left
			float c = (x+1) + (segmentsWidth+1) * (y+1);	// top-right
			float d = (x+1) + (segmentsWidth+1) * y;		// bottom-right

			// uv coordinates
			Vector2 uv[] = {
				Vector2( (float)x/segmentsWidth, (float)(y)/segmentsHeight),
				Vector2( (float)x/segmentsWidth, (float)(y+1)/segmentsHeight),				
				Vector2( (float)(x+1)/segmentsWidth, (float)(y+1)/segmentsHeight),
				Vector2( (float)(x+1)/segmentsWidth, (float)(y)/segmentsHeight)				
			};
			


			// triangles 
			Face3 face[2];
			if ((x + y) % 2 == 0)
			{
				face[0] = Face3(a, b, d);
				face[1] = Face3(b, c, d);
				face[0].texcoord[0] = uv[0];
				face[0].texcoord[1] = uv[1];
				face[0].texcoord[2] = uv[3];
				face[1].texcoord[0] = uv[1];
				face[1].texcoord[1] = uv[2];
				face[1].texcoord[2] = uv[3];
			}
			else
			{
				face[0] = Face3(a, b, c);
				face[1] = Face3(a, c, d);
				face[0].texcoord[0] = uv[0];
				face[0].texcoord[1] = uv[1];
				face[0].texcoord[2] = uv[2];
				face[1].texcoord[0] = uv[0];
				face[1].texcoord[1] = uv[2];
				face[1].texcoord[2] = uv[3];
			}

			// setup normals and add to faces
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

	
	// if there is a heightmap supplied, get values
	if (heightmap != NULL)
	{
		for (int y=0; y<segmentsHeight+1; y++)
			for (int x=0; x<segmentsWidth+1; x++)
			{
				vertices[y * (segmentsWidth+1) + x].y = heightmap[y * (segmentsWidth+1) + x];
			}
	}

	this->useVertexNormals = true;

	ComputeFaceNormals();
	ComputeVertexNormals();
	ComputeCentroids();
	ComputeBoundingBox();
}


