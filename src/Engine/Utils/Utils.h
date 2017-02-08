#pragma once
#include "dirent.h"
#include "Vector.h"
#include "Array.h"


class CMesh;


// Get static array size
template<typename T, size_t N>
inline int ArraySize(T (&ra)[N]) 
{
	return N;
}

//////////////////////////////////////////////////////////////////////////
// 
// Geometry
//
//////////////////////////////////////////////////////////////////////////
void TraverseGrid2D( float startX, float startY, float endX, float endY, float mapWidth, float mapHeight, float gridSize, OUT CArray<Vector2> &ret );
void TraverseGrid3D( float startX, float startY, float startZ, float endX, float endY, FLOAT endZ, float mapWidth, float mapHeight, float mapDepth, float gridSize, OUT CArray<Vector3> &ret );

void TextureBlock(CMesh *mesh, 
				  float fx, float fy, float fw, float fh,
				  float bx, float by, float bw, float bh,
				  float lx, float ly, float lw, float lh,
				  float rx, float ry, float rw, float rh,
				  float ux, float uy, float uw, float uh,
				  float dx, float dy, float dw, float dh
				  );


//////////////////////////////////////////////////////////////////////////
//
// String utils
//
//////////////////////////////////////////////////////////////////////////
inline string MergeStrings(string list[], int lines)
{
	stringstream ss;
	for (int i=0; i<lines; i++) ss << list[i] << "\n";
	return ss.str();
}

string replace(const string& sData,
			   const string& sFrom,
			   const string& sTo);

#define MERGE(array) MergeStrings(array, ArraySize(array))

inline bool StartsWith(const std::string& text,const std::string& token)
{
	if(text.length() < token.length())
		return false;
	return (text.compare(0, token.length(), token) == 0);
}

inline string str(char *msg, ...)
{
	char pBuffer[1024];
	va_list args;
	va_start( args, msg );
	vsprintf( &pBuffer[0], msg, args );
	string s = pBuffer;	
	va_end(args);
	return s;
}

//////////////////////////////////////////////////////////////////////////
//
// File utils
//
//////////////////////////////////////////////////////////////////////////
inline string ReadFile(string name)
{
	ifstream t(name.c_str());
	return string(istreambuf_iterator<char>(t), istreambuf_iterator<char>());
}

inline void WriteFile(string name, string content)
{
	ofstream out(name.c_str());
	out << content;
	out.close();
}

vector<string> GetFilesInDirectory(char *dirname);
vector<string> GetDirectoriesInDirectory(char *dirname);

inline string GetFileExtension(string filename)
{
	return filename.substr(filename.find_last_of(".") + 1);
}

inline string GetPathFileName(string path)
{
	std::replace(path.begin(), path.end(), '\\', '/');
	int pos = path.find_last_of("/");
	return path.substr(pos == string::npos ? 0 : pos+1);
}

inline string GetFileName(string filename)
{
	filename = GetPathFileName(filename);
	return filename.substr(0, filename.find_last_of("."));
}

inline string GetFileDirectory(string path)
{
	std::replace(path.begin(), path.end(), '\\', '/');
	return path.substr(0, path.find_last_of("/"));
}

bool HasFileChanged(string path);


//////////////////////////////////////////////////////////////////////////
//
// Math
//
//////////////////////////////////////////////////////////////////////////

inline float sign(float x)
{
	return (x < 0.0f)? -1.0f : 1.0f;
}

inline float frand(float x=1.0f)
{
	return (rand() / (float) RAND_MAX) * x;
}

inline float frand(float a, float b)
{
	return a + frand(1.0f) * (b - a);
}

inline void swapf(float& a, float& b)
{
	float c = a;
	a = b;
	b = c;
}

/*inline float log2( float n )  
{  
	return log( n ) / log( 2.0f );  
}*/

inline float Pi()
{
	static const float pi = atan(1.0f) * 4.0f;
	return pi;
}

inline float TwoPi()
{
	static const float two_pi = 2.0f * Pi();
	return two_pi;
}

inline float RadiansToDegrees(float rad)
{
	static const float k = 180.0f / Pi();
	return rad * k;
}

