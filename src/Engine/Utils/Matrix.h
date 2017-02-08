#pragma once
#include "platform.h"
#include <math.h>
#include "Vector.h"

struct Matrix3 
{
	float m[9];

	Matrix3() {}

	Matrix3& Transpose() 
	{
		float tmp;

		tmp = m[1]; m[1] = m[3]; m[3] = tmp;
		tmp = m[2]; m[2] = m[6]; m[6] = tmp;
		tmp = m[5]; m[5] = m[7]; m[7] = tmp;
		return *this;
	}

	Matrix3& TransposeIntoArray( float* r ) 
	{	
		r[ 0 ] = m[ 0 ];
		r[ 1 ] = m[ 3 ];
		r[ 2 ] = m[ 6 ];
		r[ 3 ] = m[ 1 ];
		r[ 4 ] = m[ 4 ];
		r[ 5 ] = m[ 7 ];
		r[ 6 ] = m[ 2 ];
		r[ 7 ] = m[ 5 ];
		r[ 8 ] = m[ 8 ];

		return *this;
	}
};

struct Matrix4
{
	float n11,n12,n13,n14;
	float n21,n22,n23,n24;
	float n31,n32,n33,n34;
	float n41,n42,n43,n44;

	float	flat[16];
	Matrix3	m33;

	Matrix4() 
	{
		Identity();
	}

	Matrix4(D3DXMATRIX &m) 
	{
		Set(m._11, m._21, m._31, m._41, m._12, m._22, m._32, m._42, m._13, m._23, m._33, m._43, m._14, m._24, m._34, m._44);
	}

	Matrix4( float n11, float n12, float n13, float n14, float n21, float n22, float n23, float n24, float n31, float n32, float n33, float n34, float n41, float n42, float n43, float n44 ) 
	{
		Set(n11, n12, n13, n14, n21, n22, n23, n24, n31, n32, n33, n34, n41, n42, n43, n44);
	};

	Matrix4( float n[16] ) 
	{
		//Set(n[0], n[1], n[2], n[3], n[4], n[5], n[6], n[7], n[8], n[9], n[10], n[11], n[12], n[13], n[14], n[15]);
		n11 = n[0]; n12 = n[4]; n13 = n[8]; n14 = n[12];
		n21 = n[1]; n22 = n[5]; n23 = n[9]; n24 = n[13];
		n31 = n[2]; n32 = n[6]; n33 = n[10]; n34 = n[14];
		n41 = n[3]; n42 = n[7]; n43 = n[11]; n44 = n[15];
	};

	void Set( float n11, float n12, float n13, float n14, float n21, float n22, float n23, float n24, float n31, float n32, float n33, float n34, float n41, float n42, float n43, float n44 ) 
	{
		this->n11 = n11; this->n12 = n12; this->n13 = n13; this->n14 = n14;
		this->n21 = n21; this->n22 = n22; this->n23 = n23; this->n24 = n24;
		this->n31 = n31; this->n32 = n32; this->n33 = n33; this->n34 = n34;
		this->n41 = n41; this->n42 = n42; this->n43 = n43; this->n44 = n44;
	};

	void Set( Matrix4& m )
	{
		this->n11 = m.n11; this->n12 = m.n12; this->n13 = m.n13; this->n14 = m.n14;
		this->n21 = m.n21; this->n22 = m.n22; this->n23 = m.n23; this->n24 = m.n24;
		this->n31 = m.n31; this->n32 = m.n32; this->n33 = m.n33; this->n34 = m.n34;
		this->n41 = m.n41; this->n42 = m.n42; this->n43 = m.n43; this->n44 = m.n44;
	}

	operator D3DXMATRIX() const
	{
		return D3DXMATRIX( n11, n21, n31, n41, n12, n22, n32, n42, n13, n23, n33, n43, n14, n24, n34, n44);
	}

	Matrix4& Identity()
	{
		Set(1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1);
		return *this;
	}

	Matrix4 Copy() 
	{
		return Matrix4(*this);
	}

	Vector3 GetPosition() 
	{	
		return Vector3(n14, n24, n34); 
	}

	Matrix4& SetPosition(Vector3 v)
	{
		n14 = v.x;
		n24 = v.y;
		n34 = v.z;
		return *this;
	}

