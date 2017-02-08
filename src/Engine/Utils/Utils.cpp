#include "platform.h"
#include "Utils.h"
#include "Geometry.h"
#include "Mesh.h"

//////////////////////////////////////////////////////////////////////////
// Vector3 Implementation
//////////////////////////////////////////////////////////////////////////
Vector3& Vector3::SetRotationFromMatrix( Matrix4& m ) 
{
	y = -atan2( m.n13, m.n33 ) - M_PI;
	x = -atan2( m.n22, m.n32 ) + M_PI_2;
	z = 0;//atan2( m.n22, m.n21 ) - M_PI_2;

	return *this;
}

//////////////////////////////////////////////////////////////////////////
// File utils
//////////////////////////////////////////////////////////////////////////
vector<string> GetFilesInDirectory(char *dirname)
{
	vector<string> ret;
	DIR *dir;
	struct dirent *ent;
	dir = opendir (dirname);
	if (dir != NULL) {

		/* print all the files and directories within directory */
		while ((ent = readdir (dir)) != NULL) {
			if (S_ISREG(ent->d_type))
				ret.push_back(string(ent->d_name));
		}
		closedir (dir);
	} else {
		/* could not open directory */
		perror ("");
		Error("ERROR: Could not open directory %s",dirname);		
	}
	return ret;
}

vector<string> GetDirectoriesInDirectory(char *dirname)
{
	vector<string> ret;
	DIR *dir;
	struct dirent *ent;
	dir = opendir (dirname);
	if (dir != NULL) {

		/* print all the files and directories within directory */
		while ((ent = readdir (dir)) != NULL) {
			if (S_ISDIR(ent->d_type))
				ret.push_back(string(ent->d_name));
		}
		closedir (dir);
	} else {
		/* could not open directory */
		perror ("");
		Error("ERROR: Could not open directory %s",dirname);		
	}
	return ret;
}

//////////////////////////////////////////////////////////////////////////
// String utils
//////////////////////////////////////////////////////////////////////////
string replace(const string& sData,
			   const string& sFrom,
			   const string& sTo)
{
	string sNew = sData;

	if (! sNew.empty())
	{
		string::size_type toLen = sTo.length();
		string::size_type frLen = sFrom.length();
		string::size_type loc = 0;

		while (string::npos != (loc = sNew.find(sFrom, loc)))
		{
			sNew.replace(loc, frLen, sTo);
			loc += toLen;

			if (loc >= sNew.length())
				break;
		}
	}

	return sNew;
}




//////////////////////////////////////////////////////////////////////////
void TraverseGrid2D( float startX, float startY, float endX, float endY, float mapWidth, float mapHeight, float gridSize, OUT CArray<Vector2> &ret )
{	
	startX += mapWidth / 2;
	startY += mapHeight / 2;
	endX += mapWidth / 2;
	endY += mapHeight / 2;

	startX /= gridSize;
	startY /= gridSize;
	endX /= gridSize;
	endY /= gridSize;

	// calculate the direction of the ray (linear algebra)
	float dirX = ( endX - startX );
	float dirY = ( endY - startY );
	float length = (float)sqrt( dirX * dirX + dirY * dirY );
	dirX /= length; // normalize the direction vector
	dirY /= length;



	float tDeltaX = 1.0f / abs( dirX ); // how far we must move in the ray direction before we encounter a new voxel in x-direction
	float tDeltaY = 1.0f / abs( dirY ); // same but y-direction

	// start voxel coordinates
	int x = (int)startX;  // use your transformer function here
	int y = (int)startY;

	// end voxel coordinates
	int endX1 = (int)endX;
	int endY1 = (int)endY;

	if ( startX == endX && startY == endY )
	{
		ret.AddToTail( Vector2( x, y ) );
		return;
	}

	// decide which direction to start walking in
	int stepX = (int)sign( dirX );
	int stepY = (int)sign( dirY );

	float tMaxX, tMaxY;
	// calculate distance to first intersection in the voxel we start from
	if ( dirX < 0 )
	{
		tMaxX = ( x * 1.0f - startX ) / dirX;
	}
	else
	{
		tMaxX = ( ( x + 1 ) * 1.0f - startX ) / dirX;
	}

	if ( dirY < 0 )
	{
		tMaxY = ( y * 1.0f - startY ) / dirY;
	}
	else
	{
		tMaxY = ( ( y + 1 ) * 1.0f - startY ) / dirY;
	}

	// check if first is occupied
	ret.AddToTail( Vector2( x, y ) );

	bool reachedX = false, reachedY = false;

	while ( true )
	{
		if ( tMaxX < tMaxY )
		{
			tMaxX += tDeltaX;
			x += stepX;
		}
		else
		{
			tMaxY += tDeltaY;
			y += stepY;
		}
		ret.AddToTail( Vector2( x, y ) );

		if ( stepX > 0.0f )
		{
			if ( x >= endX1 )
			{
				reachedX = true;
			}
		}
		else if ( x <= endX1 )
		{
			reachedX = true;
		}

		if ( stepY > 0.0f )
		{
			if ( y >= endY1 )
			{
				reachedY = true;
			}
		}
		else if ( y <= endY1 )
		{
			reachedY = true;
		}

		if ( reachedX && reachedY )
		{
			break;
		}
	}

	return;
}