inline float DegreesToRadians(float deg)
{
	static const float k = Pi() / 180.0f;
	return deg * k;
}

inline float QuadraticEaseOut( float k ) { return - k * ( k - 2 ); }                                        
inline float CubicEaseOut( float k ) { return --k * k * k + 1; }                                            
inline float CircularEaseOut( float k ) { return sqrt( 1 - --k * k ); }                                
inline float SinusoidalEaseOut( float k ) { return sin( k * M_PI / 2 ); }                           
inline float ExponentialEaseOut( float k ) { return k == 1 ? 1 : - pow( 2, - 10.0f * k ) + 1.0f; }          

inline float CheckAngle(float &angle)
{
	float i;
	if ( angle < 0 ) { i = floor(angle / 360.0f); angle -= i * 360.0f; }
	if ( angle > 360.0f ) { i = floor(angle / 360.0f); angle -= i * 360.0f; }
	return angle;
}

using namespace std;

class Vector3;
class Vector2;

void StepAngle(Vector3 &angle, Vector3 &targetAngle, float rotationSpeed, float frametime);
void StepLinear(float &current, float &end, float speed, float frametime);
void StepAngle(float &current, float end, float speed, float frametime);
float GetParabolicLaunchAngle(float g, float v, float x, float y);
bool SameSide(Vector3 p1, Vector3 p2, Vector3 a, Vector3 b);
bool IsPointInsideTriangle(Vector2 point, Vector2 v1, Vector2 v2, Vector2 v3);

//////////////////////////////////////////////////////////////////////////
//
// Interpolators
//
//////////////////////////////////////////////////////////////////////////
template <typename T>
T Lerp( T start, T end, float percent )
{
	return (start + percent*(end-start));
}

template <typename T>
T Slerp(T start, T end, float percent)
{
	// Dot product - the cosine of the angle between 2 vectors.
	float dot = T::Dot(start, end);     
	// Clamp it to be in the range of Acos()
	// This may be unnecessary, but floating point
	// precision can be a fickle mistress.
	clamp(dot, -1.0f, 1.0f);
	// Acos(dot) returns the angle between start and end,
	// And multiplying that by percent returns the angle between
	// start and the final result.
	float theta = acos(dot)*percent;
	Vector3 RelativeVec = end - start*dot;
	RelativeVec.Normalize();     // Orthonormal basis
	// The final result.
	return ((start*cos(theta)) + (RelativeVec*sin(theta)));
}

template <typename T>
T Nlerp(T start, T end, float percent)
{
	return Lerp(start,end,percent).Normalize();
}

//////////////////////////////////////////////////////////////////////////
//
// Other utilities
//
//////////////////////////////////////////////////////////////////////////

template<typename T>
inline void SetFlag(T &val, int flag, bool enable)
{
	if (enable)
		val |= flag;
	else
		val &= ~flag;
}

//////////////////////////////////////////////////////////////////////////
// Klasa Util, zrzesza wszystkie narzedzia uniwersalne
//////////////////////////////////////////////////////////////////////////
struct SUtils
{

	//////////////////////////////////////////////////////////////////////////
	// Zwraca dystans miedzy dwoma punktami
	static double Distance( int x, int y, int x2, int y2 )
	{
		return (double)(sqrt( (float)((x2-x)*(x2-x) + (y2-y)*(y2-y))));
	}

	//////////////////////////////////////////////////////////////////////////
	// Zwraca dystans miedzy dwoma punktami
	static double Distancef( float x, float y, float x2, float y2 )
	{
		return (double)(sqrt( (float)((x2-x)*(x2-x) + (y2-y)*(y2-y))));
	}

