#ifndef _XML_HELPERS_H__
#define _XML_HELPERS_H__


#include "SCMLpp.h"
#include "tinyxml.h"

bool toBool(const SCML_STRING& str);
int toInt(const SCML_STRING& str);
float toFloat(const SCML_STRING& str);

SCML_STRING toString(bool b);
SCML_STRING toString(int n);
SCML_STRING toString(float f, int precision = -1);


bool xmlGetBoolAttr(TiXmlElement* elem, const SCML_STRING& attribute);
bool xmlGetBoolAttr(TiXmlElement* elem, const SCML_STRING& attribute, bool defaultValue);
SCML_STRING xmlGetStringAttr(TiXmlElement* elem, const SCML_STRING& attribute);
SCML_STRING xmlGetStringAttr(TiXmlElement* elem, const SCML_STRING& attribute, const SCML_STRING& default_value);
int xmlGetIntAttr(TiXmlElement* elem, const SCML_STRING& attribute);
int xmlGetIntAttr(TiXmlElement* elem, const SCML_STRING& attribute, int default_value);
int xmlGetIntAttr(TiXmlElement* elem, const SCML_STRING& attribute, int default_value, bool& found );
float xmlGetFloatAttr(TiXmlElement* elem, const SCML_STRING& attribute);
float xmlGetFloatAttr(TiXmlElement* elem, const SCML_STRING& attribute, float default_value);
float xmlGetFloatAttr(TiXmlElement* elem, const SCML_STRING& attribute, float default_value, bool& found);

#if defined(_MSC_VER) && !defined(MARMALADE)
    #define snprintf c99_snprintf
    int c99_snprintf(char* str, size_t size, const char* format, ...);
#endif

#endif
