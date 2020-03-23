#include "moon_url_parse.h"
/**
 *	Free memory of parsed url
 */
void moon_parsed_url_free(p_parsed_url purl)
{
    if ( NULL != purl ) 
	{
        if ( NULL != purl->scheme ) free(purl->scheme);
        if ( NULL != purl->host ) free(purl->host);
        if ( NULL != purl->port ) free(purl->port);
        if ( NULL != purl->path )  free(purl->path);
        if ( NULL != purl->query ) free(purl->query);
        if ( NULL != purl->fragment ) free(purl->fragment);
        if ( NULL != purl->username ) free(purl->username);
        if ( NULL != purl->password ) free(purl->password);
        free(purl);
    }
}

/**
 * function desc:
 *	Retrieves the IP adress of a hostname
 */
char* moon_hostname_to_ip(char *hostname)
{
	int errnum = 0;
	char *ip = "0.0.0.0";
	struct hostent *h;
#ifdef _WIN32
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2,2),&wsaData) != 0)
	{
		errnum = WSAGetLastError();
		printf("Can't init socket\n");
		return NULL;
	}
#endif
	if ((h=gethostbyname(hostname)) == NULL) 
	{  
		printf("gethostbyname");
		errnum = WSAGetLastError();
		return NULL;
	}
	return inet_ntoa(*((struct in_addr *)h->h_addr_list[0]));
}

/**
 * Check whether the character is permitted in scheme string
 */
int moon_is_scheme_char(int c)
{
	return (!isalpha(c) && '+' != c && '-' != c && '.' != c) ? 0 : 1;
}

/**
 *	Parses a specified URL and returns the structure named 'parsed_url'
 *	Implented according to:
 *	RFC 1738 - http://www.ietf.org/rfc/rfc1738.txt
 *	RFC 3986 -  http://www.ietf.org/rfc/rfc3986.txt
 */
