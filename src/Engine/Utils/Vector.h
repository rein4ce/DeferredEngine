#pragma once
#include <math.h>

float frand(float a, float b);
class Matrix4;


//////////////////////////////////////////////////////////////////////////
// Vector3
//////////////////////////////////////////////////////////////////////////
class Vector3
{
public:
	union
	{
		struct
		{
			float x,y,z;
		};
		struct
		{
			float coord[3];
		};
	};

	static const Vector3& Blank() { static const Vector3 v(0, 0, 0); return v; }
public:
	inline Vector3(void)
	{ x=y=z=0; }

	inline Vector3(float Ix,float Iy, float Iz)
		: x(Ix)
		, y(Iy)
		, z(Iz)
	{}

	inline Vector3 &operator /=(const float Scalar)	{ x /= Scalar; y /= Scalar;	z /= Scalar; return *this; }
	inline Vector3 &operator *=(const float Scalar)	{ x *= Scalar; y *= Scalar; z *= Scalar; return *this; }
	inline Vector3 &operator +=(const Vector3 &Other) { x += Other.x;	y += Other.y; z += Other.z; return *this; }
	inline Vector3 &operator -=(const Vector3 &Other) { x -= Other.x;	y -= Other.y; z -= Other.z; return *this;	}

	inline Vector3 operator ^ (const Vector3 &V)	const	{	return Vector3((y * V.z) - (z * V.y), (z * V.x) - (x * V.z), (x * V.y) - (y * V.x)); }
	inline float operator * (const Vector3 &V)	const	{	return (x*V.x) + (y*V.y) + (z*V.z); } // dot product

	inline Vector3 operator * (float  s)			const	{	return Vector3(x*s, y*s, z*s); }
	inline Vector3 operator / (float  s)			const	{	return Vector3(x/s, y/s, z/s); }
	inline Vector3 operator + (const Vector3 &V)	const	{	return Vector3(x+V.x, y+V.y, z+V.z); }
	inline Vector3 operator - (const Vector3 &V)	const	{	return Vector3(x-V.x, y-V.y, z-V.z); }
	friend Vector3 operator * (float k, const Vector3& V) {	return Vector3(V.x*k, V.y*k, V.z*k); }
	inline Vector3 operator -(void) const { return Vector3(-x, -y, -z); }

	inline bool		operator > (Vector3 &other) { return other.x < x && other.y < y && other.z < z; }
	inline bool		operator < (Vector3 &other) { return other.x > x && other.y > y && other.z > z; }

	inline float	LengthSq()						const { return x*x + y*y + z*z; }
	inline float	Length(void)					const { return (float) sqrt(x*x + y*y + z*z); }
	inline float	Dot(const Vector3 &V)			const { return (x*V.x) + (y*V.y) + (z*V.z); }
	inline Vector3	Copy(const Vector3 &V)			const { return Vector3(*this); }
	inline Vector3	Cross(const Vector3 &V)			const { return Vector3((y * V.z) - (z * V.y), (z * V.x) - (x * V.z), (x * V.y) - (y * V.x)); }
	inline float	Distance(const Vector3 &V)		const { return (float)(V-(*this)).Length(); }
	inline bool		IsZero()						const { return LengthSq() < 0.0001f; }

	static inline float Dot(Vector3 &a, Vector3 &b) { return a.Dot(b); }
	static inline Vector3 Cross(Vector3 &a, Vector3 &b) { return a.Cross(b); }

	Vector3& Randomise(const Vector3& xMin, const Vector3& xMax)
	{
		x = frand(xMin.x, xMax.x);
		y = frand(xMin.y, xMax.y);
		z = frand(xMin.z, xMax.z);
		return *this;
	}

	Vector3& Normalize(void) 
	{	
		float fLength = Length();	

		if (fLength != 0.0f) 
			(*this) *= (1.0f / fLength); 

		return *this;	
	}

