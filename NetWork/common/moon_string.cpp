#include "moon_string.h"
#include <Windows.h>

/**
 * Gets the offset of one string in another string
 */
int moon_str_index_of(const char *a, char *b)
{
	char *offset = (char*)strstr(a, b);
	return offset - a;
}

/**
 *	Checks if one string contains another string
 */
int moon_str_contains(const char *haystack, const char *needle)
{
	char *pos = (char*)strstr(haystack, needle);
	if(pos)
		return 1;
	else
		return 0;
}

/**
 *	Removes last character from string
 */
char* moon_trim_end(char *string, char to_trim)
{
	char last_char = string[strlen(string) -1];
	if(last_char == to_trim)
	{
		char *new_string = string;
		new_string[strlen(string) - 1] = 0;
		return new_string;
	}
	else
	{
		return string;
	}
}

/**
 *	Concecates two strings, a wrapper for strcat from string.h, handles the resizing and copying
 */
char* moon_str_cat(char *a, char *b)
{
	char *target = (char*)malloc(strlen(a) + strlen(b) + 1);
	strcpy(target, a);
	strcat(target, b);
	return target;
}

/** 
 *	Converts an integer value to its hex character
 */
char moon_to_hex(char code) 
{
	static char hex[] = "0123456789abcdef";
	return hex[code & 15];
}

/**
 *	URL encodes a string
 */
