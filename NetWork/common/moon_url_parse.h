/***********************************************************************
 * file desc:
 *	this file is about of parse url
 * time:
 *	2018-05-23
 * maker:
 *	haoran dai
 ***********************************************************************/

#ifndef _MOON_URL_H
#define _MOON_URL_H

#pragma GCC diagnostic ignored "-Wwrite-strings"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#pragma comment(lib, "Ws2_32.lib")
#elif _LINUX
#include <sys/socket.h>
#elif __FreeBSD__
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#else
#error Platform not suppoted.
#endif

#include <errno.h>
#include "../api/define/moon_http.h"


#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

/*define NULL */
#ifndef NULL
#define NULL 0
#endif


/**
 *	Free memory of parsed url
 */
void moon_parsed_url_free(p_parsed_url purl);

/**
 * function desc:
 *	Retrieves the IP adress of a hostname
 */
char* moon_hostname_to_ip(char *hostname);

/**
 * Check whether the character is permitted in scheme string
 */
int moon_is_scheme_char(int c);

/**
 *	Parses a specified URL and returns the structure named 'parsed_url'
 *	Implented according to:
 *	RFC 1738 - http://www.ietf.org/rfc/rfc1738.txt
 *	RFC 3986 -  http://www.ietf.org/rfc/rfc3986.txt
 */
p_parsed_url moon_parse_url(const char *url);

#endif