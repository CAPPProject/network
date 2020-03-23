#ifndef _MOON_HTTP_H
#define _MOON_HTTP_H

/**
 *  说明：定义下载接收数据长度通知回调
 *  参数：
 *		hasReciptLength：已接收的数据长度
 */
typedef void(*DownLoadReceiptDataLengthCallback)(unsigned long hasReciptLength);

/**
 * url struct
 */
typedef struct _parsed_url 
{
	char *uri;					/* mandatory */
    char *scheme;               /* mandatory */
    char *host;                 /* mandatory */
	char *ip; 					/* mandatory */
    char *port;                 /* optional */
    char *path;                 /* optional */
    char *query;                /* optional */
    char *fragment;             /* optional */
    char *username;             /* optional */
    char *password;             /* optional */
} parsed_url,*p_parsed_url;

/*
 *	HTTP html response struct
 */
typedef struct _http_response
{
	parsed_url *request_uri;
	char *body;
	char *status_code;
	int status_code_int;
	long body_length;
	char *status_text;
	char *request_headers;
	char *response_headers;
}http_response,*p_http_response;


/**
 * create http_response,free it when not use
 */
http_response* moon_http_response_create();

/**
 *	Free memory of http_response
 */
void moon_http_response_free(http_response *hresp);

/**
 * 函数功能：
 *	解析http响应头
 * 参数：
 *	response_head：响应头字符串
 *	hresp：http响应结构体
 */
void moon_http_response_head_parse(const char* response_head,http_response *hresp);

#endif
