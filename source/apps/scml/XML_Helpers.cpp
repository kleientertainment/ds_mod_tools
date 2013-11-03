#include "XML_Helpers.h"
#include <stdio.h>
#include <stdarg.h>

// Visual Studio does not support snprintf properly.
// This is from Valentin Milea on Stack Overflow.  http://stackoverflow.com/questions/2915672/snprintf-and-visual-studio-2010/8712996#8712996
#if defined(_MSC_VER) && !defined(MARMALADE)

    int c99_vsnprintf(char* str, size_t size, const char* format, va_list ap)
    {
        int count = -1;

        if (size != 0)
            count = _vsnprintf_s(str, size, _TRUNCATE, format, ap);
        if (count == -1)
            count = _vscprintf(format, ap);

        return count;
    }

    int c99_snprintf(char* str, size_t size, const char* format, ...)
    {
        int count;
        va_list ap;

        va_start(ap, format);
        count = c99_vsnprintf(str, size, format, ap);
        va_end(ap);

        return count;
    }
#else
    #include "libgen.h"
#endif // _MSC_VER


SCML_STRING toLower(const SCML_STRING& str)
{
    SCML_STRING result = str;
    for(unsigned int i = 0; i < SCML_STRING_SIZE(result); i++)
    {
        result[i] = tolower(result[i]);
    }
    return result;
}


bool toBool(const SCML_STRING& str)
{
    if(toLower(str) == "true")
        return true;
    if(toLower(str) == "false")
        return false;
    return atoi(SCML_TO_CSTRING(str)) != 0;
}

int toInt(const SCML_STRING& str)
{
    return atoi(SCML_TO_CSTRING(str));
}

float toFloat(const SCML_STRING& str)
{
    return (float)atof(SCML_TO_CSTRING(str));
}



SCML_STRING toString(bool b)
{
    return (b? "true" : "false");
}

SCML_STRING toString(int n)
{
    char buf[20];
    sprintf(buf, "%d", n);
    return buf;
}

char* stripTrailingDecimalZeros(char* buf)
{
    if(buf == NULL)
        return NULL;
    
    char* c = buf;
    
    // See if it has a decimal
    bool hasDecimal = false;
    while(*c != '\0')
    {
        if(*c == '.')
            hasDecimal = true;
        c++;
    }
    
    // If there is a decimal part, remove all the trailing zeros and the decimal point if we reach it.
    if(hasDecimal)
    {
        while(c != buf)
        {
            c--;
            if(*c == '0')
                *c = '\0';
            else
            {
                if(*c == '.')
                    *c = '\0';
                break;
            }
        }
    }
    
    return buf;
}

SCML_STRING toString(float f, int precision)
{
    char buf[20];
    if(precision >= 0)
        snprintf(buf, 20, "%.*f", precision, f);
    else
    {
        snprintf(buf, 20, "%f", f);
        stripTrailingDecimalZeros(buf);
    }
    return buf;
}












bool xmlAttrExists(TiXmlElement* elem, const SCML_STRING& attribute)
{
    return (elem->Attribute(SCML_TO_CSTRING(attribute)) != NULL);
}

SCML_STRING xmlGetStringAttr(TiXmlElement* elem, const SCML_STRING& attribute)
{
    const char* attr = elem->Attribute(SCML_TO_CSTRING(attribute));
    if(attr == NULL)
    {
        printf("Failed to load attribute '%s' from '%s' element.\n", SCML_TO_CSTRING(attribute), elem->Value());
        return "";
    }
    return attr;
}

SCML_STRING xmlGetStringAttr(TiXmlElement* elem, const SCML_STRING& attribute, const SCML_STRING& defaultValue)
{
    const char* attr = elem->Attribute(SCML_TO_CSTRING(attribute));
    if(attr == NULL)
        return defaultValue;
    return attr;
}

bool xmlGetBoolAttr(TiXmlElement* elem, const SCML_STRING& attribute)
{
    const char* attr = elem->Attribute(SCML_TO_CSTRING(attribute));
    if(attr == NULL)
    {
        printf("Failed to load attribute '%s' from '%s' element.\n", SCML_TO_CSTRING(attribute), elem->Value());
        return false;
    }
    return toBool(attr);
}

bool xmlGetBoolAttr(TiXmlElement* elem, const SCML_STRING& attribute, bool defaultValue)
{
    const char* attr = elem->Attribute(SCML_TO_CSTRING(attribute));
    if(attr == NULL)
        return defaultValue;
    return toBool(attr);
}

int xmlGetIntAttr(TiXmlElement* elem, const SCML_STRING& attribute)
{
    const char* attr = elem->Attribute(SCML_TO_CSTRING(attribute));
    if(attr == NULL)
    {
        printf("Failed to load attribute '%s' from '%s' element.\n", SCML_TO_CSTRING(attribute), elem->Value());
        return 0;
    }
    return toInt(attr);
}

int xmlGetIntAttr(TiXmlElement* elem, const SCML_STRING& attribute, int defaultValue)
{
    const char* attr = elem->Attribute(SCML_TO_CSTRING(attribute));
    if(attr == NULL)
        return defaultValue;
    return toInt(attr);
}

int xmlGetIntAttr(TiXmlElement* elem, const SCML_STRING& attribute, int defaultValue, bool& found )
{
    const char* attr = elem->Attribute(SCML_TO_CSTRING(attribute));
    found = attr != NULL;
    if(!found)
    {
        return defaultValue;
    }
    return toInt(attr);
}

float xmlGetFloatAttr(TiXmlElement* elem, const SCML_STRING& attribute)
{
    const char* attr = elem->Attribute(SCML_TO_CSTRING(attribute));
    if(attr == NULL)
    {
        printf("Failed to load attribute '%s' from '%s' element.\n", SCML_TO_CSTRING(attribute), elem->Value());
        return 0.0f;
    }
    return toFloat(attr);
}


float xmlGetFloatAttr(TiXmlElement* elem, const SCML_STRING& attribute, float defaultValue)
{
    const char* attr = elem->Attribute(SCML_TO_CSTRING(attribute));
    if(attr == NULL)
        return defaultValue;
    return toFloat(attr);
}

float xmlGetFloatAttr(TiXmlElement* elem, const SCML_STRING& attribute, float defaultValue, bool& found)
{
    const char* attr = elem->Attribute(SCML_TO_CSTRING(attribute));
    found = attr != NULL;
    if(!found)
    {
        return defaultValue;
    }
    return toFloat(attr);
}



