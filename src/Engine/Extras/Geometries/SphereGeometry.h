#pragma once
#include "platform.h"
#include "Geometry.h"
#include "Vector.h"
#include "Face.h"

class CSphereGeometry : public CGeometry
{
public:
	CSphereGeometry(float radius = 1.0f, int segmentsWidth = 32, int segmentsHeight = 24, 
		float phiStart = 0, float phiLength = M_PI*2, float thetaStart = 0, float thetaLength = M_PI)
	{
		int segmentsX = max(3, segmentsWidth);
		int segmentsY = max(2, segmentsHeight);

		vector<vector<int>> verts;
		vector<vector<Vector2>> uvs;

		this->useVertexNormals = true;
		
		for (int y=0; y<=segmentsY; y++)
		{
			vector<int> verticesRow;
			vector<Vector2> uvsRow;
			
			for (int x=0; x<=segmentsX; x++)
			{
				float u = (float)x / segmentsX;
				float v = (float)y / segmentsY;

				float xpos = -radius * cos(phiStart + u * phiLength) * sin(thetaStart + v * thetaLength);
				float ypos = radius * cos(thetaStart + v * thetaLength);
				float zpos = radius * sin(phiStart + u * phiLength) * sin(thetaStart + v * thetaLength);

				vertices.AddToTail( Vector3( xpos, ypos, zpos ));

				verticesRow.push_back(vertices.Size()-1);
				uvsRow.push_back(Vector2(u, v));
			}

			verts.push_back(verticesRow);
			uvs.push_back(uvsRow);
		}


		for (int y=0; y<segmentsY; y++)
		{
			for (int x=0; x<segmentsX; x++)
			{
				int v1 = verts[y][x+1]; 
				int v2 = verts[y][x]; 
				int v3 = verts[y+1][x]; 
				int v4 = verts[y+1][x+1]; 

				Vector3 n1 = vertices[v1]; n1.Normalize();
				Vector3 n2 = vertices[v2]; n2.Normalize();
				Vector3 n3 = vertices[v3]; n3.Normalize();
				Vector3 n4 = vertices[v4]; n4.Normalize();

				Vector2 uv1 = uvs[y][x+1];
				Vector2 uv2 = uvs[y][x];
				Vector2 uv3 = uvs[y+1][x];
				Vector2 uv4 = uvs[y+1][x+1];

				if (abs(vertices[v1].y) == radius)
				{
					Face3 face = Face3( v1, v3, v4 );
					face.vertexNormals[0] = n1;
					face.vertexNormals[1] = n3;
					face.vertexNormals[2] = n4;
					face.materialIndex = 0;
					faces.AddToTail(face);
				}
				else if (abs(vertices[v3].y) == radius)
				{
					Face3 face = Face3( v1, v2, v3 );
					face.vertexNormals[0] = n1;
					face.vertexNormals[1] = n2;
					face.vertexNormals[2] = n3;
					face.materialIndex = 0;
					faces.AddToTail(face);
				}
				else 
				{
					Face3 face[2] = { Face3( v1, v2, v3 ), Face3( v1, v3, v4 ) };
					face[0].vertexNormals[0] = n1;
					face[0].vertexNormals[1] = n2;
					face[0].vertexNormals[2] = n3;
					face[1].vertexNormals[0] = n1;
					face[1].vertexNormals[1] = n3;
					face[1].vertexNormals[2] = n4;
					face[0].materialIndex = 0;
					face[1].materialIndex = 0;
					faces.AddToTail(face[0]);
					faces.AddToTail(face[1]);
				}
			}
		}

		ComputeCentroids();
		ComputeFaceNormals();
		ComputeTangents();

		boundingSphere = radius;
		useVertexNormals = true;
	}
};