	//////////////////////////////////////////////////////////////////////////
	// Zwraca dystans punkty C od linii AB
	static void DistanceFromLine(	double cx, double cy, double ax, double ay ,
									double bx, double by, double &distanceSegment,
									double &distanceLine)
	{
		double r_numerator = (cx-ax)*(bx-ax) + (cy-ay)*(by-ay);
		double r_denomenator = (bx-ax)*(bx-ax) + (by-ay)*(by-ay);
		double r = r_numerator / r_denomenator;
		double px = ax + r*(bx-ax);
		double py = ay + r*(by-ay);
		double s =  ((ay-cy)*(bx-ax)-(ax-cx)*(by-ay) ) / r_denomenator;
		distanceLine = fabs(s)*sqrt(r_denomenator);
		double xx = px;
		double yy = py;
		if ( (r >= 0) && (r <= 1) )
		{
			distanceSegment = distanceLine;
		}
		else
		{
			double dist1 = (cx-ax)*(cx-ax) + (cy-ay)*(cy-ay);
			double dist2 = (cx-bx)*(cx-bx) + (cy-by)*(cy-by);
			if (dist1 < dist2)
			{
				xx = ax;
				yy = ay;
				distanceSegment = sqrt(dist1);
			}
			else
			{
				xx = bx;
				yy = by;
				distanceSegment = sqrt(dist2);
			}
		}
		return;
	}
};

//////////////////////////////////////////////////////////////////////////
// String utilities
//////////////////////////////////////////////////////////////////////////
namespace String 
{
	inline string GetFilename(string text)
	{
		int i = text.rfind("/");
		int j = text.rfind("\\");
		if (j == text.npos) j = 0;
		if (i == text.npos) i = j;
		if (j < i) i = j;
		return text.substr(i);
	}

	inline string GetFilenameExt(string filename)
	{
		int i = filename.find_last_of('.');
		if (i == filename.npos) return string("");
		return filename.substr(i+1);
	}
}



//////////////////////////////////////////////////////////////////////////
#define mathsInnerProduct(v,q) \
	((v)[0] * (q)[0] + \
	(v)[1] * (q)[1] + \
	(v)[2] * (q)[2])	

//#define DotProduct(x,y)			((x)[0]*(y)[0]+(x)[1]*(y)[1]+(x)[2]*(y)[2])
#define VectorSubtract(a,b,c)	((c)[0]=(a)[0]-(b)[0],(c)[1]=(a)[1]-(b)[1],(c)[2]=(a)[2]-(b)[2])
#define VectorAdd(a,b,c)		((c)[0]=(a)[0]+(b)[0],(c)[1]=(a)[1]+(b)[1],(c)[2]=(a)[2]+(b)[2])
#define VectorCopy(a,b)			((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2])
#define	VectorScale(v, s, o)	((o)[0]=(v)[0]*(s),(o)[1]=(v)[1]*(s),(o)[2]=(v)[2]*(s))
#define VectorClear(a)			((a)[0]=(a)[1]=(a)[2]=0)

#define ARGB_A(u) (((u)>>24) & 0x000000FF)
#define ARGB_R(u) (((u)>>16) & 0x000000FF)
#define ARGB_G(u) (((u)>> 8) & 0x000000FF)
#define ARGB_B(u) (((u)>> 0) & 0x000000FF)

/* a = b x c */

#define mathsCrossProduct(a,b,c) \
	(a)[0] = (b)[1] * (c)[2] - (c)[1] * (b)[2]; \
	(a)[1] = (b)[2] * (c)[0] - (c)[2] * (b)[0]; \
	(a)[2] = (b)[0] * (c)[1] - (c)[0] * (b)[1];


/* vector a = b - c, where b and c represent points*/

#define mathsVector(a,b,c) \
	(a)[0] = (b)[0] - (c)[0];	\
	(a)[1] = (b)[1] - (c)[1];	\
	(a)[2] = (b)[2] - (c)[2];

inline void mathsNormalize(float *v) 
{
	float d = (sqrt((v[0]*v[0]) + (v[1]*v[1]) + (v[2]*v[2])));
	v[0] = v[0] / d;
	v[1] = v[1] / d;
	v[2] = v[2] / d;
}


///////////////////////////////////////////////////// ti/////////////////////

struct SRGBA;

struct SRGB
{
	byte r,g,b;

	SRGB(){};
	SRGB(byte _r,byte _g,byte _b){r=_r;g=_g;b=_b;};
	SRGB& operator=(const SRGB &rgb){r=rgb.r;g=rgb.g;b=rgb.b;return *this;};
	DWORD ToInt32() { return (r << 24) & (g << 16) & (b << 8) & 0xff; }	