char * moon_urlencode(char *str) 
{
	char *pstr = str, *buf = (char*)malloc(strlen(str) * 3 + 1), *pbuf = buf;
  	while (*pstr) 
	{
    	if (isalnum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.' || *pstr == '~') 
      		*pbuf++ = *pstr;
    	else if (*pstr == ' ') 
      		*pbuf++ = '+';
    	else 
      		*pbuf++ = '%', *pbuf++ = moon_to_hex(*pstr >> 4), *pbuf++ = moon_to_hex(*pstr & 15);
    	pstr++;
  	}
  	*pbuf = '\0';
  	return buf;
}

/**
 *	Replacement for the string.h strndup, fixes a bug
 */
char *moon_str_ndup (const char *str, size_t max)
{
    size_t len = strnlen (str, max);
    char *res = (char*)malloc (len + 1);
	memset(res,0,len + 1);
    if (res)
    {
        memcpy (res, str, len);
        res[len] = '\0';
    }
    return res;
}

/**
 *	Replacement for the string.h strdup, fixes a bug
 */
char *moon_str_dup(const char *src)
{
   char *tmp = (char*)malloc(strlen(src) + 1);
   if(tmp)
       strcpy(tmp, src);
   return tmp;
}

/**
 * Function desc:
 *	Search and replace a string with another string , in a string
 * Params:
 *	search:the string for search
 *	replace:the string for replace
 *	subject:source string
 * return:
 *	Returns the replaced string
 */
char *moon_str_replace(const char *search , char *replace , char *subject)
{
	char  *p = NULL , *old = NULL , *new_subject = NULL ;
	int c = 0 , search_size;
	search_size = strlen(search);
	//Calculates the number of occurrences of the replaced string
	for(p = strstr(subject , search) ; p != NULL ; p = strstr(p + search_size , search))
	{
		c++;
	}
	//Calculating the replaced string requires memory size
	c = ( strlen(replace) - search_size )*c + strlen(subject) + 1;
	new_subject = (char*)malloc( c );
	//strcpy(new_subject , "");
	memset(new_subject,0,c);
	old = subject;	
	for(p = strstr(subject , search) ; p != NULL ; p = strstr(p + search_size , search))
	{
		strncpy(new_subject + strlen(new_subject) , old , p - old);
		strcpy(new_subject + strlen(new_subject) , replace);
		old = p + search_size;
	}
	strcpy(new_subject + strlen(new_subject) , old);	
	return new_subject;
}

/**
 *	Get's all characters until '*until' has been found
 */
char* moon_get_until(char *haystack, char *until)
{
	int offset = moon_str_index_of(haystack, until);
	return moon_str_ndup(haystack, offset);
}

/* decodeblock - decode 4 '6-bit' characters into 3 8-bit binary bytes */
void moon_decodeblock(unsigned char in[], char *clrstr) 
{
	unsigned char out[4];
	out[0] = in[0] << 2 | in[1] >> 4;
	out[1] = in[1] << 4 | in[2] >> 2;
	out[2] = in[2] << 6 | in[3] >> 0;
	out[3] = '\0';
	strncat((char *)clrstr, (char *)out, sizeof(out));
}

/**
 *	Decodes a Base64 string
 */
char* moon_base64_decode(char *b64src) 
{
	char *clrdst = (char*)malloc( ((strlen(b64src) - 1) / 3 ) * 4 + 4 + 50);
	char b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	int c, phase, i;
	unsigned char in[4];
	char *p;
	clrdst[0] = '\0';
	phase = 0; i=0;
	while(b64src[i]) 
	{
		c = (int) b64src[i];
		if(c == '=') 
		{
			moon_decodeblock(in, clrdst); 
			break;
		}
		p = strchr(b64, c);
		if(p) 
		{
			in[phase] = p - b64;
			phase = (phase + 1) % 4;
			if(phase == 0) 
			{
				moon_decodeblock(in, clrdst);
				in[0]=in[1]=in[2]=in[3]=0;
			}
		}
		i++;
	}
	clrdst = (char*)realloc(clrdst, strlen(clrdst) + 1);
	return clrdst;
}

/* encodeblock - encode 3 8-bit binary bytes as 4 '6-bit' characters */
void moon_encodeblock( unsigned char in[], char b64str[], int len ) 
{
	char b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	unsigned char out[5];
	out[0] = b64[ in[0] >> 2 ];
	out[1] = b64[ ((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4) ];
	out[2] = (unsigned char) (len > 1 ? b64[ ((in[1] & 0x0f) << 2) |
		((in[2] & 0xc0) >> 6) ] : '=');
	out[3] = (unsigned char) (len > 2 ? b64[ in[2] & 0x3f ] : '=');
	out[4] = '\0';
	strncat((char *)b64str, (char *)out, sizeof(out));
}


/**
 *	Encodes a string with Base64
 */
char* moon_base64_encode(char *clrstr) 
{
	char *b64dst = (char*)malloc(strlen(clrstr) + 50);
	char b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	unsigned char in[3];
	int i, len = 0;
	int j = 0;

	b64dst[0] = '\0';
	while(clrstr[j]) 
	{
		len = 0;
		for(i=0; i<3; i++) 
		{
			in[i] = (unsigned char) clrstr[j];
			if(clrstr[j]) 
			{
				len++; j++;
			}
			else in[i] = 0;
		}
		if( len ) 
		{
			moon_encodeblock( in, b64dst, len );
		}
	}
	b64dst = (char*)realloc(b64dst, strlen(b64dst) + 1);
	return b64dst;
}

/**get string length*/
long moon_string_length(const char* str)
{
	long index = 0;
	if (str == NULL)
	{
		return 0;
	}
	while(str[index] != '\0')
	{
		index++;
	}
	return index;
}

/**
 * 去掉字符串的首尾空格
 */
string moon_string_trim(string str)
{
	string tmpStr = str;
	if( !tmpStr.empty() )
    {
        tmpStr.erase(0,tmpStr.find_first_not_of(" "));
        tmpStr.erase(tmpStr.find_last_not_of(" ") + 1);
    }
	return tmpStr;
}

/**
 * 十六进制转十进制
 */
int htoi(char *s)
{
	int value;  
	int c;  

	c = ((unsigned char *)s)[0];  
	if (isupper(c))  
		c = tolower(c);  
	value = (c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10) * 16;  

	c = ((unsigned char *)s)[1];  
	if (isupper(c))  
		c = tolower(c);  
	value += c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10;  

	return (value);
}

/**
 * url解码
 */
string url_decode(string &str_source)
{
	char const *in_str = str_source.c_str();  
	int in_str_len = strlen(in_str);  
	int out_str_len = 0;  
	string out_str;  
	char *str;  

	str = _strdup(in_str);  
	char *dest = str;  
	char *data = str;  

	while (in_str_len--) {  
		if (*data == '+') {  
			*dest = ' ';  
		}  
		else if (*data == '%' && in_str_len >= 2 && isxdigit((int) *(data + 1))   
			&& isxdigit((int) *(data + 2))) {  
				*dest = (char) htoi(data + 1);  
				data += 2;  
				in_str_len -= 2;  
		} else {  
			*dest = *data;  
		}  
		data++;  
		dest++;  
	}  
	*dest = '\0';  
	out_str_len =  dest - str;  
	out_str = str;  
	free(str);  
	return out_str;
}

//将UTF-8转到Unicode
wstring utf8_to_unicode( const string& str )
{
	int  len = 0;
	len = str.length();
	int  unicodeLen = ::MultiByteToWideChar( CP_UTF8,
		0,
		str.c_str(),
		-1,
		NULL,
		0 );  
	wchar_t *  pUnicode;  
	pUnicode = new  wchar_t[unicodeLen+1];  
	memset(pUnicode,0,(unicodeLen+1)*sizeof(wchar_t));  
	::MultiByteToWideChar( CP_UTF8,
		0,
		str.c_str(),
		-1,
		(LPWSTR)pUnicode,
		unicodeLen );  
	wstring  rt;  
	rt = ( wchar_t* )pUnicode;
	delete  pUnicode; 

	return  rt;  
}

//将unicode转到ascii
string unicode_to_ascii( const wstring& str )
{
	char*     pElementText;
	int    iTextLen;
	// wide char to multi char
	iTextLen = WideCharToMultiByte( CP_ACP,
		0,
		str.c_str(),
		-1,
		NULL,
		0,
		NULL,
		NULL );
	pElementText = new char[iTextLen + 1];
	memset( ( void* )pElementText, 0, sizeof( char ) * ( iTextLen + 1 ) );
	::WideCharToMultiByte( CP_ACP,
		0,
		str.c_str(),
		-1,
		pElementText,
		iTextLen,
		NULL,
		NULL );
	string strText;
	strText = pElementText;
	delete[] pElementText;
	return strText;
}

//字符串替换
string string_replace(string source,string oldstr,string newstr)
{
	int pos = -1;
	while((pos = source.find(oldstr, 0)) != std::string::npos)
	{
		source.replace(pos, oldstr.length(), newstr);
	}
	return source;
}