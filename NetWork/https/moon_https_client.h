/********************************************************************************
 *  模块说明：
 *		该模块采用curl、openssl实现的https协议的POST/GET请求
 *
 ********************************************************************************/
#ifndef _MOON_HTTPS_CLIENT_H
#define _MOON_HTTPS_CLIENT_H

#include "../api/define/moon_http.h"

/**
 * 函数说明：
 *	http GET请求
 * 参数说明：
 *	url：请求的url
 * 返回值：
 *	请求成功返回http_response，失败返回NULL
 */
http_response* moon_http_get_ex(const char* url);

/**
 * 函数说明：
 *	http POST请求
 * 参数说明：
 *	url：请求的url
 *	params：请求的参数,如：ppara1=val1&para2=val2&…
 * 返回值：
 *	请求成功返回http_response，失败返回NULL
 */
http_response* moon_http_post_ex(const char* url, const char* params);

/**
 * 函数说明：
 *	https GET请求，无证书
 * 参数说明：
 *	url：请求的url
 * 返回值：
 *	请求成功返回http_response，失败返回NULL
 */
http_response* moon_https_get(const char* url);

/**
 * 函数说明：
 *	https GET请求，无证书
 * 参数说明：
 *	url：请求的url
 *	headerName：添加的请求头KEY数组
 *	headerValue：添加的请求头VALUE数组
 *	headerCount：请求头的数量
 */
http_response* moon_https_get_ex(const char* url,const char* headerName[],const char* headerValue[],int headerCount);

/**
 * 函数说明：
 *	https POST请求，无证书
 * 参数说明：
 *	url：请求的url
 *	params：请求的参数,如：ppara1=val1&para2=val2&…
 * 返回值：
 *	请求成功返回http_response，失败返回NULL
 */
http_response* moon_https_post(const char* url, const char* params);

/**
 * 函数说明：
 *	https POST请求，无证书
 * 参数说明：
 *	url：请求的url
 *	params：请求的参数,如：ppara1=val1&para2=val2&…
 *	headerName：添加的请求头KEY数组
 *	headerValue：添加的请求头VALUE数组
 *	headerCount：请求头的数量
 */
http_response* moon_https_post_ex(const char* url,const char* params,const char* headerName[],const char* headerValue[],int headerCount);


/**
 * 函数说明：
 *   http下载文件
 * 参数说明：
 *   url：文件地址
 *   callback：接收数据长度回调，可以用于进度提示
 *   saveFilePath：下载文件保存路径
 * 返回值：
 *   成功返回http_response，失败返回NULL
 */
http_response* moon_http_download(const char*url,const char* saveFilePath,DownLoadReceiptDataLengthCallback callback);
#endif