	operator D3DXCOLOR() const { return D3DXCOLOR( (float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f, 1.0f); };
	operator Vector3() const { return Vector3( (float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f); };
	operator Vector4() const { return Vector4( (float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f, 1.0f); };
};

struct SRGBA
{
	byte r,g,b,a;

	SRGBA(){ r=g=b=a=255; };
	SRGBA(string hex) { FromHex(hex); }
	SRGBA(byte _r,byte _g,byte _b,byte _a=255){r=_r;g=_g;b=_b;a=_a;};
	SRGBA(SRGB &rgb){r=rgb.r;g=rgb.g;b=rgb.b;a=255;};
	SRGBA& operator=(const SRGBA &rgb){r=rgb.r;g=rgb.g;b=rgb.b;a=rgb.a;return *this;};
	SRGBA& operator=(const SRGB &rgb){r=rgb.r;g=rgb.g;b=rgb.b;a=255;return *this;};
	DWORD ToInt32() { return (r << 24) | (g << 16) | (b << 8) | a; }	
	string ToHex()
	{
		char s[128];
		sprintf_s(s, "%.2X%.2X%.2X%.2X", r, g, b, a);
		return s;
	}
	void FromHex(string hex)
	{	
		int _r, _g, _b, _a;
		int result = sscanf(hex.c_str(), "%2X%2X%2X%2X", &_r, &_g, &_b, &_a);
		r = _r;
		g = _g;
		b = _b;
		a = _a;
	}

	operator SRGB() const { return SRGB(r,g,b); }
	operator D3DXCOLOR() const { return D3DXCOLOR( (float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f, (float)a / 255.0f); };
	operator Vector3() const { return Vector3( (float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f); };
	operator Vector4() const { return Vector4( (float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f, (float)a / 255.0f); };
};

#define RED			SRGBA(255,0,0)
#define GREEN		SRGBA(0,255,0)
#define BLUE		SRGBA(0,0,255)
#define YELLOW		SRGBA(255,255,0)
#define WHITE		SRGBA(255,255,255)
#define BLACK		SRGBA(0,0,0)
#define MAGENTA		SRGBA(255,0,255)
#define GREY		SRGBA(155,155,155)
#define DARKGREY	SRGBA(55,55,55)
#define TRANSPARENT	SRGBA(0,0,0,0)
#define ORANGE		SRGBA(255,168,0)

#include "MathUtils.h"
#include "DXUtils.h"


template<typename Data>
class concurrent_queue
{
private:
	std::queue<Data> the_queue;
	mutable boost::mutex the_mutex;
	boost::condition_variable the_condition_variable;
public:
	void push(Data const& data)
	{
		boost::mutex::scoped_lock lock(the_mutex);
		the_queue.push(data);
		lock.unlock();
		the_condition_variable.notify_one();
	}

	bool empty() const
	{
		boost::mutex::scoped_lock lock(the_mutex);
		return the_queue.empty();
	}

	bool try_pop(Data& popped_value)
	{
		boost::mutex::scoped_lock lock(the_mutex);
		if(the_queue.empty())
		{
			return false;
		}

		popped_value=the_queue.front();
		the_queue.pop();
		return true;
	}

	void wait_and_pop(Data& popped_value)
	{
		boost::mutex::scoped_lock lock(the_mutex);
		while(the_queue.empty())
		{
			the_condition_variable.wait(lock);
		}

		popped_value=the_queue.front();
		the_queue.pop();
	}

};


//////////////////////////////////////////////////////////////////////////
// Quad Tree
//////////////////////////////////////////////////////////////////////////
template <typename T>
class CQuadTree
{
public:
	CQuadTree() {};
	CQuadTree(T data): hasChildren(false) 
	{
		this->data = data;
		children[0] = children[1] = children[2] = children[3] = NULL;
	};
	~CQuadTree()
	{
		if (!hasChildren) return;
		SAFE_DELETE(children[0]);
		SAFE_DELETE(children[1]);
		SAFE_DELETE(children[2]);
		SAFE_DELETE(children[3]);
	}
	
	T					data;
	bool				hasChildren;
	class CQuadTree<T>	*children[4];
};