	Vector3& Min(const Vector3& A, const Vector3& B)
	{
		x = (A.x < B.x)? A.x : B.x;
		y = (A.y < B.y)? A.y : B.y;
		z = (A.z < B.z)? A.z : B.z;
		return *this;
	}
	Vector3& Max(const Vector3& A, const Vector3& B)
	{
		x = (A.x > B.x)? A.x : B.x;
		y = (A.y > B.y)? A.y : B.y;
		z = (A.z > B.z)? A.z : B.z;
		return *this;
	}

	float operator[](int i) const { return coord[i]; }
	float& operator[](int i) { return coord[i]; }

	Vector3& MultiplyWithMatrix(const Vector3* Mtx)
	{
		float ix = (Mtx[0][0] * x) + (Mtx[0][1] * y) + (Mtx[0][2] * z);
		float iy = (Mtx[1][0] * x) + (Mtx[1][1] * y) + (Mtx[1][2] * z);
		float iz = (Mtx[2][0] * x) + (Mtx[2][1] * y) + (Mtx[2][2] * z);
		x = ix;
		y = iy;
		z = iz;
		return *this;
	}

	static void CreateRandomOrientationMatrix(Vector3* Orient)
	{
		do
		{	
			Vector3 Dir, Up;

			Dir.Randomise(Vector3(-1, -1, -1), Vector3(1, 1, 1));
			Dir.Normalize();
			Up.Randomise(Vector3(-1, -1, -1), Vector3(1, 1, 1));
			Up.Normalize();

			CreateOrientationMatrix(Orient, Dir, Up);
		}
		while (fabs(Orient[1].Length() - 1.0f) > 1.0E-4f);
	}

	static void CreateOrientationMatrix(Vector3* Orient, const Vector3& Dir, const Vector3& Up)
	{ 
		Vector3 MyUp = Up;

		if(fabs(Dir * Up) > 0.99999f)
		{
			if (fabs(Up.y) > 0.999f)
			{
				MyUp = Vector3(1, 0, 0);
			}
			else
			{
				MyUp = Vector3(0, 1, 0);
			}
		}

		Orient[0] = Dir;
		Orient[1] = MyUp;
		Orient[2] = Orient[0] ^ Orient[1];
		Orient[2].Normalize();
		Orient[1] = Orient[2] ^ Orient[0];
		Orient[1].Normalize();
	}

	Vector3& Set( float _x, float _y, float _z )
	{
		x = _x;
		y = _y;
		z = _z;
		return *this;
	}

	// Implemented in Utils.cpp
	Vector3& SetRotationFromMatrix( Matrix4& m );

	Vector3 GetDirectionRadian()
	{
		Vector3 ret;
		ret.x = cos(y) * cos(x);
		ret.y = sin(y) * cos(x);
		ret.z = sin(x);
		return ret;
	}

	Vector3 GetDirection()
	{
		float x = this->x * ToRad;
		float y = this->y * ToRad;
		float z = this->z * ToRad;
		Vector3 ret;
		ret.x = cos(y) * cos(x);
		ret.y = sin(y) * cos(x);
		ret.z = sin(x);
		return ret;
	}

	Vector3 GetAnglesRadian()
	{
		Vector3 ret;
		ret.y = atan2(x, y);
		ret.x = atan2(z, sqrt(x*x + y*y));
		ret.z = 0;
		return ret;
	}

	Vector3 GetAngles()
	{
		return GetAnglesRadian() * ToDeg;
	}
};


//////////////////////////////////////////////////////////////////////////
// Vector2
//////////////////////////////////////////////////////////////////////////
class Vector2
{
public:
	union
	{
		struct
		{
			float x,y;
		};
		struct
		{
			float coord[2];
		};
	};

	static const Vector2& Blank() { static const Vector2 v(0, 0); return v; }
public:
	inline Vector2(void)
	{ x=y=0;}

	inline Vector2(float Ix,float Iy)
		: x(Ix)
		, y(Iy)		
	{}

