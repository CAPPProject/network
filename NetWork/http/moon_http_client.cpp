#include "moon_http_client.h"

/**
 *	Handles redirect if needed for get requests
 */
http_response* moon_handle_redirect_get(http_response* hresp, char* custom_headers)
{
	if(hresp->status_code_int > 300 && hresp->status_code_int < 399)
	{
		char *token = strtok(hresp->response_headers, "\r\n");
		while(token != NULL)
		{
			if(moon_str_contains(token, "Location:"))
			{
				/* Extract url */
				char *location = moon_str_replace("Location: ", "", token);
				return moon_http_get(location, custom_headers);
			}
			token = strtok(NULL, "\r\n");
		}
	}
	else
	{
		/* We're not dealing with a redirect, just return the same structure */
		return hresp;
	}
}

/**
 *	Handles redirect if needed for head requests
 */
http_response* moon_handle_redirect_head(http_response* hresp, char* custom_headers)
{
	if(hresp->status_code_int > 300 && hresp->status_code_int < 399)
	{
		char *token = strtok(hresp->response_headers, "\r\n");
		while(token != NULL)
		{
			if(moon_str_contains(token, "Location:"))
			{
				/* Extract url */
				char *location = moon_str_replace("Location: ", "", token);
				return moon_http_head(location, custom_headers);
			}
			token = strtok(NULL, "\r\n");
		}
	}
	else
	{
		/* We're not dealing with a redirect, just return the same structure */
		return hresp;
	}
}

/**
 *	Handles redirect if needed for post requests
 */
http_response* moon_handle_redirect_post(http_response* hresp, char* custom_headers, char *post_data)
{
	if(hresp->status_code_int > 300 && hresp->status_code_int < 399)
	{
		char *token = strtok(hresp->response_headers, "\r\n");
		while(token != NULL)
		{
			if(moon_str_contains(token, "Location:"))
			{
				/* Extract url */
				char *location = moon_str_replace("Location: ", "", token);
				return moon_http_post(location, custom_headers, post_data);
			}
			token = strtok(NULL, "\r\n");
		}
	}
	else
	{
		/* We're not dealing with a redirect, just return the same structure */
		return hresp;
	}
}

/**
 *	Makes a HTTP request and returns the response
 */
