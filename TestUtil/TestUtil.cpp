// TestUtil.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "../NetWork/api/http_client.h"
#include <stdio.h>
#include <string.h>

#pragma comment(lib,"../Debug/NetWork.lib")


void DownloadDataCall(unsigned long size)
{
	printf("��ǰ�Ѿ�����:%ld",size);
}

int _tmain(int argc, _TCHAR* argv[])
{
	/*const char* headerKey[] =
	{
		"Referer"
	};

	const char* headerValue[] =
	{
		"https://ui.ptlogin2.qq.com/cgi-bin/login?daid=164&target=self&style=16&mibao_css=m_webqq&appid=501004106&enable_qlogin=0&no_verifyimg=1&s_url=http%3A%2F%2Fw.qq.com%2Fproxy.html&f_url=loginerroralert&strong_login=1&login_state=10&t=20131024001"
	};
	http_response* p = https_get_ex("https://ssl.ptlogin2.qq.com/ptqrlogin?ptqrtoken=1501220135&webqq_type=10&remember_uin=1&login2qq=1&aid=501004106&u1=http%3A%2F%2Fw.qq.com%2Fproxy.html%3Flogin2qq%3D1%26webqq_type%3D10&ptredirect=0&ptlang=2052&daid=164&from_ui=1&pttype=1&dumy=&fp=loginerroralert&0-0-157510&mibao_css=m_webqq&t=undefined&g=1&js_type=0&js_ver=10184&login_sig=&pt_randsalt=3",headerKey,headerValue,1);

	
	http_response_free(p);*/

	//��������
	http_response* pHttpResponse = http_download("http://192.168.2.132:8080/downloadUpgradePkg?currentVersion=V1.0.0.4&pkgType=1001","E:\\CompanyProject\\�Զ�������Ŀ\\��02��source\\��01���ͻ���\\SmartClientUpdate\\updateTmp\\",DownloadDataCall);
	if (pHttpResponse == NULL)
	{
		printf("�ļ�����ʧ�ܣ���������");
		return 0;
	}
	return 0;
}



//#define MAXBUF 4096  
//#define DEF_PORT    443
//#define DEF_MONITOR_PORT    4444
//const char* str_url = "www.2345.com";
// 
//void ShowCerts(SSL * ssl)  
//{  
//    X509 *cert;  
//    char *line;  
//    cert = SSL_get_peer_certificate(ssl);  
//    if (cert != NULL)   
//    {  
//        printf("����֤����Ϣ:\n");  
//        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);  
//        printf("֤��: %s\n", line);  
//        OPENSSL_free(line);  
//        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);  
//        printf("�䷢��: %s\n", line);  
//        OPENSSL_free(line);  
//        X509_free(cert);  
//    }  
//    else  
//        printf("��֤����Ϣ��\n");  
//}  
////    ssl  Client
//int _tmain(int argc, _TCHAR* argv[])
//{
//    int sockfd, len;  
//    struct sockaddr_in dest;  
//    char buffer[MAXBUF + 1];  
//    SSL_CTX *ctx;  
//    SSL *ssl;  
// 
//    /* SSL ���ʼ�����ο� ssl-server.c ���� */  
//    SSL_library_init();   //SSL ���ʼ�� 
//    OpenSSL_add_all_algorithms();  //�������� SSL �㷨
//    SSL_load_error_strings();  //�������� SSL ������Ϣ
//    ctx = SSL_CTX_new(SSLv23_client_method());  // �� SSL V2 �� V3 ��׼���ݷ�ʽ����һ�� SSL_CTX ���� SSL Content Text *//* Ҳ������ SSLv2_server_method() �� SSLv3_server_method() ������ʾ V2 �� V3��׼ 
//    if (ctx == NULL)   
//    {  
//        ERR_print_errors_fp(stdout);  
//        exit(1);  
//    }  
// 
//    WSADATA wsaData;  
//    int ret = WSAStartup( MAKEWORD(2, 2), &wsaData );  
//    if ( ret != 0 )   
//    {  
//        return -1;  
//    }  
//    /* ����һ�� socket ���� tcp ͨ�� */  
//    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)  
//    {  
//        perror("Socket");  
//        exit(errno);  
//    }  
//    printf("socket created\n");  
// 
//    //�󶨿ͻ���
//    //sockaddr_in native;
//    //native.sin_family = AF_INET;
//    //native.sin_port = htons(444);
//    //native.sin_addr.S_un.S_addr = inet_addr("192.168.9.59");
//    //int i = bind(sockfd , (const sockaddr*)&native , sizeof(sockaddr));
// 
//    /* ��ʼ���������ˣ��Է����ĵ�ַ�Ͷ˿���Ϣ */  
//    char   str_ip[32] = {0};
//    struct hostent* hptr = gethostbyname(str_url);
//    inet_ntop(hptr->h_addrtype , hptr->h_addr , str_ip , sizeof(str_ip));
//    memset(&dest,0, sizeof(dest));  
//    dest.sin_family = AF_INET;   
//    dest.sin_addr.S_un.S_addr = inet_addr(str_ip);  
//    dest.sin_port             = htons (DEF_PORT);  
//    //dest.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
//    //dest.sin_port             = htons (6000); 
// 
// 
//    printf("address created\n");  
// 
//    /* ���ӷ����� */  
//    if (connect(sockfd, (struct sockaddr *) &dest, sizeof(dest)) != 0) {  
//        perror("Connect ");  
//        exit(errno);  
//    }  
//    printf("server connected\n");  
// 
//    /* ���� ctx ����һ���µ� SSL */  
//    ssl = SSL_new(ctx);  
//    SSL_set_fd(ssl, sockfd);  
//    /* ���� SSL ���� */  
//    if (SSL_connect(ssl) == -1)  
//        ERR_print_errors_fp(stderr);  
//    else  
//    {  
//        printf("Connected with %s encryption\n", SSL_get_cipher(ssl));  
//        ShowCerts(ssl);  
//    }  
// 
//    char* pPackets = "GET https://www.2345.com/ HTTP/1.1\r\n"\
//        "Host: www.2345.com\r\n"\
//        "Connection: keep-alive\r\n"\
//        "Cache-Control: max-age=0\r\n"\
//        "Upgrade-Insecure-Requests: 1\r\n"\
//        "User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/55.0.2883.87 Safari/537.36\r\n"\
//        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"\
//        "Accept-Encoding: gzip, deflate, sdch, br\r\n"\
//        "Accept-Language: zh-CN,zh;q=0.8\r\n\r\n";
// 
//    memset(buffer,0, MAXBUF + 1);  
//    strcpy(buffer , pPackets);
//    len = SSL_write(ssl, buffer, strlen(buffer));  
// 
//    char* recv_buffer = new char[1200];  
//    memset(recv_buffer , 0 , 1200);
//    len = SSL_read(ssl, recv_buffer, 1200);  
// 
// 
//    memset(recv_buffer , 0 , 1200);
//    len = SSL_read(ssl, recv_buffer, 1200);  
// 
// 
//    SSL_shutdown(ssl);  
//    SSL_free(ssl);  
//    closesocket(sockfd);  
//    SSL_CTX_free(ctx);  
//    system("pause");  
//    return 0;  
//}