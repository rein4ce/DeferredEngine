#include "platform.h"
#include "Config.h"

ConfigFile CConfig::Read(string filename)
{
	ifstream fs(filename.c_str());
	ConfigFile ret;
	string group;

	boost::match_results<string::const_iterator> what;
	boost::regex reGroup("^\\[(.+)\\]");
	boost::regex reProperty("^(\\S+)\\s*=\\s*\"?([^\"]+)\"?");

	string line;
	while (getline(fs, line))
	{
		if (line[0] == '[' && boost::regex_match(line, what, reGroup))
		{
			string name = string(what[1].first, what[1].second);
			group = name;
			ret[group];			
			continue;
		}

		if (boost::regex_match(line, what, reProperty))
		{
			string name = string(what[1].first, what[1].second);
			string value = string(what[2].first, what[2].second);			
			ret[group][name] = SConfigValue(value);
		}
	}
	
	return ret;
}