	inline Vector2 &operator /=(const float Scalar)	{ x /= Scalar; y /= Scalar;	return *this; }
	inline Vector2 &operator *=(const float Scalar)	{ x *= Scalar; y *= Scalar; return *this; }
	inline Vector2 &operator +=(const Vector2 &Other) { x += Other.x;	y += Other.y;return *this; }
	inline Vector2 &operator -=(const Vector2 &Other) { x -= Other.x;	y -= Other.y; return *this;	}
	inline float operator ^ (const Vector2 &V)	const	{	return (x*V.y) - (y*V.x); }
	inline float operator * (const Vector2 &V)	const	{	return (x*V.x) + (y*V.y); }
	inline Vector2 operator * (float  s)			const	{	return Vector2(x*s, y*s); }
	inline Vector2 operator / (float  s)			const	{	return Vector2(x/s, y/s); }
	inline Vector2 operator + (const Vector2 &V)	const	{	return Vector2(x+V.x, y+V.y); }
	inline Vector2 operator - (const Vector2 &V)	const	{	return Vector2(x-V.x, y-V.y); }
	friend Vector2 operator * (float k, const Vector2& V) {	return Vector2(V.x*k, V.y*k); }
	inline Vector2 operator -(void) const { return Vector2(-x, -y); }

	inline float	LengthSq()					const { return x*x + y*y; }
	inline float	Length(void)				const { return (float) sqrt(x*x + y*y); }
	inline float	Dot(const Vector2 &V)		const	{ return (x*V.x) + (y*V.y); }
	inline float	Cross(const Vector2 &V)		const	{ return V.Cross(*this); }
	inline Vector2	Copy(const Vector2 &V)		const	{ return Vector2(*this); }
	inline float	Distance(const Vector2 &V)	const { return (float)(V-(*this)).Length(); }
	inline bool		IsZero()					const { return LengthSq() < 0.0001f; }

	Vector2& Randomise(const Vector2& xMin, const Vector2& xMax)
	{
		x = frand(xMin.x, xMax.x);
		y = frand(xMin.y, xMax.y);		
		return *this;
	}

	float Normalize(void) 
	{	
		float fLength = Length();	

		if (fLength == 0.0f) 
			return 0.0f; 

		(*this) *= (1.0f / fLength); 

		return fLength;	
	}

	Vector2& Min(const Vector2& A, const Vector2& B)
	{
		x = (A.x < B.x)? A.x : B.x;
		y = (A.y < B.y)? A.y : B.y;		
		return *this;
	}
	Vector2& Max(const Vector2& A, const Vector2& B)
	{
		x = (A.x > B.x)? A.x : B.x;
		y = (A.y > B.y)? A.y : B.y;		
		return *this;
	}

	float operator[](int i) const { return coord[i]; }
	float& operator[](int i) { return coord[i]; }

	Vector2& MultiplyWithMatrix(const Vector2* Mtx)
	{
		float ix = (Mtx[0][0] * x) + (Mtx[0][1] * y);
		float iy = (Mtx[1][0] * x) + (Mtx[1][1] * y);		
		x = ix;
		y = iy;		
		return *this;
	}

	void Set( float _x, float _y )
	{
		x = _x;
		y = _y;
	}

	Vector2 Direction(void) const
	{
		Vector2 Temp(*this);
		Temp.Normalize();
		return Temp;
	}

	float Angle(const Vector2& xE)
	{
		float dot = (*this) * xE;
		float cross = (*this) ^ xE;
		// angle between segments
		float angle = (float) atan2(cross, dot);
		return angle;
	}

	Vector2& Rotate(float angle)
	{
		float tx = x;
		x =  x * cos(angle) - y * sin(angle);
		y = tx * sin(angle) + y * cos(angle);
		return *this;
	}

	Vector2& Rotate(const Vector2& xCentre, float fAngle)
	{
		Vector2 D = *this - xCentre;
		D.Rotate(fAngle);
		*this = xCentre + D;
		return *this;
	}

