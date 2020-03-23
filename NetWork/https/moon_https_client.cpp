#include "moon_https_client.h"
#include "../common/moon_url_parse.h"
#include "curl/curl.h"
#include "../Common/moon_string.h"
#include <string>

using namespace std;

//下载数据接收的结构体
typedef struct _DownloadDataReceipt{
	DownLoadReceiptDataLengthCallback callback;//回调函数
	string httpResponseHead;//文件头
	string saveFilePath;//保存的文件路径
	FILE * fp;//文件指针
	unsigned long hasReciptLength;//已下载的文件长度
}DownloadDataReceipt;


/**
 * ptr是指向存储数据的指针， 
 * size是每个块的大小， 
 * nmemb是指块的数目， 
 * stream是用户参数。 
 * 所以根据以上这些参数的信息可以知道，ptr中的数据的总长度是size*nmemb 
*/  
static size_t call_write_func(void* buffer, size_t size, size_t nmemb, void* lpVoid)
{
	std::string* str = dynamic_cast<std::string*>((std::string *)lpVoid);
	if( NULL == str || NULL == buffer )
	{
		return -1;
	}

    char* pData = (char*)buffer;
    str->append(pData, size * nmemb);
	return nmemb;
}

// 返回http header回调函数    
static size_t header_callback(const char  *buffer, size_t size, size_t nmemb, void* lpVoid)    
{    
	std::string* str = dynamic_cast<std::string*>((std::string *)lpVoid);
	if( NULL == str || NULL == buffer )
	{
		return -1;
	}

	char* pData = (char*)buffer;
	str->append(pData, size * nmemb);
	return nmemb;
}

//下载数据头回调
static size_t dowload_write_head(void* buffer, size_t size, size_t nmemb, void* lpVoid)
{
	DownloadDataReceipt* dowloadData = dynamic_cast<DownloadDataReceipt*>((DownloadDataReceipt *)lpVoid);
	if( NULL == dowloadData || NULL == buffer )
	{
		return -1;
	}

	char* pData = (char*)buffer;
	dowloadData->httpResponseHead.append(pData, size * nmemb);
	return nmemb;
}

//解析下载的文件名称
static string parse_download_file_name(string responseHead)
{
	string fileName = "";
	string headers = responseHead;
	//一行一行读取头
	string line = "";
	for (int i = 0;i < responseHead.size();i++)
	{
		if (headers[i] == '\n')
		{
			//表示一行读取完毕
			if (line.find("Content-Disposition:") != string::npos)
			{
				line = line.replace(line.find("Content-Disposition:"),strlen("Content-Disposition:"),"");
				//去掉换行
				if(moon_str_contains(line.c_str(),"\r"))
				{
					line = string_replace(line.c_str(),"\r","");
				}
				if(moon_str_contains(line.c_str(),"\n"))
				{
					line = string_replace(line.c_str(),"\n","");
				}
				//去掉空格
				line = moon_string_trim(line);
				break;
			}
			//清空行
			line = "";
		}
		line += headers[i];
	}
	if (line.length() > 0)
	{
		int pos = line.find("filename=");
		if(pos != string::npos)
		{
			fileName = line.substr(pos + strlen("filename="));
		}
	}
	fileName = url_decode(fileName);
	wstring wstr = utf8_to_unicode(fileName);
	fileName = unicode_to_ascii(wstr);
	return fileName;
}

//下载数据体回调
static size_t dowload_write_data(void* buffer, size_t size, size_t nmemb, void* lpVoid)
{
	DownloadDataReceipt* dowloadData = dynamic_cast<DownloadDataReceipt*>((DownloadDataReceipt *)lpVoid);
	if( NULL == dowloadData || NULL == buffer )
	{
		return -1;
	}

	char* pData = (char*)buffer;
	string data;
	data.append(pData, size * nmemb);
	dowloadData->hasReciptLength += data.length();
	//将数据写入文件
	//解析文件名称
	string fileName = parse_download_file_name(dowloadData->httpResponseHead);
	string filePathName = dowloadData->saveFilePath;
	if(filePathName[filePathName.length() - 1] == '\\' || filePathName[filePathName.length() - 1] == '/')
	{
		filePathName += fileName;
	}
	else
	{
		filePathName += "\\";
		filePathName += fileName;
	}
	if(dowloadData->fp == NULL)
	{
		errno_t eResult = fopen_s(&dowloadData->fp,filePathName.c_str(),"wb");
		// 打开文件失败
		if (eResult != 0)
			exit(-1);
	}
	fwrite(data.c_str(),sizeof(char),data.length(),dowloadData->fp);
	//将下载的进度放入消息队列
	if (dowloadData->callback != NULL)
	{
		dowloadData->callback(dowloadData->hasReciptLength);
	}

	return nmemb;
}