http_response* moon_http_request(char *http_headers, parsed_url *purl)
{
	/* Declare variable */
	char *body = NULL;
	int sent = 0;
	int errnum = 0;
	int sock;
	int tmpres;
	char buf[BUFSIZ+1];
	struct sockaddr_in *remote;
	http_response *hresp = NULL;
	char *response = NULL;
	char BUF[BUFSIZ];
	size_t recived_len = 0;
	char *status_line = NULL;
	char *status_code = NULL;
	char *status_text = NULL;
	char *headers = NULL;
	unsigned long length = 0;
#ifdef _WIN32
	WSADATA wsaData;
#endif

	/* Parse url */
	if(purl == NULL)
	{
		printf("Unable to parse url");
		return NULL;
	}

	/* Allocate memeory for htmlcontent */
	hresp = (http_response*)malloc(sizeof(http_response));
	if(hresp == NULL)
	{
		printf("Unable to allocate memory for htmlcontent.");
		return NULL;
	}
	hresp->body = NULL;
	hresp->request_headers = NULL;
	hresp->response_headers = NULL;
	hresp->status_code = NULL;
	hresp->status_text = NULL;
	hresp->request_uri = NULL;
	/*window need init socket*/
#ifdef _WIN32
	if (WSAStartup(MAKEWORD(2,2),&wsaData) != 0)
	{
		errnum = WSAGetLastError();
		printf("Can't init socket\n");
		moon_http_response_free(hresp);
		return NULL;
	}
#endif
	/* Create TCP socket */
	if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		printf("Can't create TCP socket\n");
		moon_http_response_free(hresp);
		return NULL;
	}

	/* Set remote->sin_addr.s_addr */
	remote = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in *));
	remote->sin_family = AF_INET;
  	tmpres = inet_pton(AF_INET, purl->ip, (void *)(&(remote->sin_addr.s_addr)));
  	if( tmpres < 0)
  	{
    	printf("Can't set remote->sin_addr.s_addr\n");
    	return NULL;
  	}
	else if(tmpres == 0)
  	{
		printf("Not a valid IP");
    	return NULL;
  	}
	remote->sin_port = htons(atoi(purl->port));

	/* Connect */
	if(connect(sock, (struct sockaddr *)remote, sizeof(struct sockaddr)) < 0)
	{
	    printf("Could not connect");
		return NULL;
	}

	/* Send headers to server */	
	while(sent < strlen(http_headers))
	{
	    tmpres = send(sock, http_headers+sent, strlen(http_headers)-sent, 0);
		if(tmpres == -1)
		{
			printf("Can't send headers");
			return NULL;
		}
		sent += tmpres;
	 }

	/* Recieve into response*/
	response = (char*)malloc(1);
	memset(response,0,1);
	while((recived_len = recv(sock, BUF, BUFSIZ-1, 0)) > 0)
	{
        BUF[recived_len] = '\0';
		length = strlen(response) + strlen(BUF) + 1;
		response = (char*)realloc(response,length);
		sprintf(response, "%s%s", response, BUF);
	}
	if (recived_len < 0)
    {
		free(http_headers);
		#ifdef _WIN32
			closesocket(sock);
		#else
			close(sock);
		#endif
        printf("Unabel to recieve");
		return NULL;
    }

	/* Reallocate response */
	response = (char*)realloc(response, strlen(response) + 1);

	/* Close socket */
	#ifdef _WIN32
		closesocket(sock);
	#else
		close(sock);
	#endif
	/* Parse status code and text */
	status_line = moon_get_until(response, "\r\n");
	status_line = moon_str_replace("HTTP/1.1 ", "", status_line);
	status_code = moon_str_ndup(status_line, 4);
	status_code = moon_str_replace(" ", "", status_code);
	status_text = moon_str_replace(status_code, "", status_line);
	status_text = moon_str_replace(" ", "", status_text);
	hresp->status_code = status_code;
	hresp->status_code_int = atoi(status_code);
	hresp->status_text = status_text;
	
	/* Parse response headers */	
	headers = moon_get_until(response, "\r\n\r\n");
	hresp->response_headers = headers;

	/* Assign request headers */
	hresp->request_headers = http_headers;

	/* Assign request url */
	hresp->request_uri = purl;

	/* Parse body */
	body = strstr(response, "\r\n\r\n");
	body = moon_str_replace("\r\n\r\n", "", body);
	hresp->body = body;

	/* Return response */
	return hresp;
}

/**
 *	Makes a HTTP GET request to the given url
 */