	Vector3 GetColumnX() 
	{	
		return Vector3(n11, n21, n31); 
	}

	Vector3 GetColumnY() 
	{	
		return Vector3(n12, n22, n32); 
	}

	Vector3 GetColumnZ() 
	{	
		return Vector3(n13, n23, n33); 
	}

	Matrix4& LookAt( Vector3& eye, Vector3& center, Vector3& up ) 
	{
		Vector3 x, y, z;

		D3DXMATRIX m;
		D3DXVECTOR3 e = D3DXVECTOR3(eye.x,eye.y,eye.z);
		D3DXVECTOR3 t = D3DXVECTOR3(center.x,center.y,center.z);
		D3DXVECTOR3 u = D3DXVECTOR3(up.x,up.y,up.z);
		D3DXMatrixLookAtRH(&m, &e, &t, &u);
		return Matrix4(m);
	};

	static Matrix4& Multiply( Matrix4& a, Matrix4& b ) 
	{
		Matrix4 A = Matrix4(a);		
		A *= b;
		return A;
	}

	Matrix4& operator *(Matrix4& m)
	{	
		*this *= m;
		return *this;
	}

	Matrix4& operator *=(Matrix4& m) 
	{
		Set(
			n11 * m.n11 + n12 * m.n21 + n13 * m.n31 + n14 * m.n41,
			n11 * m.n12 + n12 * m.n22 + n13 * m.n32 + n14 * m.n42,
			n11 * m.n13 + n12 * m.n23 + n13 * m.n33 + n14 * m.n43,
			n11 * m.n14 + n12 * m.n24 + n13 * m.n34 + n14 * m.n44,

			n21 * m.n11 + n22 * m.n21 + n23 * m.n31 + n24 * m.n41,
			n21 * m.n12 + n22 * m.n22 + n23 * m.n32 + n24 * m.n42,
			n21 * m.n13 + n22 * m.n23 + n23 * m.n33 + n24 * m.n43,
			n21 * m.n14 + n22 * m.n24 + n23 * m.n34 + n24 * m.n44,

			n31 * m.n11 + n32 * m.n21 + n33 * m.n31 + n34 * m.n41,
			n31 * m.n12 + n32 * m.n22 + n33 * m.n32 + n34 * m.n42,
			n31 * m.n13 + n32 * m.n23 + n33 * m.n33 + n34 * m.n43,
			n31 * m.n14 + n32 * m.n24 + n33 * m.n34 + n34 * m.n44,

			n41 * m.n11 + n42 * m.n21 + n43 * m.n31 + n44 * m.n41,
			n41 * m.n12 + n42 * m.n22 + n43 * m.n32 + n44 * m.n42,
			n41 * m.n13 + n42 * m.n23 + n43 * m.n33 + n44 * m.n43,
			n41 * m.n14 + n42 * m.n24 + n43 * m.n34 + n44 * m.n44 );
		return *this;
	}

	Matrix4& MultiplyToArray(Matrix4& a, Matrix4& b, float *r)
	{
		(*this) *= b;
		r[ 0 ] = n11; r[ 1 ] = n21; r[ 2 ] = n31; r[ 3 ] = n41;
		r[ 4 ] = n12; r[ 5 ] = n22; r[ 6 ] = n32; r[ 7 ] = n42;
		r[ 8 ]  = n13; r[ 9 ]  = n23; r[ 10 ] = n33; r[ 11 ] = n43;
		r[ 12 ] = n14; r[ 13 ] = n24; r[ 14 ] = n34; r[ 15 ] = n44;
		return *this;
	}

	Matrix4& MultiplyScalar(float s) 
	{
		n11 *= s; n12 *= s; n13 *= s; n14 *= s;
		n21 *= s; n22 *= s; n23 *= s; n24 *= s;
		n31 *= s; n32 *= s; n33 *= s; n34 *= s;
		n41 *= s; n42 *= s; n43 *= s; n44 *= s;
		return *this;
	}

	Matrix4& operator *(float s)
	{
		return MultiplyScalar(s);
	}

	Vector3& MultiplyVector3( Vector3 v ) 
	{
		float vx = v.x, vy = v.y, vz = v.z;
		float d = 1.0f / ( n41 * vx + n42 * vy + n43 * vz + n44 );

		v.x = ( n11 * vx + n12 * vy + n13 * vz + n14 ) * d;
		v.y = ( n21 * vx + n22 * vy + n23 * vz + n24 ) * d;
		v.z = ( n31 * vx + n32 * vy + n33 * vz + n34 ) * d;

		return v;
	}

