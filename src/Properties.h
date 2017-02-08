#pragma once

enum 
{
	PROPERTY_DEFAULT,
	PROPERTY_STRING,
	PROPERTY_NUMBER,
	PROPERTY_CHECKBOX,
	PROPERTY_FILE
};

struct SProperty
{
	string			name;
	int				type;

	SProperty()
	{
		type = PROPERTY_DEFAULT;
	}

	virtual string		GetValue() { return "<invalid>"; };
	virtual void		SetValue(string value) {};
	virtual int			GetType() { return PROPERTY_STRING; };
};

template <typename T>
struct SPropertyDefine : SProperty
{	
	T				*pVariable;

	SPropertyDefine(string name, T *p, int type = PROPERTY_DEFAULT)
	{
		this->name = name;
		this->pVariable = p;
		this->type = type;
	}

	virtual string GetValue()
	{
		Assert(pVariable);
		stringstream ss;
		ss << *pVariable;	
		return ss.str();
	}

	virtual void		SetValue(string value);
	virtual int			GetType();
};

template <>
inline string SPropertyDefine<float>::GetValue()
{
	Assert(pVariable);
	stringstream ss;
	ss.precision(3);
	ss << *pVariable;	
	return ss.str();
}

template <>
inline string SPropertyDefine<double>::GetValue()
{
	Assert(pVariable);
	stringstream ss;
	ss.precision(3);
	ss << *pVariable;	
	return ss.str();
}

template <>
inline string SPropertyDefine<int>::GetValue()
{
	Assert(pVariable);
	stringstream ss;
	ss << *pVariable;	
	return ss.str();
}

template <>
inline string SPropertyDefine<bool>::GetValue()
{
	Assert(pVariable);
	return *pVariable == true ? "true" : "false";	
}

template <>
inline string SPropertyDefine<SRGBA>::GetValue()
{
	Assert(pVariable); 
	return pVariable->ToHex();	
}



template <>
inline void SPropertyDefine<float>::SetValue(string text)
{
	*pVariable = atof(text.c_str());
}

template <>
inline void SPropertyDefine<double>::SetValue(string text)
{
	*pVariable = atof(text.c_str());
}

template <>
inline void SPropertyDefine<int>::SetValue(string text)
{
	*pVariable = atoi(text.c_str());
}

template <>
inline void SPropertyDefine<string>::SetValue(string text)
{
	*pVariable = text;
}

template <>
inline void SPropertyDefine<bool>::SetValue(string text)
{
	if (text == "true") 
		*pVariable = true;
	else
		*pVariable = false;
}

template <>
inline void SPropertyDefine<SRGBA>::SetValue(string text)
{
	pVariable->FromHex(text);
}


template <> inline int SPropertyDefine<float>::GetType() { return type == PROPERTY_DEFAULT ? PROPERTY_NUMBER : type; }
template <> inline int SPropertyDefine<double>::GetType() { return type == PROPERTY_DEFAULT ? PROPERTY_NUMBER : type; }
template <> inline int SPropertyDefine<string>::GetType() { return type == PROPERTY_DEFAULT ? PROPERTY_STRING : type; }
template <> inline int SPropertyDefine<bool>::GetType() { return type == PROPERTY_DEFAULT ? PROPERTY_CHECKBOX : type; }
template <> inline int SPropertyDefine<SRGBA>::GetType() { return type == PROPERTY_DEFAULT ? PROPERTY_STRING : type; }