/**
 * 函数说明：
 *	http GET请求
 * 参数说明：
 *	url：请求的url
 * 返回值：
 *	请求成功返回http_response，失败返回NULL
 */
http_response* moon_http_get_ex(const char* url)
{
	http_response* pHttpResponse = NULL;
	string szbuffer;
	string szheader_buffer;
	CURLcode res;
	long body_length = 0;
	CURL* curl = curl_easy_init();
	if(NULL == curl)
	{
		return pHttpResponse;
	}
	pHttpResponse = moon_http_response_create();
	pHttpResponse->request_uri = moon_parse_url(url);
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	//抓取内容后，回调函数 
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, call_write_func);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&szbuffer);
	//抓取头信息，回调函数  
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, &szheader_buffer);
	/**
	* 当多个线程都使用超时处理的时候，同时主线程中有sleep或是wait等操作。
	* 如果不设置这个选项，libcurl将会发信号打断这个wait从而导致程序退出。
	*/
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 8);//连接超时，这个数值如果设置太短可能导致数据请求不到就断开了
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);//接收数据时超时设置，如果10秒内数据未接收完，直接退出
	res = curl_easy_perform(curl);
	if (res)//0表示成功，非0失败
	{
		moon_http_response_free(pHttpResponse);
		pHttpResponse = NULL;
		curl_easy_cleanup(curl);
		return NULL;
	}
	curl_easy_cleanup(curl);
	//解析响应头
	moon_http_response_head_parse(szheader_buffer.c_str(),pHttpResponse);
	//响应体
	pHttpResponse->body_length = szbuffer.length();
	pHttpResponse->body = (char*)malloc(szbuffer.length());
	memset(pHttpResponse->body,0,szbuffer.length());
	memcpy(pHttpResponse->body,szbuffer.c_str(),szbuffer.length());
	return pHttpResponse;
}

/**
 * 函数说明：
 *	http POST请求
 * 参数说明：
 *	url：请求的url
 *	params：请求的参数,如：ppara1=val1&para2=val2&…
 * 返回值：
 *	请求成功返回http_response，失败返回NULL
 */
http_response* moon_http_post_ex(const char* url, const char* params)
{
	http_response* pHttpResponse = NULL;
	string szbuffer;
	string szheader_buffer;
	CURLcode res;
	long body_length = 0;
	CURL* curl = curl_easy_init();
	if(NULL == curl)
	{
		return pHttpResponse;
	}
	pHttpResponse = moon_http_response_create();
	pHttpResponse->request_uri = moon_parse_url(url);
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, params);
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	//抓取内容后，回调函数 
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, call_write_func);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&szbuffer);
	//抓取头信息，回调函数  
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, &szheader_buffer);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 8);//连接超时，这个数值如果设置太短可能导致数据请求不到就断开了
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);//接收数据时超时设置，如果10秒内数据未接收完，直接退出
	res = curl_easy_perform(curl);
	if (res)//0表示成功，非0失败
	{
		moon_http_response_free(pHttpResponse);
		pHttpResponse = NULL;
		curl_easy_cleanup(curl);
		return NULL;
	}
	curl_easy_cleanup(curl);
	//解析响应头
	moon_http_response_head_parse(szheader_buffer.c_str(),pHttpResponse);
	//响应体
	pHttpResponse->body_length = szbuffer.length();
	pHttpResponse->body = (char*)malloc(szbuffer.length());
	memset(pHttpResponse->body,0,szbuffer.length());
	memcpy(pHttpResponse->body,szbuffer.c_str(),szbuffer.length());
	return pHttpResponse;
}

/**
 * 函数说明：
 *	https GET请求，无证书
 * 参数说明：
 *	url：请求的url
 * 返回值：
 *	请求成功返回http_response，失败返回NULL
 */
http_response* moon_https_get(const char* url)
{
	http_response* pHttpResponse = NULL;
	string szbuffer;
	string szheader_buffer;
	CURLcode res;
	long body_length = 0;
	CURL* curl = curl_easy_init();
	if(NULL == curl)
	{
		return pHttpResponse;
	}
	pHttpResponse = moon_http_response_create();
	pHttpResponse->request_uri = moon_parse_url(url);
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	//抓取内容后，回调函数 
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, call_write_func);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&szbuffer);
	//抓取头信息，回调函数  
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, &szheader_buffer);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 8);//连接超时，这个数值如果设置太短可能导致数据请求不到就断开了
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);//接收数据时超时设置，如果10秒内数据未接收完，直接退出
	res = curl_easy_perform(curl);
	if (res)//0表示成功，非0失败
	{
		moon_http_response_free(pHttpResponse);
		pHttpResponse = NULL;
		curl_easy_cleanup(curl);
		return NULL;
	}
	curl_easy_cleanup(curl);
	//解析响应头
	moon_http_response_head_parse(szheader_buffer.c_str(),pHttpResponse);
	//响应体
	pHttpResponse->body_length = szbuffer.length();
	pHttpResponse->body = (char*)malloc(szbuffer.length());
	memset(pHttpResponse->body,0,szbuffer.length());
	memcpy(pHttpResponse->body,szbuffer.c_str(),szbuffer.length());
	return pHttpResponse;
}