	Vector3& RotateAxis( Vector3& v ) 
	{
		float vx = v.x, vy = v.y, vz = v.z;

		v.x = vx * n11 + vy * n12 + vz * n13;
		v.y = vx * n21 + vy * n22 + vz * n23;
		v.z = vx * n31 + vy * n32 + vz * n33;

		v.Normalize();
		return v;
	}

	Vector4 CrossVector( Vector4 a ) 
	{
		Vector4 v = Vector4();
		v.x = n11 * a.x + n12 * a.y + n13 * a.z + n14 * a.w;
		v.y = n21 * a.x + n22 * a.y + n23 * a.z + n24 * a.w;
		v.z = n31 * a.x + n32 * a.y + n33 * a.z + n34 * a.w;
		v.w = ( a.w ) ? n41 * a.x + n42 * a.y + n43 * a.z + n44 * a.w : 1;
		return v;
	}

	float Determinant() 
	{
		return (
			n14 * n23 * n32 * n41-
			n13 * n24 * n32 * n41-
			n14 * n22 * n33 * n41+
			n12 * n24 * n33 * n41+

			n13 * n22 * n34 * n41-
			n12 * n23 * n34 * n41-
			n14 * n23 * n31 * n42+
			n13 * n24 * n31 * n42+

			n14 * n21 * n33 * n42-
			n11 * n24 * n33 * n42-
			n13 * n21 * n34 * n42+
			n11 * n23 * n34 * n42+

			n14 * n22 * n31 * n43-
			n12 * n24 * n31 * n43-
			n14 * n21 * n32 * n43+
			n11 * n24 * n32 * n43+

			n12 * n21 * n34 * n43-
			n11 * n22 * n34 * n43-
			n13 * n22 * n31 * n44+
			n12 * n23 * n31 * n44+

			n13 * n21 * n32 * n44-
			n11 * n23 * n32 * n44-
			n12 * n21 * n33 * n44+
			n11 * n22 * n33 * n44
			);
	}

	Matrix4& Transpose() 
	{
		float tmp;

		tmp = n21; n21 = n12; n12 = tmp;
		tmp = n31; n31 = n13; n13 = tmp;
		tmp = n32; n32 = n23; n23 = tmp;

		tmp = n41; n41 = n14; n14 = tmp;
		tmp = n42; n42 = n24; n24 = tmp;
		tmp = n43; n43 = n34; n43 = tmp;
		return *this;
	}

	Matrix4 Clone() 
	{
		return Matrix4(*this);
	}

	float* Flatten() 
	{
		flat[ 0 ] = n11; flat[ 1 ] = n21; flat[ 2 ] = n31; flat[ 3 ] = n41;
		flat[ 4 ] = n12; flat[ 5 ] = n22; flat[ 6 ] = n32; flat[ 7 ] = n42;
		flat[ 8 ]  = n13; flat[ 9 ]  = n23; flat[ 10 ] = n33; flat[ 11 ] = n43;
		flat[ 12 ] = n14; flat[ 13 ] = n24; flat[ 14 ] = n34; flat[ 15 ] = n44;
		return flat;
	}

	float* FlattenToArray( float* flat ) 
	{
		/*flat[ 0 ] = n11; flat[ 1 ] = n12; flat[ 2 ] = n13; flat[ 3 ] = n14;
		flat[ 4 ] = n21; flat[ 5 ] = n22; flat[ 6 ] = n23; flat[ 7 ] = n24;
		flat[ 8 ]  = n31; flat[ 9 ]  = n32; flat[ 10 ] = n33; flat[ 11 ] = n34;
		flat[ 12 ] = n41; flat[ 13 ] = n42; flat[ 14 ] = n43; flat[ 15 ] = n44;*/

		flat[ 0 ] = n11; flat[ 1 ] = n21; flat[ 2 ] = n31; flat[ 3 ] = n41;
		flat[ 4 ] = n12; flat[ 5 ] = n22; flat[ 6 ] = n32; flat[ 7 ] = n42;
		flat[ 8 ]  = n13; flat[ 9 ]  = n23; flat[ 10 ] = n33; flat[ 11 ] = n43;
		flat[ 12 ] = n14; flat[ 13 ] = n24; flat[ 14 ] = n34; flat[ 15 ] = n44;

		return flat;
	}