p_parsed_url moon_parse_url(const char *url)
{
	/* Define variable */
    p_parsed_url purl;
    const char *tmpstr;
    const char *curstr;
    int len;
    int i;
    int userpass_flag;
    int bracket_flag;
	char *ip = NULL;

    /* Allocate the parsed url storage */
    purl = (parsed_url*)malloc(sizeof(parsed_url));
    if ( NULL == purl ) 
	{
        return NULL;
    }
    purl->scheme = NULL;
    purl->host = NULL;
    purl->port = NULL;
    purl->path = NULL;
    purl->query = NULL;
    purl->fragment = NULL;
    purl->username = NULL;
    purl->password = NULL;
    curstr = url;

    /*
     * <scheme>:<scheme-specific-part>
     * <scheme> := [a-z\+\-\.]+
     *             upper case = lower case for resiliency
     */
    /* Read scheme */
    tmpstr = strchr(curstr, ':');
    if ( NULL == tmpstr ) 
	{
        moon_parsed_url_free(purl); fprintf(stderr, "Error on line %d (%s)\n", __LINE__, __FILE__);
		
        return NULL;
    }

    /* Get the scheme length */
    len = tmpstr - curstr;

    /* Check restrictions */
    for ( i = 0; i < len; i++ ) 
	{
        if (moon_is_scheme_char(curstr[i]) == 0) 
		{
            /* Invalid format */
            moon_parsed_url_free(purl); fprintf(stderr, "Error on line %d (%s)\n", __LINE__, __FILE__);
            return NULL;
        }
    }
    /* Copy the scheme to the storage */
    purl->scheme = (char*)malloc(sizeof(char) * (len + 1));
    if ( NULL == purl->scheme ) 
	{
        moon_parsed_url_free(purl); fprintf(stderr, "Error on line %d (%s)\n", __LINE__, __FILE__);
		
        return NULL;
    }

    (void)strncpy(purl->scheme, curstr, len);
    purl->scheme[len] = '\0';

    /* Make the character to lower if it is upper case. */
    for ( i = 0; i < len; i++ ) 
	{
        purl->scheme[i] = tolower(purl->scheme[i]);
    }

    /* Skip ':' */
    tmpstr++;
    curstr = tmpstr;

    /*
     * //<user>:<password>@<host>:<port>/<url-path>
     * Any ":", "@" and "/" must be encoded.
     */
    /* Eat "//" */
    for ( i = 0; i < 2; i++ ) 
	{
        if ( '/' != *curstr ) 
		{
            moon_parsed_url_free(purl); fprintf(stderr, "Error on line %d (%s)\n", __LINE__, __FILE__);
            return NULL;
        }
        curstr++;
    }

    /* Check if the user (and password) are specified. */
    userpass_flag = 0;
    tmpstr = curstr;
    while ( '\0' != *tmpstr ) 
	{
        if ( '@' == *tmpstr ) 
		{
            /* Username and password are specified */
            userpass_flag = 1;
            break;
        } 
		else if ( '/' == *tmpstr ) 
		{
            /* End of <host>:<port> specification */
            userpass_flag = 0;
            break;
        }
        tmpstr++;
    }

    /* User and password specification */
    tmpstr = curstr;
    if ( userpass_flag ) 
	{
        /* Read username */
        while ( '\0' != *tmpstr && ':' != *tmpstr && '@' != *tmpstr ) 
		{
            tmpstr++;
        }
        len = tmpstr - curstr;
        purl->username = (char*)malloc(sizeof(char) * (len + 1));
        if ( NULL == purl->username ) 
		{
            moon_parsed_url_free(purl); fprintf(stderr, "Error on line %d (%s)\n", __LINE__, __FILE__);
            return NULL;
        }
        (void)strncpy(purl->username, curstr, len);
        purl->username[len] = '\0';

        /* Proceed current pointer */
        curstr = tmpstr;
        if ( ':' == *curstr ) 
		{
            /* Skip ':' */
            curstr++;
            
            /* Read password */
            tmpstr = curstr;
            while ( '\0' != *tmpstr && '@' != *tmpstr ) 
			{
                tmpstr++;
            }
            len = tmpstr - curstr;
            purl->password = (char*)malloc(sizeof(char) * (len + 1));
            if ( NULL == purl->password ) 
			{
                moon_parsed_url_free(purl); fprintf(stderr, "Error on line %d (%s)\n", __LINE__, __FILE__);
                return NULL;
            }
            (void)strncpy(purl->password, curstr, len);
            purl->password[len] = '\0';
            curstr = tmpstr;
        }
        /* Skip '@' */
        if ( '@' != *curstr ) 
		{
            moon_parsed_url_free(purl); fprintf(stderr, "Error on line %d (%s)\n", __LINE__, __FILE__);
            return NULL;
        }
        curstr++;
    }

    if ( '[' == *curstr ) 
	{
        bracket_flag = 1;
    } 
	else 
	{
        bracket_flag = 0;
    }
    /* Proceed on by delimiters with reading host */
    tmpstr = curstr;
    while ( '\0' != *tmpstr ) {
        if ( bracket_flag && ']' == *tmpstr )
 		{
            /* End of IPv6 address. */
            tmpstr++;
            break;
        } 
		else if ( !bracket_flag && (':' == *tmpstr || '/' == *tmpstr) ) 
		{
            /* Port number is specified. */
            break;
        }
        tmpstr++;
    }
    len = tmpstr - curstr;
    purl->host = (char*)malloc(sizeof(char) * (len + 1));
    if ( NULL == purl->host || len <= 0 ) 
	{
        moon_parsed_url_free(purl); fprintf(stderr, "Error on line %d (%s)\n", __LINE__, __FILE__);
        return NULL;
    }
    (void)strncpy(purl->host, curstr, len);
    purl->host[len] = '\0';
    curstr = tmpstr;

    /* Is port number specified? */
    if ( ':' == *curstr ) 
	{
        curstr++;
        /* Read port number */
        tmpstr = curstr;
        while ( '\0' != *tmpstr && '/' != *tmpstr ) 
		{
            tmpstr++;
        }
        len = tmpstr - curstr;
        purl->port = (char*)malloc(sizeof(char) * (len + 1));
        if ( NULL == purl->port ) 
		{
            moon_parsed_url_free(purl); fprintf(stderr, "Error on line %d (%s)\n", __LINE__, __FILE__);
            return NULL;
        }
        (void)strncpy(purl->port, curstr, len);
        purl->port[len] = '\0';
        curstr = tmpstr;
    }
	else
	{
		//http or https
		if (strcmp("http",purl->scheme) == 0)
		{
			purl->port = (char*)malloc(3);
			memset(purl->port,0,3);
			strcpy(purl->port,"80");
		}
		else if (strcmp("https",purl->scheme) == 0)
		{
			purl->port = (char*)malloc(4);
			memset(purl->port,0,4);
			strcpy(purl->port,"443");
		}
	}
	
	/* Get ip */
	ip = moon_hostname_to_ip(purl->host);
	purl->ip = ip;
	
	/* Set uri */
	purl->uri = (char*)url;

    /* End of the string */
    if ( '\0' == *curstr ) 
	{
        return purl;
    }

    /* Skip '/' */
    if ( '/' != *curstr ) 
	{
        moon_parsed_url_free(purl); fprintf(stderr, "Error on line %d (%s)\n", __LINE__, __FILE__);
        return NULL;
    }
    curstr++;

    /* Parse path */
    tmpstr = curstr;
    while ( '\0' != *tmpstr && '#' != *tmpstr  && '?' != *tmpstr ) 
	{
        tmpstr++;
    }
    len = tmpstr - curstr;
    purl->path = (char*)malloc(sizeof(char) * (len + 1));
    if ( NULL == purl->path ) 
	{
        moon_parsed_url_free(purl); fprintf(stderr, "Error on line %d (%s)\n", __LINE__, __FILE__);
        return NULL;
    }
    (void)strncpy(purl->path, curstr, len);
    purl->path[len] = '\0';
    curstr = tmpstr;

    /* Is query specified? */
    if ( '?' == *curstr ) 
	{
        /* Skip '?' */
        curstr++;
        /* Read query */
        tmpstr = curstr;
        while ( '\0' != *tmpstr && '#' != *tmpstr ) 
		{
            tmpstr++;
        }
        len = tmpstr - curstr;
        purl->query = (char*)malloc(sizeof(char) * (len + 1));
        if ( NULL == purl->query ) 
		{
            moon_parsed_url_free(purl); fprintf(stderr, "Error on line %d (%s)\n", __LINE__, __FILE__);
            return NULL;
        }
        (void)strncpy(purl->query, curstr, len);
        purl->query[len] = '\0';
        curstr = tmpstr;
    }

    /* Is fragment specified? */
    if ( '#' == *curstr ) 
	{
        /* Skip '#' */
        curstr++;
        /* Read fragment */
        tmpstr = curstr;
        while ( '\0' != *tmpstr ) 
		{
            tmpstr++;
        }
        len = tmpstr - curstr;
        purl->fragment = (char*)malloc(sizeof(char) * (len + 1));
        if ( NULL == purl->fragment )
 		{
            moon_parsed_url_free(purl); fprintf(stderr, "Error on line %d (%s)\n", __LINE__, __FILE__);
            return NULL;
        }
        (void)strncpy(purl->fragment, curstr, len);
        purl->fragment[len] = '\0';
        curstr = tmpstr;
    }
	return purl;
}