	void Clamp(const Vector2& min, const Vector2& max)
	{
		x = (x < min.x)? min.x : (x > max.x)? max.x : x;
		y = (y < min.y)? min.y : (y > max.y)? max.y : y;
	}

	
};


//////////////////////////////////////////////////////////////////////////
// Vector3
//////////////////////////////////////////////////////////////////////////
class Vector4
{
public:
	union
	{
		struct
		{
			float x,y,z,w;
		};
		struct
		{
			float coord[4];
		};
	};

	static const Vector4& Blank() { static const Vector4 v(0, 0, 0, 0); return v; }
public:
	inline Vector4(void)
	{x=y=z=w=0;}

	inline Vector4(float Ix,float Iy, float Iz, float Iw)
		: x(Ix)
		, y(Iy)
		, z(Iz)
		, w(Iw)
	{}

	inline Vector4(Vector3 &v)
		: x(v.x)
		, y(v.y)
		, z(v.z)
		, w(0)
	{}

	inline Vector4 &operator /=(const float Scalar)	{ x /= Scalar; y /= Scalar;	z /= Scalar; w /= Scalar; return *this; }
	inline Vector4 &operator *=(const float Scalar)	{ x *= Scalar; y *= Scalar; z *= Scalar; w *= Scalar; return *this; }
	inline Vector4 &operator +=(const Vector4 &Other) { x += Other.x;	y += Other.y; z += Other.z; w += Other.w; return *this; }
	inline Vector4 &operator -=(const Vector4 &Other) { x -= Other.x;	y -= Other.y; z -= Other.z; w -= Other.w; return *this;	}

	inline Vector4 operator * (float  s)			const	{	return Vector4(x*s, y*s, z*s, w*s); }
	inline Vector4 operator / (float  s)			const	{	return Vector4(x/s, y/s, z/s, w/s); }
	inline Vector4 operator + (const Vector4 &V)	const	{	return Vector4(x+V.x, y+V.y, z+V.z, w+V.w); }
	inline Vector4 operator - (const Vector4 &V)	const	{	return Vector4(x-V.x, y-V.y, z-V.z, w-V.w); }
	friend Vector4 operator * (float k, const Vector4& V) {	return Vector4(V.x*k, V.y*k, V.z*k, V.w*k); }
	inline Vector4 operator -(void) const { return Vector4(-x, -y, -z, -w); }

	inline float	LengthSq()						const { return x*x + y*y + z*z + w*w; }
	inline float	Length(void)					const { return (float) sqrt(x*x + y*y + z*z + w*w); }
	inline float	Dot(const Vector4 &V)			const { return (x*V.x) + (y*V.y) + (z*V.z) + (w*V.w); }
	inline Vector4	Copy(const Vector4 &V)			const { return Vector4(*this); }
	inline float	Distance(const Vector4 &V)		const { return (float)(V-(*this)).Length(); }
	inline bool		IsZero()						const { return LengthSq() < 0.0001f; }

	Vector4& Randomise(const Vector4& xMin, const Vector4& xMax)
	{
		x = frand(xMin.x, xMax.x);
		y = frand(xMin.y, xMax.y);
		z = frand(xMin.z, xMax.z);
		w = frand(xMin.w, xMax.w);
		return *this;
	}

	Vector4& Normalize(void) 
	{	
		float fLength = Length();	

		if (fLength != 0.0f) 
			(*this) *= (1.0f / fLength); 

		return *this;	
	}

	float operator[](int i) const { return coord[i]; }
	float& operator[](int i) { return coord[i]; }

	Vector4& Set( float _x, float _y, float _z, float _w )
	{
		x = _x;
		y = _y;
		z = _z;
		w = _w;
		return *this;
	}

	float copySign(float a, float b)
	{
		return b < 0 ? -abs(a) : abs(a);
	}	
};

