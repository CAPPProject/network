#ifndef _MOON_STRING_H
#define _MOON_STRING_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <ctype.h>
using namespace std;

/**
 * Gets the offset of one string in another string
 */
int moon_str_index_of(const char *a, char *b);

/**
 *	Checks if one string contains another string
 */
int moon_str_contains(const char *haystack, const char *needle);

/**
 *	Removes last character from string
 */
char* moon_trim_end(char *string, char to_trim);

/**
 *	Concecates two strings, a wrapper for strcat from string.h, handles the resizing and copying
 */
char* moon_str_cat(char *a, char *b);

/** 
 *	Converts an integer value to its hex character
 */
char moon_to_hex(char code);

/**
 *	URL encodes a string
 */
char * moon_urlencode(char *str);

/**
 *	Replacement for the string.h strndup, fixes a bug
 */
char *moon_str_ndup (const char *str, size_t max);

/**
 *	Replacement for the string.h strdup, fixes a bug
 */
char *moon_str_dup(const char *src);

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
char *moon_str_replace(const char *search , char *replace , char *subject);

/**
 *	Get's all characters until '*until' has been found
 */
char* moon_get_until(char *haystack, char *until);


/* decodeblock - decode 4 '6-bit' characters into 3 8-bit binary bytes */
void moon_decodeblock(unsigned char in[], char *clrstr);

/**
 *	Decodes a Base64 string
 */
char* moon_base64_decode(char *b64src);

/* encodeblock - encode 3 8-bit binary bytes as 4 '6-bit' characters */
void moon_encodeblock( unsigned char in[], char b64str[], int len );

/**
 *	Encodes a string with Base64
 */
char* moon_base64_encode(char *clrstr);

/**get string length*/
long moon_string_length(const char* str);

/**
 * 去掉字符串的首尾空格
 */
string moon_string_trim(string str);

/**
 * 十六进制转十进制
 */
int htoi(char *s);

/**
 * url解码
 */
string url_decode(string &str_source);

//将UTF-8转到Unicode
wstring utf8_to_unicode( const string& str );

//将unicode转到ascii
string unicode_to_ascii( const wstring& str );

//字符串替换
string string_replace(string source,string oldstr,string newstr);

#endif