	float* FlattenToArrayOffset( float* flat, int offset ) {

		flat[ offset ] = n11;
		flat[ offset + 1 ] = n21;
		flat[ offset + 2 ] = n31;
		flat[ offset + 3 ] = n41;

		flat[ offset + 4 ] = n12;
		flat[ offset + 5 ] = n22;
		flat[ offset + 6 ] = n32;
		flat[ offset + 7 ] = n42;

		flat[ offset + 8 ]  = n13;
		flat[ offset + 9 ]  = n23;
		flat[ offset + 10 ] = n33;
		flat[ offset + 11 ] = n43;

		flat[ offset + 12 ] = n14;
		flat[ offset + 13 ] = n24;
		flat[ offset + 14 ] = n34;
		flat[ offset + 15 ] = n44;

		return flat;
	}

	Matrix4& SetTranslation( float x, float y, float z ) 
	{
		Set(
			1, 0, 0, x,
			0, 1, 0, y,
			0, 0, 1, z,
			0, 0, 0, 1
			);
		return *this;
	}

	Matrix4& SetScale( float x, float y, float z ) 
	{
		Set(
			x, 0, 0, 0,
			0, y, 0, 0,
			0, 0, z, 0,
			0, 0, 0, 1
			);

		return *this;
	}

	Matrix4& SetRotationX( float theta ) 
	{
		float c = cosf( theta ), s = sinf( theta );

		Set(
			1, 0,  0, 0,
			0, c, -s, 0,
			0, s,  c, 0,
			0, 0,  0, 1
			);
		return *this;
	}

	Matrix4& SetRotationY( float theta ) 
	{
		float c = cosf( theta ), s = sinf( theta );

		Set(
			c, 0, s, 0,
			0, 1, 0, 0,
			-s, 0, c, 0,
			0, 0, 0, 1
			);

		return *this;
	}

	Matrix4& setRotationZ( float theta ) 
	{
		float c = cosf( theta ), s = sinf( theta );

		Set(
			c, -s, 0, 0,
			s,  c, 0, 0,
			0,  0, 1, 0,
			0,  0, 0, 1
			);
		return *this;
	}

	Matrix4& SetRotationAxis( Vector3& axis, float angle ) 
	{
		float c = cosf( angle ),
			s = sinf( angle ),
			t = 1 - c,
			x = axis.x, y = axis.y, z = axis.z,
			tx = t * x, ty = t * y;

		Set(
			tx * x + c, tx * y - s * z, tx * z + s * y, 0,
			tx * y + s * z, ty * y + c, ty * z - s * x, 0,
			tx * z - s * y, ty * z + s * x, t * z * z + c, 0,
			0, 0, 0, 1
			);
		return *this;
	}