//////////////////////////////////////////////////////////////////////////
void TraverseGrid3D( float startX, float startY, float startZ, float endX, float endY, FLOAT endZ, float mapWidth, float mapHeight, float mapDepth, float gridSize, OUT CArray<Vector3> &ret )
{	
	startX += mapWidth / 2;
	startY += mapHeight / 2;
	startZ += mapDepth / 2;
	endX += mapWidth / 2;
	endY += mapHeight / 2;
	endZ += mapDepth / 2;

	startX /= gridSize;
	startY /= gridSize;
	startZ /= gridSize;
	endX /= gridSize;
	endY /= gridSize;
	endZ /= gridSize;

	// calculate the direction of the ray (linear algebra)
	float dirX = ( endX - startX );
	float dirY = ( endY - startY );
	float dirZ = ( endZ - startZ );
	float length = (float)sqrt( dirX * dirX + dirY * dirY + dirZ * dirZ );
	dirX /= length; // normalize the direction vector
	dirY /= length;
	dirZ /= length;


	float tDeltaX = 1.0f / abs( dirX ); // how far we must move in the ray direction before we encounter a new voxel in x-direction
	float tDeltaY = 1.0f / abs( dirY ); // same but y-direction
	float tDeltaZ = 1.0f / abs( dirZ );

	// start voxel coordinates
	int x = (int)startX;  // use your transformer function here
	int y = (int)startY;
	int z = (int)startZ;

	// end voxel coordinates
	int endX1 = (int)endX;
	int endY1 = (int)endY;
	int endZ1 = (int)endZ;

	if ( startX == endX && startY == endY && startZ == endZ )
	{
		ret.AddToTail( Vector3( x, y, z ) );
		return;
	}

	// decide which direction to start walking in
	int stepX = (int)sign( dirX );
	int stepY = (int)sign( dirY );
	int stepZ = (int)sign( dirZ );

	float tMaxX, tMaxY, tMaxZ;
	// calculate distance to first intersection in the voxel we start from
	if ( dirX < 0 )
	{
		tMaxX = ( x * 1.0f - startX ) / dirX;
	}
	else
	{
		tMaxX = ( ( x + 1 ) * 1.0f - startX ) / dirX;
	}

	if ( dirY < 0 )
	{
		tMaxY = ( y * 1.0f - startY ) / dirY;
	}
	else
	{
		tMaxY = ( ( y + 1 ) * 1.0f - startY ) / dirY;
	}

	if ( dirZ < 0 )
	{
		tMaxZ = ( z * 1.0f - startZ ) / dirZ;
	}
	else
	{
		tMaxZ = ( ( z + 1 ) * 1.0f - startZ ) / dirZ;
	}

	// check if first is occupied
	ret.AddToTail( Vector3( x, y, z ) );

	bool reachedX = false, reachedY = false, reachedZ = false;

	while ( true )
	{
		if ( tMaxX < tMaxY )
		{
			if ( tMaxX < tMaxZ )
			{
				tMaxX += tDeltaX;
				x += stepX;
			}
			else
			{
				tMaxZ += tDeltaZ;
				z += stepZ;
			}
		}
		else
		{
			if ( tMaxY < tMaxZ )
			{
				tMaxY += tDeltaY;
				y += stepY;
			}
			else
			{
				tMaxZ += tDeltaZ;
				z += stepZ;
			}
		}		
		ret.AddToTail( Vector3( x, y, z ) );


		if ( stepX > 0.0f )
		{
			if ( x >= endX1 )
			{
				reachedX = true;
			}
		}
		else if ( x <= endX1 )
		{
			reachedX = true;
		}

		if ( stepY > 0.0f )
		{
			if ( y >= endY1 )
			{
				reachedY = true;
			}
		}
		else if ( y <= endY1 )
		{
			reachedY = true;
		}

		if ( stepZ > 0.0f )
		{
			if ( z >= endZ1 )
			{
				reachedZ = true;
			}
		}
		else if ( z <= endZ1 )
		{
			reachedZ = true;
		}

		if (sqrt((float)((x-startX)*(x-startX) + (y-startY)*(y-startY) + (z-startZ)*(z-startZ))) > length+1) break;

		//if (ret.Size() > (length + )) break;

		if ( reachedX && reachedY && reachedZ )
		{
			break;
		}
	}

	return;
}

