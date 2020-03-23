/***********************************************************************
 *  file desc
 *		the http client that implement GET/POST request
 *	time:
 *		2018-05-23 9:49
 *	maker:
 *		haoran dai
 ***********************************************************************/
#ifndef _MOON_HTTP_CLIENT_H
#define _MOON_HTTP_CLINET_H

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
#include "../Common/moon_string.h"
#include "../common/moon_url_parse.h"
#include "../api/define/moon_http.h"


#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif



/**
 *	Handles redirect if needed for get requests
 */
http_response* moon_handle_redirect_get(http_response* hresp, char* custom_headers);

/**
 *	Handles redirect if needed for head requests
 */
http_response* moon_handle_redirect_head(http_response* hresp, char* custom_headers);

/**
 *	Handles redirect if needed for post requests
 */
http_response* moon_handle_redirect_post(http_response* hresp, char* custom_headers, char *post_data);

/**
 *	Makes a HTTP request and returns the response
 */
http_response* moon_http_request(char *http_headers, parsed_url *purl);

/**
 *	Makes a HTTP GET request to the given url
 */
http_response* moon_http_get(char *url, char *custom_headers);

/**
 *	Makes a HTTP POST request to the given url
 */
http_response* moon_http_post(char *url, char *custom_headers, char *post_data);

/**
 *	Makes a HTTP HEAD request to the given url
 */
http_response* moon_http_head(char *url, char *custom_headers);



/**
 *	Do HTTP OPTIONs requests
 */
http_response* moon_http_options(char *url);

#endif