	Matrix4& GetInverse( Matrix4& m ) 
	{
		float n11 = m.n11, n12 = m.n12, n13 = m.n13, n14 = m.n14,
			n21 = m.n21, n22 = m.n22, n23 = m.n23, n24 = m.n24,
			n31 = m.n31, n32 = m.n32, n33 = m.n33, n34 = m.n34,
			n41 = m.n41, n42 = m.n42, n43 = m.n43, n44 = m.n44;

		this->n11 = n23*n34*n42 - n24*n33*n42 + n24*n32*n43 - n22*n34*n43 - n23*n32*n44 + n22*n33*n44;
		this->n12 = n14*n33*n42 - n13*n34*n42 - n14*n32*n43 + n12*n34*n43 + n13*n32*n44 - n12*n33*n44;
		this->n13 = n13*n24*n42 - n14*n23*n42 + n14*n22*n43 - n12*n24*n43 - n13*n22*n44 + n12*n23*n44;
		this->n14 = n14*n23*n32 - n13*n24*n32 - n14*n22*n33 + n12*n24*n33 + n13*n22*n34 - n12*n23*n34;
		this->n21 = n24*n33*n41 - n23*n34*n41 - n24*n31*n43 + n21*n34*n43 + n23*n31*n44 - n21*n33*n44;
		this->n22 = n13*n34*n41 - n14*n33*n41 + n14*n31*n43 - n11*n34*n43 - n13*n31*n44 + n11*n33*n44;
		this->n23 = n14*n23*n41 - n13*n24*n41 - n14*n21*n43 + n11*n24*n43 + n13*n21*n44 - n11*n23*n44;
		this->n24 = n13*n24*n31 - n14*n23*n31 + n14*n21*n33 - n11*n24*n33 - n13*n21*n34 + n11*n23*n34;
		this->n31 = n22*n34*n41 - n24*n32*n41 + n24*n31*n42 - n21*n34*n42 - n22*n31*n44 + n21*n32*n44;
		this->n32 = n14*n32*n41 - n12*n34*n41 - n14*n31*n42 + n11*n34*n42 + n12*n31*n44 - n11*n32*n44;
		this->n33 = n13*n24*n41 - n14*n22*n41 + n14*n21*n42 - n11*n24*n42 - n12*n21*n44 + n11*n22*n44;
		this->n34 = n14*n22*n31 - n12*n24*n31 - n14*n21*n32 + n11*n24*n32 + n12*n21*n34 - n11*n22*n34;
		this->n41 = n23*n32*n41 - n22*n33*n41 - n23*n31*n42 + n21*n33*n42 + n22*n31*n43 - n21*n32*n43;
		this->n42 = n12*n33*n41 - n13*n32*n41 + n13*n31*n42 - n11*n33*n42 - n12*n31*n43 + n11*n32*n43;
		this->n43 = n13*n22*n41 - n12*n23*n41 - n13*n21*n42 + n11*n23*n42 + n12*n21*n43 - n11*n22*n43;
		this->n44 = n12*n23*n31 - n13*n22*n31 + n13*n21*n32 - n11*n23*n32 - n12*n21*n33 + n11*n22*n33;
		this->MultiplyScalar( 1.0f / this->Determinant() );
		return *this;
	}

	Matrix4& GetInverse() 
	{
		Matrix4 m;
		D3DXMATRIX M = *this;
		D3DXMATRIX out;
		D3DXMatrixInverse(&out, NULL, &M);
		m = out;
		return m;
	}

	Matrix4& SetRotationFromQuaternion( Vector4& q ) 
	{
		float x = q.x, y = q.y, z = q.z, w = q.w,
			x2 = x + x, y2 = y + y, z2 = z + z,
			xx = x * x2, xy = x * y2, xz = x * z2,
			yy = y * y2, yz = y * z2, zz = z * z2,
			wx = w * x2, wy = w * y2, wz = w * z2;

		n11 = 1 - ( yy + zz );
		n12 = xy - wz;
		n13 = xz + wy;

		n21 = xy + wz;
		n22 = 1 - ( xx + zz );
		n23 = yz - wx;

		n31 = xz - wy;
		n32 = yz + wx;
		n33 = 1 - ( xx + yy );

		return *this;
	}

	Matrix4& SetRotationFromEuler( Vector3& v, string order ) 
	{

		float x = v.x * ToRad, y = v.y * ToRad, z = v.z * ToRad,
			a = cos( x ), b = sin( x ),
			c = cos( y ), d = sin( y ),
			e = cos( z ), f = sin( z );

		if (order == "YXZ")
		{
			float ce = c * e, cf = c * f, de = d * e, df = d * f;

			n11 = ce + df * b;
			n12 = de * b - cf;
			n13 = a * d;

			n21 = a * f;
			n22 = a * e;
			n23 = - b;

			n31 = cf * b - de;
			n32 = df + ce * b;
			n33 = a * c;

		} else 
			if (order == "ZXY")
			{
				float ce = c * e, cf = c * f, de = d * e, df = d * f;

				n11 = ce - df * b;
				n12 = - a * f;
				n13 = de + cf * b;

				n21 = cf + de * b;
				n22 = a * e;
				n23 = df - ce * b;

				n31 = - a * d;
				n32 = b;
				n33 = a * c;
			} else
				if (order == "ZYX")
				{
					float ae = a * e, af = a * f, be = b * e, bf = b * f;

					n11 = c * e;
					n12 = be * d - af;
					n13 = ae * d + bf;

					n21 = c * f;
					n22 = bf * d + ae;
					n23 = af * d - be;

					n31 = - d;
					n32 = b * c;
					n33 = a * c;
				} else
					if (order == "YZX")
					{
						float ac = a * c, ad = a * d, bc = b * c, bd = b * d;

						n11 = c * e;
						n12 = bd - ac * f;
						n13 = bc * f + ad;

						n21 = f;
						n22 = a * e;
						n23 = - b * e;

						n31 = - d * e;
						n32 = ad * f + bc;
						n33 = ac - bd * f;
					} else
						if (order == "XZY")
						{
							float ac = a * c, ad = a * d, bc = b * c, bd = b * d;

							n11 = c * e;
							n12 = - f;
							n13 = d * e;

							n21 = ac * f + bd;
							n22 = a * e;
							n23 = ad * f - bc;

							n31 = bc * f - ad;
							n32 = b * e;
							n33 = bd * f + ac;

						} 
						else
						{
							float ae = a * e, af = a * f, be = b * e, bf = b * f;

							n11 = c * e;
							n12 = - c * f;
							n13 = d;

							n21 = af + be * d;
							n22 = ae - bf * d;
							n23 = - b * c;

							n31 = bf - ae * d;
							n32 = be + af * d;
							n33 = a * c;
						}

						return *this;

	}