//////////////////////////////////////////////////////////////////////////
void TextureBlock(CMesh *mesh, 
				  float fx, float fy, float fw, float fh,
				  float bx, float by, float bw, float bh,
				  float lx, float ly, float lw, float lh,
				  float rx, float ry, float rw, float rh,
				  float ux, float uy, float uw, float uh,
				  float dx, float dy, float dw, float dh
				  )
{
	float x1,y1,x2,y2;
	float u = 1.0f / 16.0f;
	float v = 1.0f / 8.0f;

	// front
	x1 = x2 = fx; y1 = y2 = fy; x2 += fw; y2 += fh;
	mesh->geometry->faces[10].texcoord[0] = Vector2(x2*u, y1*v);
	mesh->geometry->faces[10].texcoord[1] = Vector2(x2*u, y2*v);
	mesh->geometry->faces[10].texcoord[2] = Vector2(x1*u, y2*v);

	mesh->geometry->faces[11].texcoord[1] = Vector2(x1*u, y1*v);
	mesh->geometry->faces[11].texcoord[2] = Vector2(x2*u, y1*v);
	mesh->geometry->faces[11].texcoord[0] = Vector2(x1*u, y2*v);

	// back
	x1 = x2 = bx; y1 = y2 = by; x2 += bw; y2 += bh;
	mesh->geometry->faces[8].texcoord[0] = Vector2(x2*u, y1*v);
	mesh->geometry->faces[8].texcoord[1] = Vector2(x2*u, y2*v);
	mesh->geometry->faces[8].texcoord[2] = Vector2(x1*u, y2*v);

	mesh->geometry->faces[9].texcoord[1] = Vector2(x1*u, y1*v);
	mesh->geometry->faces[9].texcoord[2] = Vector2(x2*u, y1*v);
	mesh->geometry->faces[9].texcoord[0] = Vector2(x1*u, y2*v);

	// left
	x1 = x2 = lx; y1 = y2 = ly; x2 += lw; y2 += lh;
	mesh->geometry->faces[0].texcoord[0] =Vector2(x2*u, y1*v);
	mesh->geometry->faces[0].texcoord[1] =Vector2(x2*u, y2*v);
	mesh->geometry->faces[0].texcoord[2] =Vector2(x1*u, y2*v);

	mesh->geometry->faces[1].texcoord[1] =Vector2(x1*u, y1*v);
	mesh->geometry->faces[1].texcoord[2] =Vector2(x2*u, y1*v);
	mesh->geometry->faces[1].texcoord[0] =Vector2(x1*u, y2*v);

	// right
	x1 = x2 = rx; y1 = y2 = ry; x2 += rw; y2 += rh;
	mesh->geometry->faces[2].texcoord[0] = Vector2(x2*u, y1*v);
	mesh->geometry->faces[2].texcoord[1] = Vector2(x2*u, y2*v);
	mesh->geometry->faces[2].texcoord[2] = Vector2(x1*u, y2*v);

	mesh->geometry->faces[3].texcoord[1] = Vector2(x1*u, y1*v);
	mesh->geometry->faces[3].texcoord[2] = Vector2(x2*u, y1*v);
	mesh->geometry->faces[3].texcoord[0] = Vector2(x1*u, y2*v);

	// up
	x1 = x2 = ux; y1 = y2 = uy; x2 += uw; y2 += uh;
	mesh->geometry->faces[4].texcoord[0] = Vector2(x2*u, y1*v);
	mesh->geometry->faces[4].texcoord[1] = Vector2(x2*u, y2*v);
	mesh->geometry->faces[4].texcoord[2] = Vector2(x1*u, y2*v);

	mesh->geometry->faces[5].texcoord[1] = Vector2(x1*u, y1*v);
	mesh->geometry->faces[5].texcoord[2] = Vector2(x2*u, y1*v);
	mesh->geometry->faces[5].texcoord[0] = Vector2(x1*u, y2*v);

	// bottom
	x1 = x2 = dx; y1 = y2 = dy; x2 += dw; y2 += dh;
	mesh->geometry->faces[6].texcoord[0] = Vector2(x2*u, y1*v);
	mesh->geometry->faces[6].texcoord[1] = Vector2(x2*u, y2*v);
	mesh->geometry->faces[6].texcoord[2] = Vector2(x1*u, y2*v);

	mesh->geometry->faces[7].texcoord[1] = Vector2(x1*u, y1*v);
	mesh->geometry->faces[7].texcoord[2] = Vector2(x2*u, y1*v);
	mesh->geometry->faces[7].texcoord[0] = Vector2(x1*u, y2*v);
}

