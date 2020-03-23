#include "moon_https_client.h"
#include "../common/moon_url_parse.h"
#include "curl/curl.h"
#include "../Common/moon_string.h"
#include <string>

using namespace std;

//�������ݽ��յĽṹ��
typedef struct _DownloadDataReceipt{
	DownLoadReceiptDataLengthCallback callback;//�ص�����
	string httpResponseHead;//�ļ�ͷ
	string saveFilePath;//������ļ�·��
	FILE * fp;//�ļ�ָ��
	unsigned long hasReciptLength;//�����ص��ļ�����
}DownloadDataReceipt;


/**
 * ptr��ָ��洢���ݵ�ָ�룬 
 * size��ÿ����Ĵ�С�� 
 * nmemb��ָ�����Ŀ�� 
 * stream���û������� 
 * ���Ը���������Щ��������Ϣ����֪����ptr�е����ݵ��ܳ�����size*nmemb 
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

// ����http header�ص�����    
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

//��������ͷ�ص�
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

//�������ص��ļ�����
static string parse_download_file_name(string responseHead)
{
	string fileName = "";
	string headers = responseHead;
	//һ��һ�ж�ȡͷ
	string line = "";
	for (int i = 0;i < responseHead.size();i++)
	{
		if (headers[i] == '\n')
		{
			//��ʾһ�ж�ȡ���
			if (line.find("Content-Disposition:") != string::npos)
			{
				line = line.replace(line.find("Content-Disposition:"),strlen("Content-Disposition:"),"");
				//ȥ������
				if(moon_str_contains(line.c_str(),"\r"))
				{
					line = string_replace(line.c_str(),"\r","");
				}
				if(moon_str_contains(line.c_str(),"\n"))
				{
					line = string_replace(line.c_str(),"\n","");
				}
				//ȥ���ո�
				line = moon_string_trim(line);
				break;
			}
			//�����
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

//����������ص�
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
	//������д���ļ�
	//�����ļ�����
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
		// ���ļ�ʧ��
		if (eResult != 0)
			exit(-1);
	}
	fwrite(data.c_str(),sizeof(char),data.length(),dowloadData->fp);
	//�����صĽ��ȷ�����Ϣ����
	if (dowloadData->callback != NULL)
	{
		dowloadData->callback(dowloadData->hasReciptLength);
	}

	return nmemb;
}

/**
 * ����˵����
 *	http GET����
 * ����˵����
 *	url�������url
 * ����ֵ��
 *	����ɹ�����http_response��ʧ�ܷ���NULL
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
	//ץȡ���ݺ󣬻ص����� 
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, call_write_func);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&szbuffer);
	//ץȡͷ��Ϣ���ص�����  
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, &szheader_buffer);
	/**
	* ������̶߳�ʹ�ó�ʱ�����ʱ��ͬʱ���߳�����sleep����wait�Ȳ�����
	* ������������ѡ�libcurl���ᷢ�źŴ�����wait�Ӷ����³����˳���
	*/
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 8);//���ӳ�ʱ�������ֵ�������̫�̿��ܵ����������󲻵��ͶϿ���
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);//��������ʱ��ʱ���ã����10��������δ�����ֱ꣬���˳�
	res = curl_easy_perform(curl);
	if (res)//0��ʾ�ɹ�����0ʧ��
	{
		moon_http_response_free(pHttpResponse);
		pHttpResponse = NULL;
		curl_easy_cleanup(curl);
		return NULL;
	}
	curl_easy_cleanup(curl);
	//������Ӧͷ
	moon_http_response_head_parse(szheader_buffer.c_str(),pHttpResponse);
	//��Ӧ��
	pHttpResponse->body_length = szbuffer.length();
	pHttpResponse->body = (char*)malloc(szbuffer.length());
	memset(pHttpResponse->body,0,szbuffer.length());
	memcpy(pHttpResponse->body,szbuffer.c_str(),szbuffer.length());
	return pHttpResponse;
}

