#include "../stdafx.h"
#include "http_client.h"
#include "../http/moon_http_client.h"
#include "../https/moon_https_client.h"
#include <string>

http_response* http_get(const char *url)
{
	return moon_http_get_ex(url);
}

/**
 * http post request
 */
http_response* http_post(const char *url,const char *params)
{
	return moon_http_post_ex(url,params);
}

http_response* https_get(const char *url)
{
	return moon_https_get(url);
}

http_response* https_get_ex(const char* url,const char* headerName[],const char* headerValue[],int headerCount)
{
	return moon_https_get_ex(url,headerName,headerValue,headerCount);
}

/**
 * https post request
 */
http_response* https_post(const char *url,const char* params)
{
	return moon_https_post(url,params);
}

/**
 * https get request
 */
http_response* https_post_ex(const char* url,const char* params,const char* headerName[],const char* headerValue[],int headerCount)
{
	return moon_https_post_ex(url,params,headerName,headerValue,headerCount);
}

/**
 * http download
 */
HTTP_CLIENT_DLL_ http_response* http_download(const char*url,const char* saveFilePath,DownLoadReceiptDataLengthCallback callback)
{
	return moon_http_download(url,saveFilePath,callback);
}

/**
 * http response destory
 */
void http_response_free(http_response* hresp)
{
	moon_http_response_free(hresp);
}