//////////////////////////////////////////////////////////////////////////
bool HasFileChanged(string path)
{
	static struct __stat64 fileinfo;
	static map<string, __time64_t> fileTimes;
	if (-1 != _stat64(path.c_str(), &fileinfo))
	{
		__time64_t time = fileTimes[path];
		if (time == 0) time = fileTimes[path] = fileinfo.st_mtime;
		if (time != fileinfo.st_mtime)
		{
			fileTimes[path] = fileinfo.st_mtime;
			return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
// Adjust angle gradually towards the target angle
//////////////////////////////////////////////////////////////////////////
void StepAngle(Vector3 &angle, Vector3 &targetAngle, float rotationSpeed, float frametime)
{
	if (angle.x > targetAngle.x) 
	{
		angle.x -= rotationSpeed * frametime;
		if (angle.x < targetAngle.x) angle.x = targetAngle.x;
	}
	else if (angle.x < targetAngle.x)
	{
		angle.x += rotationSpeed * frametime;
		if (angle.x > targetAngle.x) angle.x = targetAngle.x;
	}

	if (angle.y > targetAngle.y) 
	{
		angle.y -= rotationSpeed * frametime;
		if (angle.y < targetAngle.y) angle.y = targetAngle.y;
	}
	else if (angle.y < targetAngle.y)
	{
		angle.y += rotationSpeed * frametime;
		if (angle.y > targetAngle.y) angle.y = targetAngle.y;
	}

	if (angle.z > targetAngle.z) 
	{
		angle.z -= rotationSpeed * frametime;
		if (angle.z < targetAngle.z) angle.z = targetAngle.z;
	}
	else if (angle.z < targetAngle.z)
	{
		angle.z += rotationSpeed * frametime;
		if (angle.z > targetAngle.z) angle.z = targetAngle.z;
	}
}

//////////////////////////////////////////////////////////////////////////
void StepLinear(float &current, float &end, float speed, float frametime)
{
	if (current < end)
	{
		current += speed * frametime;
		if (current > end) current = end;
	}
	else if (current > end)
	{
		current -= speed * frametime;
		if (current < end) current = end;
	}
}

//////////////////////////////////////////////////////////////////////////
void StepAngle(float &current, float end, float speed, float frametime)
{
	if (end > 180) end -= 360;
	if (abs(current - end) > 180) current -= 360;
	//if (current > 180) current -= 360;

	if (current < end)
	{
		current += speed * frametime;
		if (current > end) current = end;
	}
	else if (current > end)
	{
		current -= speed * frametime;
		if (current < end) current = end;
	}
	CheckAngle(current);
}


//////////////////////////////////////////////////////////////////////////
float GetParabolicLaunchAngle(float g, float v, float x, float y)
{
	float s = (v * v * v * v) - g * (g * (x * x) + 2 * y * (v * v)); //substitution
	return atan(((v * v) + sqrt(s)) / (g * x)); // launch angle
}

bool SameSide(Vector3 p1, Vector3 p2, Vector3 a, Vector3 b)
{
	Vector3 cp1, cp2;
	cp1 = (b - a).Cross(p1 - a);
	cp2 = (b - a).Cross(p2 - a);
	if (cp1.Dot(cp2) >= 0)
		return true;
	return false;
}

bool IsPointInsideTriangle(Vector2 point, Vector2 v1, Vector2 v2, Vector2 v3)
{
	Vector3 p(point.x, point.y, 0);
	Vector3 a(v1.x, v1.y, 0);
	Vector3 b(v2.x, v2.y, 0);
	Vector3 c(v3.x, v3.y, 0);
	if (SameSide(p, a, b, c) && SameSide(p, b, a, c) && SameSide(p, c, a, b))
		return true;
	return false;
}