/**
 * 函数说明：
 *	https GET请求，无证书
 * 参数说明：
 *	url：请求的url
 *	headerName：添加的请求头KEY数组
 *	headerValue：添加的请求头VALUE数组
 *	headerCount：请求头的数量
 */
http_response* moon_https_get_ex(const char* url,const char* headerName[],const char* headerValue[],int headerCount)
{
	http_response* pHttpResponse = NULL;
	string szbuffer;
	string szheader_buffer;
	CURLcode res;
	long body_length = 0;
	struct curl_slist *header_list = NULL;
	int i = 0;
	char* header = NULL;
	long headerLength = 0;
	long headerValueLength = 0;
	CURL* curl = curl_easy_init();
	if(NULL == curl)
	{
		return pHttpResponse;
	}
	pHttpResponse = moon_http_response_create();
	pHttpResponse->request_uri = moon_parse_url(url);
	//设置请求头
	
	if (headerCount > 0)
	{
		for (i = 0; i < headerCount; ++i) {
			headerLength = moon_string_length(headerName[i]);
			header = (char*)malloc(headerLength + 3);
			memset(header,0,headerLength + 3);
			memcpy(header,headerName[i],headerLength);
			header[headerLength] = ':';
			header[headerLength + 1] = ' ';
			headerValueLength = moon_string_length(headerValue[i]);
			header = (char*)realloc(header,moon_string_length(header) + headerValueLength + 1);
			memcpy(&header[headerLength + 2],headerValue[i],headerValueLength);
			header[headerLength + headerValueLength + 2] = '\0';
			header_list = curl_slist_append(header_list,header);
			free(header);
		}
	}
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	//抓取内容后，回调函数
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, call_write_func);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&szbuffer);
	//抓取头信息，回调函数  
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, &szheader_buffer);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, false);//禁止重定向
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 8);//连接超时，这个数值如果设置太短可能导致数据请求不到就断开了
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);//接收数据时超时设置，如果10秒内数据未接收完，直接退出
	res = curl_easy_perform(curl);
	if (res)//0表示成功，非0失败
	{
		moon_http_response_free(pHttpResponse);
		pHttpResponse = NULL;
		curl_easy_cleanup(curl);
		return NULL;
	}
	curl_easy_cleanup(curl);
	//解析响应头
	moon_http_response_head_parse(szheader_buffer.c_str(),pHttpResponse);
	//响应体
	pHttpResponse->body_length = szbuffer.length();
	pHttpResponse->body = (char*)malloc(szbuffer.length());
	memset(pHttpResponse->body,0,szbuffer.length());
	memcpy(pHttpResponse->body,szbuffer.c_str(),szbuffer.length());
	curl_slist_free_all(header_list);
	return pHttpResponse;
}

/**
 * 函数说明：
 *	https POST请求，无证书
 * 参数说明：
 *	url：请求的url
 *	params：请求的参数,如：ppara1=val1&para2=val2&…
 * 返回值：
 *	请求成功返回http_response，失败返回NULL
 */