	Matrix4& Scale( Vector3& v ) 
	{
		float x = v.x, y = v.y, z = v.z;

		n11 *= x; n12 *= y; n13 *= z;
		n21 *= x; n22 *= y; n23 *= z;
		n31 *= x; n32 *= y; n33 *= z;
		n41 *= x; n42 *= y; n43 *= z;

		return *this;
	}

	Matrix4& Compose( Vector3& translation, Vector4& rotation, Vector3& scale ) 
	{
		Matrix4 mRotation = Matrix4();
		Matrix4 mScale = Matrix4();

		mRotation.Identity();
		mRotation.SetRotationFromQuaternion( rotation );

		mScale.SetScale( scale.x, scale.y, scale.z );

		Set( mRotation * mScale );

		n14 = translation.x;
		n24 = translation.y;
		n34 = translation.z;

		return *this;
	}

	void Decompose( Vector3& translation, Vector3& rotation, Vector3& scale ) 
	{

		// grab the axis vectors
		Vector3 x = Vector3(), y = Vector3(), z = Vector3();

		x.Set( n11, n21, n31 );
		y.Set( n12, n22, n32 );
		z.Set( n13, n23, n33 );

		scale.x = x.Length();
		scale.y = y.Length();
		scale.z = z.Length();

		translation.x = n14;
		translation.y = n24;
		translation.z = n34;

		// scale the rotation part

		Matrix4 matrix = Matrix4();

		matrix.Set( *this );

		matrix.n11 /= scale.x;
		matrix.n21 /= scale.x;
		matrix.n31 /= scale.x;

		matrix.n12 /= scale.y;
		matrix.n22 /= scale.y;
		matrix.n32 /= scale.y;

		matrix.n13 /= scale.z;
		matrix.n23 /= scale.z;
		matrix.n33 /= scale.z;
				
		rotation.SetRotationFromMatrix( matrix );
	}

	

	Matrix4& ExtractRotation( Matrix4& m ) 
	{
		Vector3 vector = Vector3();

		float scaleX = 1.0f / vector.Set( m.n11, m.n21, m.n31 ).Length();
		float scaleY = 1.0f / vector.Set( m.n12, m.n22, m.n32 ).Length();
		float scaleZ = 1.0f / vector.Set( m.n13, m.n23, m.n33 ).Length();

		n11 = m.n11 * scaleX;
		n21 = m.n21 * scaleX;
		n31 = m.n31 * scaleX;

		n12 = m.n12 * scaleY;
		n22 = m.n22 * scaleY;
		n32 = m.n32 * scaleY;

		n13 = m.n13 * scaleZ;
		n23 = m.n23 * scaleZ;
		n33 = m.n33 * scaleZ;

		return *this;
	}

