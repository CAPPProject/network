#include "moon_http.h"
#include "../common/moon_url_parse.h"
#include "../common/moon_string.h"

/**
 * create http_response,free it when not use
 */
http_response* moon_http_response_create()
{
	http_response* hresp = (http_response*)malloc(sizeof(http_response));
	hresp->request_uri = (parsed_url*)malloc(sizeof(parsed_url));
	hresp->request_uri->fragment = NULL;
	hresp->request_uri->host = NULL;
	hresp->request_uri->ip = NULL;
	hresp->request_uri->password = NULL;
	hresp->request_uri->path = NULL;
	hresp->request_uri->port = NULL;
	hresp->request_uri->query = NULL;
	hresp->request_uri->scheme = NULL;
	hresp->request_uri->uri = NULL;
	hresp->request_uri->username = NULL;
	hresp->body = NULL;
	hresp->body_length = 0;
	hresp->request_headers = NULL;
	hresp->response_headers = NULL;
	hresp->status_code = NULL;
	hresp->status_code_int = 0;
	hresp->status_text = NULL;
	return hresp;
}

/**
 *	Free memory of http_response
 */
void moon_http_response_free(http_response *hresp)
{
	if(hresp != NULL)
	{
		if(hresp->request_uri != NULL) moon_parsed_url_free(hresp->request_uri);
		if(hresp->body != NULL) free(hresp->body);
		if(hresp->status_code != NULL) free(hresp->status_code);
		if(hresp->status_text != NULL) free(hresp->status_text);
		if(hresp->request_headers != NULL) free(hresp->request_headers);
		if(hresp->response_headers != NULL) free(hresp->response_headers);
		free(hresp);
	}
}

/**
 * 函数功能：
 *	解析http响应头
 * 参数：
 *	response_head：响应头字符串
 *	hresp：http响应结构体
 */
void moon_http_response_head_parse(const char* response_head,http_response *hresp)
{
	char* status_line = NULL;
	char* status_code = NULL;
	char* status_text = NULL;
	char* head = NULL;
	int hdsize = moon_string_length(response_head);
	hresp->response_headers = (char*)malloc(hdsize + 1);
	head = (char*)malloc(hdsize + 1);
	memset(head,0,hdsize + 1);
	memset(hresp->response_headers,0,hdsize + 1);
	memcpy(head,response_head,hdsize);
	memcpy(hresp->response_headers,response_head,hdsize);
	status_line = moon_get_until(head, "\r\n");
	status_line = moon_str_replace("HTTP/1.1 ", "", status_line);
	status_code = moon_str_ndup(status_line, 4);
	status_code = moon_str_replace(" ", "", status_code);
	status_text = moon_str_replace(status_code, "", status_line);
	status_text = moon_str_replace(" ", "", status_text);
	hresp->status_code = status_code;
	hresp->status_code_int = atoi(status_code);
	hresp->status_text = status_text;
	free(status_line);
	free(head);
}