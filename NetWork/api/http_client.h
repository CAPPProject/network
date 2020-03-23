#ifndef _HTTP_CLIENT_H
#define _HTTP_CLIENT_H

#include "define/moon_http.h"
//定义函数导出
#ifndef HTTP_CLIENT_DLL_
#define HTTP_CLIENT_DLL_ extern "C" _declspec (dllimport)
#else
#define HTTP_CLIENT_DLL_ extern "C" _declspec (dllexport)
#endif

/**
 * http get request
 */
HTTP_CLIENT_DLL_ http_response* http_get(const char *url);

/**
 * http post request
 */
HTTP_CLIENT_DLL_ http_response* http_post(const char *url,const char *params);

/**
 * https get request
 */
HTTP_CLIENT_DLL_ http_response* https_get(const char *url);

/**
 * https get request
 */
HTTP_CLIENT_DLL_ http_response* https_get_ex(const char* url,const char* headerName[],const char* headerValue[],int headerCount);

/**
 * https post request
 */
HTTP_CLIENT_DLL_ http_response* https_post(const char *url,const char* params);

/**
 * https get request
 */
HTTP_CLIENT_DLL_ http_response* https_post_ex(const char* url,const char* params,const char* headerName[],const char* headerValue[],int headerCount);

/**
 * http download
 */
HTTP_CLIENT_DLL_ http_response* http_download(const char*url,const char* saveFilePath,DownLoadReceiptDataLengthCallback callback);

/**
 * http response destory
 */
HTTP_CLIENT_DLL_ void http_response_free(http_response* hresp);



#endif