	static Matrix3 MakeInvert3x3( Matrix4& m1 ) {

		Matrix3 m33 = m1.m33;
		float	a11 =   m1.n33 * m1.n22 - m1.n32 * m1.n23,
			a21 = - m1.n33 * m1.n21 + m1.n31 * m1.n23,
			a31 =   m1.n32 * m1.n21 - m1.n31 * m1.n22,
			a12 = - m1.n33 * m1.n12 + m1.n32 * m1.n13,
			a22 =   m1.n33 * m1.n11 - m1.n31 * m1.n13,
			a32 = - m1.n32 * m1.n11 + m1.n31 * m1.n12,
			a13 =   m1.n23 * m1.n12 - m1.n22 * m1.n13,
			a23 = - m1.n23 * m1.n11 + m1.n21 * m1.n13,
			a33 =   m1.n22 * m1.n11 - m1.n21 * m1.n12,

			det = m1.n11 * a11 + m1.n21 * a12 + m1.n31 * a13,
			idet;

		// no inverse
		if ( det == 0 )
			return m33;
		
		idet = 1.0 / det;

		m33.m[ 0 ] = idet * a11; m33.m[ 1 ] = idet * a21; m33.m[ 2 ] = idet * a31;
		m33.m[ 3 ] = idet * a12; m33.m[ 4 ] = idet * a22; m33.m[ 5 ] = idet * a32;
		m33.m[ 6 ] = idet * a13; m33.m[ 7 ] = idet * a23; m33.m[ 8 ] = idet * a33;

		return m33;
	}

#ifdef far 
#undef far
#endif

#ifdef near 
#undef near
#endif


	static Matrix4 MakePerspective( float fov, float aspect, float near, float far ) 
	{
		D3DXMATRIX m;
		D3DXMatrixPerspectiveFovLH(&m, DEG2RAD(fov), aspect, near, far);
		return Matrix4(m);
	}

	static Matrix4 MakeOrtho( float left, float right, float bottom, float top, float near, float far ) 
	{
		float x, y, z, w, h, p;

		Matrix4 m = Matrix4();

		w = right - left;
		h = top - bottom;
		p = far - near;

		x = ( right + left ) / w;
		y = ( top + bottom ) / h;
		z = ( far + near ) / p;

		m.n11 = 2.0f / w; m.n12 = 0;     m.n13 = 0;      m.n14 = -x;
		m.n21 = 0;     m.n22 = 2.0f / h; m.n23 = 0;      m.n24 = -y;
		m.n31 = 0;     m.n32 = 0;     m.n33 = -2.0f / p; m.n34 = -z;
		m.n41 = 0;     m.n42 = 0;     m.n43 = 0;		m.n44 = 1;

		return m;
	}

	Vector3 GetEulerAngles()
	{
		float yaw;
		float roll;
		float pitch;
		const float minSin = 0.99995f;

		const Matrix4& matrix = *this;
		
		roll = float(0.0f);
		pitch  = float(0.0f);

		yaw = asin (- min (max (matrix.n31, float(-0.999999f)), float(0.999999f)));
		if (matrix.n31 < minSin) {
			if (matrix.n31 > (-minSin)) {
				roll = atan2 (matrix.n21, matrix.n11);
				pitch = atan2 (matrix.n32, matrix.n33);
			} else {
				pitch = atan2 (matrix.n12, matrix.n22);
			}
		} else {
			pitch = -atan2 (matrix.n12, matrix.n22);
		}
		
		return Vector3(pitch,yaw,roll);
	}

	Matrix4 PitchMatrix(float ang)
	{
		float cosAng = cos(ang);
		float sinAng = sin(ang);
		return Matrix4(1,0,0,0,0,cosAng,sinAng,0,0,-sinAng,cosAng,0,0,0,0,1);
	}

	Matrix4 YawMatrix(float ang)
	{
		float cosAng = cos(ang);
		float sinAng = sin(ang);
		return Matrix4(cosAng,0,-sinAng,0,0,1,0,0,sinAng,0,cosAng,0,0,0,0,1);
	}

	Matrix4 RollMatrix(float ang)
	{
		float cosAng = cos(ang);
		float sinAng = sin(ang);
		return Matrix4(cosAng,sinAng,0,0,-sinAng,cosAng,0,0,0,0,1,0,0,0,0,1);
	}

	Matrix4& SetEulerAngles(Vector3 angles)
	{
		int i;
		int j;

		Matrix4 pitch = PitchMatrix(angles[0]);
		Matrix4 yaw = YawMatrix(angles[1]);
		Matrix4 roll = RollMatrix(angles[2]);
		Matrix4 mat = pitch * yaw * roll;
				
		this->Set(mat);
		return *this;
	}
};

