#pragma once;
#include "platform.h"
#include <unordered_map>
#include <boost/regex.hpp>
#include <boost\algorithm\string\case_conv.hpp>
#include "Utils.h"

struct SRGBA;

struct SConfigValue
{
	bool set;
	string value;

	inline string	GetString()		{ return value; };
	inline int		GetInteger()	{ return atoi(value.c_str()); };
	inline bool		GetBool()		{ return atoi(value.c_str()) == 1; };
	inline float	GetFloat()		{ return atof(value.c_str()); };
	inline SRGBA	GetColor()
	{
		boost::regex reColor("^\\((\\d+),(\\d+),(\\d+),?(\\d+)?\\)");
		boost::match_results<string::const_iterator> what;
		if (boost::regex_match(value, what, reColor))
		{
			int R = atoi(string(what[1].first, what[1].second).c_str());
			int G = atoi(string(what[2].first, what[2].second).c_str());
			int B = atoi(string(what[3].first, what[3].second).c_str());
			int A = 255;
			if (what.size() > 4)
				A = atoi(string(what[4].first, what[4].second).c_str());
			return SRGBA(R,G,B,A);
		}
		return TRANSPARENT;
	}

	SConfigValue() { set = false; }
	SConfigValue(string val) { value = val; set = true; }
};

typedef std::unordered_map<string, std::unordered_map<string, SConfigValue> > ConfigFile;

class CConfig
{
public:
	static ConfigFile	Read(string filename);
};
