/********************************************************************************
 *  ģ��˵����
 *		��ģ�����curl��opensslʵ�ֵ�httpsЭ���POST/GET����
 *
 ********************************************************************************/
#ifndef _MOON_HTTPS_CLIENT_H
#define _MOON_HTTPS_CLIENT_H

#include "../api/define/moon_http.h"

/**
 * ����˵����
 *	http GET����
 * ����˵����
 *	url�������url
 * ����ֵ��
 *	����ɹ�����http_response��ʧ�ܷ���NULL
 */
http_response* moon_http_get_ex(const char* url);

/**
 * ����˵����
 *	http POST����
 * ����˵����
 *	url�������url
 *	params������Ĳ���,�磺ppara1=val1&para2=val2&��
 * ����ֵ��
 *	����ɹ�����http_response��ʧ�ܷ���NULL
 */
http_response* moon_http_post_ex(const char* url, const char* params);

/**
 * ����˵����
 *	https GET������֤��
 * ����˵����
 *	url�������url
 * ����ֵ��
 *	����ɹ�����http_response��ʧ�ܷ���NULL
 */
http_response* moon_https_get(const char* url);

/**
 * ����˵����
 *	https GET������֤��
 * ����˵����
 *	url�������url
 *	headerName����ӵ�����ͷKEY����
 *	headerValue����ӵ�����ͷVALUE����
 *	headerCount������ͷ������
 */
http_response* moon_https_get_ex(const char* url,const char* headerName[],const char* headerValue[],int headerCount);

/**
 * ����˵����
 *	https POST������֤��
 * ����˵����
 *	url�������url
 *	params������Ĳ���,�磺ppara1=val1&para2=val2&��
 * ����ֵ��
 *	����ɹ�����http_response��ʧ�ܷ���NULL
 */
http_response* moon_https_post(const char* url, const char* params);

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
http_response* moon_https_post_ex(const char* url,const char* params,const char* headerName[],const char* headerValue[],int headerCount);


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
http_response* moon_http_download(const char*url,const char* saveFilePath,DownLoadReceiptDataLengthCallback callback);
#endif