http_response* moon_https_post(const char* url, const char* params)
{
	http_response* pHttpResponse = NULL;
	string szbuffer;
	string szheader_buffer;
	CURLcode res;
	long body_length = 0;
	CURL* curl = curl_easy_init();
	if(NULL == curl)
	{
		return pHttpResponse;
	}
	pHttpResponse = moon_http_response_create();
	pHttpResponse->request_uri = moon_parse_url(url);
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, params);
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	//抓取内容后，回调函数 
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, call_write_func);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&szbuffer);
	//抓取头信息，回调函数  
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, &szheader_buffer);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 8);//连接超时，这个数值如果设置太短可能导致数据请求不到就断开了
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);//接收数据时超时设置，如果10秒内数据未接收完，直接退出
	res = curl_easy_perform(curl);
	if (res)//0表示成功，非0失败
	{
		moon_http_response_free(pHttpResponse);
		pHttpResponse = NULL;
		curl_easy_cleanup(curl);
		return NULL;
	}
	curl_easy_cleanup(curl);
	//解析响应头
	moon_http_response_head_parse(szheader_buffer.c_str(),pHttpResponse);
	//响应体
	pHttpResponse->body_length = szbuffer.length();
	pHttpResponse->body = (char*)malloc(szbuffer.length());
	memset(pHttpResponse->body,0,szbuffer.length());
	memcpy(pHttpResponse->body,szbuffer.c_str(),szbuffer.length());
	return pHttpResponse;
}


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
http_response* moon_https_post_ex(const char* url,const char* params,const char* headerName[],const char* headerValue[],int headerCount)
{
	http_response* pHttpResponse = NULL;
	string szbuffer;
	string szheader_buffer;
	CURLcode res;
	long body_length = 0;
	struct curl_slist *header_list = NULL;
	int i = 0;
	char* header = NULL;
	long headerLength = 0;
	long headerValueLength = 0;
	CURL* curl = curl_easy_init();
	if(NULL == curl)
	{
		return pHttpResponse;
	}
	pHttpResponse = moon_http_response_create();
	pHttpResponse->request_uri = moon_parse_url(url);
	//设置请求头

	if (headerCount > 0)
	{
		for (i = 0; i < headerCount; ++i) {
			headerLength = moon_string_length(headerName[i]);
			header = (char*)malloc(headerLength + 3);
			memset(header,0,headerLength + 3);
			memcpy(header,headerName[i],headerLength);
			header[headerLength] = ':';
			header[headerLength + 1] = ' ';
			headerValueLength = moon_string_length(headerValue[i]);
			header = (char*)realloc(header,moon_string_length(header) + headerValueLength + 1);
			memcpy(&header[headerLength + 2],headerValue[i],headerValueLength);
			header[headerLength + headerValueLength + 2] = '\0';
			header_list = curl_slist_append(header_list,header);
			free(header);
		}
	}
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, params);
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	//抓取内容后，回调函数
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, call_write_func);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&szbuffer);
	//抓取头信息，回调函数  
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, &szheader_buffer);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, false);//禁止重定向
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 8);//连接超时，这个数值如果设置太短可能导致数据请求不到就断开了
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);//接收数据时超时设置，如果10秒内数据未接收完，直接退出
	res = curl_easy_perform(curl);
	if (res)//0表示成功，非0失败
	{
		moon_http_response_free(pHttpResponse);
		pHttpResponse = NULL;
		curl_easy_cleanup(curl);
		return NULL;
	}
	curl_easy_cleanup(curl);
	//解析响应头
	moon_http_response_head_parse(szheader_buffer.c_str(),pHttpResponse);
	//响应体
	pHttpResponse->body_length = szbuffer.length();
	pHttpResponse->body = (char*)malloc(szbuffer.length());
	memset(pHttpResponse->body,0,szbuffer.length());
	memcpy(pHttpResponse->body,szbuffer.c_str(),szbuffer.length());
	curl_slist_free_all(header_list);
	return pHttpResponse;
}

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
http_response* moon_http_download(const char*url,const char* saveFilePath,DownLoadReceiptDataLengthCallback callback)
{
	DownloadDataReceipt downloadDataReceipt;
	downloadDataReceipt.hasReciptLength = 0;
	http_response *pHttpResponse = NULL;
	downloadDataReceipt.fp = NULL;
	CURLcode res;
	long body_length = 0;
	CURL* curl = curl_easy_init();
	if(NULL == curl)
	{
		return pHttpResponse;
	}
	pHttpResponse = moon_http_response_create();
	downloadDataReceipt.callback = callback;
	downloadDataReceipt.saveFilePath = saveFilePath;
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	//抓取内容后，回调函数 
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, dowload_write_data);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&downloadDataReceipt);
	//抓取头信息，回调函数  
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, dowload_write_head);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, (void *)&downloadDataReceipt);
	/**
	* 当多个线程都使用超时处理的时候，同时主线程中有sleep或是wait等操作。
	* 如果不设置这个选项，libcurl将会发信号打断这个wait从而导致程序退出。
	*/
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 8);//连接超时，这个数值如果设置太短可能导致数据请求不到就断开了
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 100);//接收数据时超时设置，如果10秒内数据未接收完，直接退出
	res = curl_easy_perform(curl);
	//关闭文件
	fclose(downloadDataReceipt.fp);
	if (res)//0表示成功，非0失败
	{
		delete pHttpResponse;
		pHttpResponse = NULL;
		curl_easy_cleanup(curl);
		return NULL;
	}
	curl_easy_cleanup(curl);
	//解析响应头
	moon_http_response_head_parse(downloadDataReceipt.httpResponseHead.c_str(),pHttpResponse);

	return pHttpResponse;
}