http_response* moon_http_get(char *url, char *custom_headers)
{
	/* Declare variable */
	char *http_headers = NULL;
	char *upwd = NULL;
	char *base64 = NULL;
	char *auth_header = NULL;
	http_response *hresp = NULL;
	/* Parse url */
	parsed_url *purl = moon_parse_url(url);
	if(purl == NULL)
	{
		printf("Unable to parse url");
		return NULL;
	}

	
	http_headers = (char*)malloc(1024);

	/* Build query/headers */
	if(purl->path != NULL)
	{
		if(purl->query != NULL)
		{
			sprintf(http_headers, "GET /%s?%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->path, purl->query, purl->host);
		}
		else
		{
			sprintf(http_headers, "GET /%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->path, purl->host);
		}
	}
	else
	{
		if(purl->query != NULL)
		{
			sprintf(http_headers, "GET /?%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->query, purl->host);
		}
		else
		{
			sprintf(http_headers, "GET / HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->host);
		}
	}

	/* Handle authorisation if needed */
	if(purl->username != NULL)
	{
		/* Format username:password pair */
		upwd = (char*)malloc(1024);
		sprintf(upwd, "%s:%s", purl->username, purl->password);
		upwd = (char*)realloc(upwd, strlen(upwd) + 1);

		/* Base64 encode */
		base64 = moon_base64_encode(upwd);

		/* Form header */
		auth_header = (char*)malloc(1024);
		sprintf(auth_header, "Authorization: Basic %s\r\n", base64);
		auth_header = (char*)realloc(auth_header, strlen(auth_header) + 1);

		/* Add to header */
		http_headers = (char*)realloc(http_headers, strlen(http_headers) + strlen(auth_header) + 2);
		sprintf(http_headers, "%s%s", http_headers, auth_header);
	}

	/* Add custom headers, and close */
	if(custom_headers != NULL)
	{
		sprintf(http_headers, "%s%s\r\n", http_headers, custom_headers);
	}
	else
	{
		sprintf(http_headers, "%s\r\n", http_headers);
	}
	http_headers = (char*)realloc(http_headers, strlen(http_headers) + 1);

	/* Make request and return response */
	hresp = moon_http_request(http_headers, purl);
	if (hresp == NULL)
	{
		return NULL;
	}
	/* Handle redirect */

	return moon_handle_redirect_get(hresp, custom_headers);
}

/**
 *	Makes a HTTP POST request to the given url
 */
http_response* moon_http_post(char *url, char *custom_headers, char *post_data)
{
	/* Declare variable */
	char *http_headers = NULL;
	char *upwd = NULL;
	char *base64 = NULL;
	char *auth_header = NULL;
	http_response *hresp = NULL;
	/* Parse url */
	parsed_url *purl = NULL;
	purl = moon_parse_url(url);
	if(purl == NULL)
	{
		printf("Unable to parse url");
		return NULL;
	}

	
	http_headers = (char*)malloc(1024);

	/* Build query/headers */
	if(purl->path != NULL)
	{
		if(purl->query != NULL)
		{
			sprintf(http_headers, "POST /%s?%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\nContent-Length:%zu\r\nContent-Type:application/x-www-form-urlencoded\r\n", purl->path, purl->query, purl->host, strlen(post_data));
		}
		else
		{
			sprintf(http_headers, "POST /%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\nContent-Length:%zu\r\nContent-Type:application/x-www-form-urlencoded\r\n", purl->path, purl->host, strlen(post_data));
		}
	}
	else
	{
		if(purl->query != NULL)
		{
			sprintf(http_headers, "POST /?%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\nContent-Length:%zu\r\nContent-Type:application/x-www-form-urlencoded\r\n", purl->query, purl->host, strlen(post_data));
		}
		else
		{
			sprintf(http_headers, "POST / HTTP/1.1\r\nHost:%s\r\nConnection:close\r\nContent-Length:%zu\r\nContent-Type:application/x-www-form-urlencoded\r\n", purl->host, strlen(post_data));
		}
	}

	/* Handle authorisation if needed */
	if(purl->username != NULL)
	{
		/* Format username:password pair */
		upwd = (char*)malloc(1024);
		sprintf(upwd, "%s:%s", purl->username, purl->password);
		upwd = (char*)realloc(upwd, strlen(upwd) + 1);

		/* Base64 encode */
		base64 = moon_base64_encode(upwd);

		/* Form header */
		auth_header = (char*)malloc(1024);
		sprintf(auth_header, "Authorization: Basic %s\r\n", base64);
		auth_header = (char*)realloc(auth_header, strlen(auth_header) + 1);

		/* Add to header */
		http_headers = (char*)realloc(http_headers, strlen(http_headers) + strlen(auth_header) + 2);
		sprintf(http_headers, "%s%s", http_headers, auth_header);
	}

	if(custom_headers != NULL)
	{
		sprintf(http_headers, "%s%s\r\n", http_headers, custom_headers);
		sprintf(http_headers, "%s\r\n%s", http_headers, post_data);
	}
	else
	{
		sprintf(http_headers, "%s\r\n%s", http_headers, post_data);
	}
	http_headers = (char*)realloc(http_headers, strlen(http_headers) + 1);

	/* Make request and return response */
	hresp = moon_http_request(http_headers, purl);

	/* Handle redirect */
	return moon_handle_redirect_post(hresp, custom_headers, post_data);
}

/**
 *	Makes a HTTP HEAD request to the given url
 */
http_response* moon_http_head(char *url, char *custom_headers)
{
	/* Declare variable */
	char *http_headers = NULL;
	char *upwd = NULL;
	char *base64 = NULL;
	char *auth_header = NULL;
	http_response *hresp = NULL;
	/* Parse url */
	parsed_url *purl = moon_parse_url(url);
	if(purl == NULL)
	{
		printf("Unable to parse url");
		return NULL;
	}

	
	http_headers = (char*)malloc(1024);

	/* Build query/headers */
	if(purl->path != NULL)
	{
		if(purl->query != NULL)
		{
			sprintf(http_headers, "HEAD /%s?%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->path, purl->query, purl->host);
		}
		else
		{
			sprintf(http_headers, "HEAD /%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->path, purl->host);
		}
	}
	else
	{
		if(purl->query != NULL)
		{
			sprintf(http_headers, "HEAD/?%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->query, purl->host);
		}
		else
		{
			sprintf(http_headers, "HEAD / HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->host);
		}
	}

	/* Handle authorisation if needed */
	if(purl->username != NULL)
	{
		/* Format username:password pair */
		upwd = (char*)malloc(1024);
		sprintf(upwd, "%s:%s", purl->username, purl->password);
		upwd = (char*)realloc(upwd, strlen(upwd) + 1);

		/* Base64 encode */
		base64 = moon_base64_encode(upwd);

		/* Form header */
		auth_header = (char*)malloc(1024);
		sprintf(auth_header, "Authorization: Basic %s\r\n", base64);
		auth_header = (char*)realloc(auth_header, strlen(auth_header) + 1);

		/* Add to header */
		http_headers = (char*)realloc(http_headers, strlen(http_headers) + strlen(auth_header) + 2);
		sprintf(http_headers, "%s%s", http_headers, auth_header);
	}

	if(custom_headers != NULL)
	{
		sprintf(http_headers, "%s%s\r\n", http_headers, custom_headers);
	}
	else
	{
		sprintf(http_headers, "%s\r\n", http_headers);
	}
	http_headers = (char*)realloc(http_headers, strlen(http_headers) + 1);

	/* Make request and return response */
	hresp = moon_http_request(http_headers, purl);

	/* Handle redirect */
	return moon_handle_redirect_head(hresp, custom_headers);
}

/**
 *	Do HTTP OPTIONs requests
 */
http_response* moon_http_options(char *url)
{
	/* Declare variable */
	char *http_headers = NULL;
	char *upwd = NULL;
	char *base64 = NULL;
	char *auth_header = NULL;
	http_response *hresp = NULL;
	/* Parse url */
	parsed_url *purl = moon_parse_url(url);
	if(purl == NULL)
	{
		printf("Unable to parse url");
		return NULL;
	}

	
	http_headers = (char*)malloc(1024);

	/* Build query/headers */
	if(purl->path != NULL)
	{
		if(purl->query != NULL)
		{
			sprintf(http_headers, "OPTIONS /%s?%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->path, purl->query, purl->host);
		}
		else
		{
			sprintf(http_headers, "OPTIONS /%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->path, purl->host);
		}
	}
	else
	{
		if(purl->query != NULL)
		{
			sprintf(http_headers, "OPTIONS/?%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->query, purl->host);
		}
		else
		{
			sprintf(http_headers, "OPTIONS / HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->host);
		}
	}

	/* Handle authorisation if needed */
	if(purl->username != NULL)
	{
		/* Format username:password pair */
		upwd = (char*)malloc(1024);
		sprintf(upwd, "%s:%s", purl->username, purl->password);
		upwd = (char*)realloc(upwd, strlen(upwd) + 1);

		/* Base64 encode */
		base64 = moon_base64_encode(upwd);

		/* Form header */
		auth_header = (char*)malloc(1024);
		sprintf(auth_header, "Authorization: Basic %s\r\n", base64);
		auth_header = (char*)realloc(auth_header, strlen(auth_header) + 1);

		/* Add to header */
		http_headers = (char*)realloc(http_headers, strlen(http_headers) + strlen(auth_header) + 2);
		sprintf(http_headers, "%s%s", http_headers, auth_header);
	}

	/* Build headers */
	sprintf(http_headers, "%s\r\n", http_headers);
	http_headers = (char*)realloc(http_headers, strlen(http_headers) + 1);

	/* Make request and return response */
	hresp = moon_http_request(http_headers, purl);

	/* Handle redirect */
	return hresp;
}