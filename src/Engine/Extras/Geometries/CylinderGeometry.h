#pragma once
#include "platform.h"
#include "Geometry.h"
#include "Vector.h"
#include "Face.h"

class CCylinderGeometry : public CGeometry
{
public:
	CCylinderGeometry(float radiusTop = 0.5f, float radiusBottom = 0.5f, float height = 2, int segmentsRadius = 32, int segmentsHeight = 1, bool openEnded = false) 
	{
		int segmentsX = segmentsRadius;
		int segmentsY = segmentsHeight;

		vector<vector<int>> verts;
		vector<vector<Vector2>> uvs;

		int y;

		for ( y = 0; y <= segmentsY; y++ ) 
		{
			vector<int> verticesRow;
			vector<Vector2> uvsRow;

			float v = (float)y / segmentsY;
			float radius = v * ( radiusBottom - radiusTop ) + radiusTop;

			for ( int x = 0; x <= segmentsX; x ++ ) 
			{
				float u = (float)x / segmentsX;

				float xpos = radius * sin( u * M_PI * 2.0f );
				float ypos = - v * height + height/2.0f;
				float zpos = radius * cos( u * M_PI * 2.0f );

				vertices.AddToTail( Vector3( xpos, ypos, zpos ) );

				verticesRow.push_back( vertices.Size() - 1 );
				uvsRow.push_back( Vector2( u, v ) );

			}

			verts.push_back( verticesRow );
			uvs.push_back( uvsRow );
		}

		for ( y = 0; y < segmentsY; y ++ ) 
		{
			for ( int x = 0; x < segmentsX; x ++ ) 
			{

				int v1 = verts[ y ][ x ];
				int v2 = verts[ y + 1 ][ x ];
				int v3 = verts[ y + 1 ][ x + 1 ];
				int v4 = verts[ y ][ x + 1 ];

				// FIXME: These normals aren't right for cones.

				Vector3 n1 = vertices[ v1 ]; n1.y = 0; n1.Normalize();
				Vector3 n2 = vertices[ v2 ]; n2.y = 0; n2.Normalize();
				Vector3 n3 = vertices[ v3 ]; n3.y = 0; n3.Normalize();
				Vector3 n4 = vertices[ v4 ]; n4.y = 0; n4.Normalize();

				Vector2 uv1 = uvs[ y ][ x ];
				Vector2 uv2 = uvs[ y + 1 ][ x ];
				Vector2 uv3 = uvs[ y + 1 ][ x + 1 ];
				Vector2 uv4 = uvs[ y ][ x + 1 ];

				Face3 face[2] = { Face3( v1, v2, v3 ), Face3( v1, v3, v4 ) };
				face[0].vertexNormals[0] = n1;
				face[0].vertexNormals[1] = n2;
				face[0].vertexNormals[2] = n3;
				face[1].vertexNormals[0] = n1;
				face[1].vertexNormals[1] = n3;
				face[1].vertexNormals[2] = n4;
				faces.AddToTail(face[0]);
				faces.AddToTail(face[1]);
			}

		}

		// top cap

		if ( !openEnded && radiusTop > 0 ) 
		{

			vertices.AddToTail(Vector3( 0, height/2.0f, 0 ) );

			for ( int x = 0; x < segmentsX; x ++ ) 
			{
				int v1 = verts[ 0 ][ x ];
				int v2 = verts[ 0 ][ x + 1 ];
				int v3 = verts.size() - 1;

				Vector3 n1 = Vector3( 0, 1, 0 );
				Vector3 n2 = Vector3( 0, 1, 0 );
				Vector3 n3 = Vector3( 0, 1, 0 );

				Vector2 uv1 = uvs[ 0 ][ x ];
				Vector2 uv2 = uvs[ 0 ][ x + 1 ];
				Vector2 uv3 = Vector2( uv2.x, 0 );

				Face3 face = Face3( v1, v2, v3 );
				face.vertexNormals[0] = n1;
				face.vertexNormals[1] = n2;
				face.vertexNormals[2] = n3;

				faces.AddToTail( face );
			}

		}

		// bottom cap

		if ( !openEnded && radiusBottom > 0 ) 
		{
			vertices.AddToTail(Vector3( 0, -height/2.0f, 0 ) );

			for ( int x = 0; x < segmentsX; x ++ ) 
			{
				int v1 = verts[ y ][ x + 1 ];
				int v2 = verts[ y ][ x ];
				int v3 = vertices.Size() - 1;

				Vector3 n1 = Vector3( 0, -1, 0 );
				Vector3 n2 = Vector3( 0, -1, 0 );
				Vector3 n3 = Vector3( 0, -1, 0 );

				Vector2 uv1 = uvs[ y ][ x ];
				Vector2 uv2 = uvs[ y ][ x + 1 ];
				Vector2 uv3 = Vector2( uv2.x, 1 );

				Face3 face = Face3( v1, v2, v3 );
				face.vertexNormals[0] = n1;
				face.vertexNormals[1] = n2;
				face.vertexNormals[2] = n3;

				faces.AddToTail( face );
			}
		}

		ComputeCentroids();
		ComputeFaceNormals();
		ComputeVertexNormals();

		this->useVertexNormals = false;
	}
};