/**
 * ����˵����
 *	http POST����
 * ����˵����
 *	url�������url
 *	params������Ĳ���,�磺ppara1=val1&para2=val2&��
 * ����ֵ��
 *	����ɹ�����http_response��ʧ�ܷ���NULL
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
	//ץȡ���ݺ󣬻ص����� 
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, call_write_func);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&szbuffer);
	//ץȡͷ��Ϣ���ص�����  
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, &szheader_buffer);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 8);//���ӳ�ʱ�������ֵ�������̫�̿��ܵ����������󲻵��ͶϿ���
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);//��������ʱ��ʱ���ã����10��������δ�����ֱ꣬���˳�
	res = curl_easy_perform(curl);
	if (res)//0��ʾ�ɹ�����0ʧ��
	{
		moon_http_response_free(pHttpResponse);
		pHttpResponse = NULL;
		curl_easy_cleanup(curl);
		return NULL;
	}
	curl_easy_cleanup(curl);
	//������Ӧͷ
	moon_http_response_head_parse(szheader_buffer.c_str(),pHttpResponse);
	//��Ӧ��
	pHttpResponse->body_length = szbuffer.length();
	pHttpResponse->body = (char*)malloc(szbuffer.length());
	memset(pHttpResponse->body,0,szbuffer.length());
	memcpy(pHttpResponse->body,szbuffer.c_str(),szbuffer.length());
	return pHttpResponse;
}

/**
 * ����˵����
 *	https GET������֤��
 * ����˵����
 *	url�������url
 * ����ֵ��
 *	����ɹ�����http_response��ʧ�ܷ���NULL
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
	//ץȡ���ݺ󣬻ص����� 
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, call_write_func);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&szbuffer);
	//ץȡͷ��Ϣ���ص�����  
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, &szheader_buffer);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 8);//���ӳ�ʱ�������ֵ�������̫�̿��ܵ����������󲻵��ͶϿ���
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);//��������ʱ��ʱ���ã����10��������δ�����ֱ꣬���˳�
	res = curl_easy_perform(curl);
	if (res)//0��ʾ�ɹ�����0ʧ��
	{
		moon_http_response_free(pHttpResponse);
		pHttpResponse = NULL;
		curl_easy_cleanup(curl);
		return NULL;
	}
	curl_easy_cleanup(curl);
	//������Ӧͷ
	moon_http_response_head_parse(szheader_buffer.c_str(),pHttpResponse);
	//��Ӧ��
	pHttpResponse->body_length = szbuffer.length();
	pHttpResponse->body = (char*)malloc(szbuffer.length());
	memset(pHttpResponse->body,0,szbuffer.length());
	memcpy(pHttpResponse->body,szbuffer.c_str(),szbuffer.length());
	return pHttpResponse;
}

/**
 * ����˵����
 *	https GET������֤��
 * ����˵����
 *	url�������url
 *	headerName����ӵ�����ͷKEY����
 *	headerValue����ӵ�����ͷVALUE����
 *	headerCount������ͷ������
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
	//��������ͷ
	
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
	//ץȡ���ݺ󣬻ص�����
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, call_write_func);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&szbuffer);
	//ץȡͷ��Ϣ���ص�����  
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, &szheader_buffer);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, false);//��ֹ�ض���
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 8);//���ӳ�ʱ�������ֵ�������̫�̿��ܵ����������󲻵��ͶϿ���
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);//��������ʱ��ʱ���ã����10��������δ�����ֱ꣬���˳�
	res = curl_easy_perform(curl);
	if (res)//0��ʾ�ɹ�����0ʧ��
	{
		moon_http_response_free(pHttpResponse);
		pHttpResponse = NULL;
		curl_easy_cleanup(curl);
		return NULL;
	}
	curl_easy_cleanup(curl);
	//������Ӧͷ
	moon_http_response_head_parse(szheader_buffer.c_str(),pHttpResponse);
	//��Ӧ��
	pHttpResponse->body_length = szbuffer.length();
	pHttpResponse->body = (char*)malloc(szbuffer.length());
	memset(pHttpResponse->body,0,szbuffer.length());
	memcpy(pHttpResponse->body,szbuffer.c_str(),szbuffer.length());
	curl_slist_free_all(header_list);
	return pHttpResponse;
}

/**
 * ����˵����
 *	https POST������֤��
 * ����˵����
 *	url�������url
 *	params������Ĳ���,�磺ppara1=val1&para2=val2&��
 * ����ֵ��
 *	����ɹ�����http_response��ʧ�ܷ���NULL
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
	//ץȡ���ݺ󣬻ص����� 
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, call_write_func);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&szbuffer);
	//ץȡͷ��Ϣ���ص�����  
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, &szheader_buffer);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 8);//���ӳ�ʱ�������ֵ�������̫�̿��ܵ����������󲻵��ͶϿ���
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);//��������ʱ��ʱ���ã����10��������δ�����ֱ꣬���˳�
	res = curl_easy_perform(curl);
	if (res)//0��ʾ�ɹ�����0ʧ��
	{
		moon_http_response_free(pHttpResponse);
		pHttpResponse = NULL;
		curl_easy_cleanup(curl);
		return NULL;
	}
	curl_easy_cleanup(curl);
	//������Ӧͷ
	moon_http_response_head_parse(szheader_buffer.c_str(),pHttpResponse);
	//��Ӧ��
	pHttpResponse->body_length = szbuffer.length();
	pHttpResponse->body = (char*)malloc(szbuffer.length());
	memset(pHttpResponse->body,0,szbuffer.length());
	memcpy(pHttpResponse->body,szbuffer.c_str(),szbuffer.length());
	return pHttpResponse;
}


/**
 * ����˵����
 *	https POST������֤��
 * ����˵����
 *	url�������url
 *	params������Ĳ���,�磺ppara1=val1&para2=val2&��
 *	headerName����ӵ�����ͷKEY����
 *	headerValue����ӵ�����ͷVALUE����
 *	headerCount������ͷ������
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
	//��������ͷ

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
	//ץȡ���ݺ󣬻ص�����
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, call_write_func);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&szbuffer);
	//ץȡͷ��Ϣ���ص�����  
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, &szheader_buffer);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, false);//��ֹ�ض���
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 8);//���ӳ�ʱ�������ֵ�������̫�̿��ܵ����������󲻵��ͶϿ���
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);//��������ʱ��ʱ���ã����10��������δ�����ֱ꣬���˳�
	res = curl_easy_perform(curl);
	if (res)//0��ʾ�ɹ�����0ʧ��
	{
		moon_http_response_free(pHttpResponse);
		pHttpResponse = NULL;
		curl_easy_cleanup(curl);
		return NULL;
	}
	curl_easy_cleanup(curl);
	//������Ӧͷ
	moon_http_response_head_parse(szheader_buffer.c_str(),pHttpResponse);
	//��Ӧ��
	pHttpResponse->body_length = szbuffer.length();
	pHttpResponse->body = (char*)malloc(szbuffer.length());
	memset(pHttpResponse->body,0,szbuffer.length());
	memcpy(pHttpResponse->body,szbuffer.c_str(),szbuffer.length());
	curl_slist_free_all(header_list);
	return pHttpResponse;
}

/**
 * ����˵����
 *   http�����ļ�
 * ����˵����
 *   url���ļ���ַ
 *   callback���������ݳ��Ȼص����������ڽ�����ʾ
 *   saveFilePath�������ļ�����·��
 * ����ֵ��
 *   �ɹ�����http_response��ʧ�ܷ���NULL
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
	//ץȡ���ݺ󣬻ص����� 
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, dowload_write_data);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&downloadDataReceipt);
	//ץȡͷ��Ϣ���ص�����  
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, dowload_write_head);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, (void *)&downloadDataReceipt);
	/**
	* ������̶߳�ʹ�ó�ʱ�����ʱ��ͬʱ���߳�����sleep����wait�Ȳ�����
	* ������������ѡ�libcurl���ᷢ�źŴ�����wait�Ӷ����³����˳���
	*/
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 8);//���ӳ�ʱ�������ֵ�������̫�̿��ܵ����������󲻵��ͶϿ���
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 100);//��������ʱ��ʱ���ã����10��������δ�����ֱ꣬���˳�
	res = curl_easy_perform(curl);
	//�ر��ļ�
	fclose(downloadDataReceipt.fp);
	if (res)//0��ʾ�ɹ�����0ʧ��
	{
		delete pHttpResponse;
		pHttpResponse = NULL;
		curl_easy_cleanup(curl);
		return NULL;
	}
	curl_easy_cleanup(curl);
	//������Ӧͷ
	moon_http_response_head_parse(downloadDataReceipt.httpResponseHead.c_str(),pHttpResponse);

	return pHttpResponse;
}