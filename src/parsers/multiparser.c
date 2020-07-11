#include <stdio.h>
#include <string.h>
#include "common/https_tls_check.h"
#include "main.h"
#include "parsers/http_proto.h"
#include "common/selector.h"

void do_http_post(char *buf, size_t len, string *response, http_reply_data* http_data, context_arg *carg)
{
	char *body = http_data->body;
	char *uri = http_data->uri;
	extern aconf *ac;

	if (!strncmp(http_data->uri, "/api", 4))
	{
		api_router(response, http_data);
	}
	else
	{
		if (ac->log_level > 2)
			printf("Query: %s\n", uri);
		if (ac->log_level > 10)
			printf("get metrics from body:\n%s\n", body);
		multicollector(http_data, NULL, 0, carg);
		string_cat(response, "HTTP/1.1 202 Accepted\n\n", strlen("HTTP/1.1 202 Accepted\n\n")+1);
	}
}

void do_http_delete(char *buf, size_t len, string *response, http_reply_data* http_data, context_arg *carg)
{
	extern aconf *ac;

	if (!strncmp(http_data->uri, "/api", 4))
		api_router(response, http_data);
	else
		string_cat(response, "HTTP/1.1 404 Not Found\r\nServer: alligator\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\n", strlen("HTTP/1.1 404 Not Found\r\nServer: alligator\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\n"));
}

void do_http_get(char *buf, size_t len, string *response, http_reply_data* http_data)
{
	if (!strncmp(http_data->uri, "/api", 4))
	{
		api_router(response, http_data);
	}
	else if (!strncmp(http_data->uri, "/stats", 6))
	{
		stat_router(response, http_data);
	}
	else
	{
		//metric_str_build(0, response);
		string *body = string_init(response->m);
		metric_str_build(0, body);

		char *content_length = malloc(255);
		snprintf(content_length, 255, "Content-Length: %zu\r\n\r\n", body->l);

		string_cat(response, "HTTP/1.1 200 OK\r\nServer: alligator\r\nContent-Type: text/plain\r\nConnection: close\r\n", strlen("HTTP/1.1 200 OK\r\nServer: alligator\r\nContent-Type: text/plain\r\nConnection: close\r\n"));
		string_cat(response, content_length, strlen(content_length));
		string_cat(response, body->s, body->l);

		free(content_length);
		string_free(body);
	}
}

void do_http_put(char *buf, size_t len, string *response, http_reply_data* http_data, context_arg *carg)
{
	do_http_post(buf, len, response, http_data, carg);
	//string_cat(response, "HTTP/1.1 400 Bad Query\n\n", strlen("HTTP/1.1 400 Bad Query\n\n")+1);
}

void do_http_response(char *buf, size_t len, string *response, context_arg *carg, http_reply_data* http_data)
{
	(void)response;
	char *tmp;
	if ( (tmp = strstr(buf, "\n\n")) )
		multicollector(http_data, tmp+2, len - (tmp-buf) - 2, carg);
	else if ( (tmp = strstr(buf, "\r\n\r\n")) )
		multicollector(http_data, tmp+4, len - (tmp-buf) - 4, carg);
}

int http_parser(char *buf, size_t len, string *response, context_arg *carg)
{
	if (!response)
		return 0;

	int ret = 1;

	http_reply_data* http_data = http_proto_get_request_data(buf, len);
	if(!http_data)
		return 0;

	if (!http_check_auth(carg, http_data))
	{
		string_cat(response, "HTTP/1.1 403 Access Forbidden\r\nServer: alligator\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\n", strlen("HTTP/1.1 403 Access Forbidden\r\nServer: alligator\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\n"));
		return 1;
	}

	if (http_data->method == HTTP_METHOD_POST)
	{
		if (http_data->body)
			do_http_post(buf, len, response, http_data, carg);
	}
	else if (http_data->method == HTTP_METHOD_RESPONSE)
	{
		do_http_response(buf, len, response, carg, http_data);
	}
	else if (http_data->method == HTTP_METHOD_GET)
	{
		do_http_get(buf, len, response, http_data);
	}
	else if (http_data->method == HTTP_METHOD_PUT)
	{
		if (http_data->body)
	 		do_http_put(buf, len, response, http_data, carg);
	}
	else if (http_data->method == HTTP_METHOD_DELETE)
	{
		if (http_data->body)
	 		do_http_delete(buf, len, response, http_data, carg);
	}
	else	ret = 0;

	http_reply_data_free(http_data);
	return ret;
}

int plain_parser(char *buf, size_t len, context_arg *carg)
{
	multicollector(NULL, buf, len, carg);
	return 1;
}

void alligator_multiparser(char *buf, size_t slen, void (*handler)(char*, size_t, context_arg*), string *response, context_arg *carg)
{
	//puts("========================================================================");
	//printf("handler (%p) parsing '%s'(%zu)\n", handler, buf, slen);
	//puts("========================================================================");
	if (!buf)
		return;

	size_t len = slen;

	if (carg)
		carg->parsed = 1;
	
	if ( handler )
	{
		char *proxybuf;
		uint64_t proxylen;
		if (carg->http_body)
		{
			proxybuf = carg->http_body;
			//printf("http %llu - (%llu - %llu))\n", carg->full_body->l, carg->http_body, carg->full_body->s);
			proxylen = (carg->full_body->l - (carg->http_body - carg->full_body->s)) + 1;
			if (proxylen > carg->full_body->l)
				proxylen = 0;
		}
		else
		{
			proxybuf = buf;
			proxylen = len;
		}
		//printf(">>>>>>>>>>>>>>>\n%zu\n'%s'\n<<<<<<<<<<<<<<<<<<\n", proxylen, proxybuf);
		handler(proxybuf, proxylen, carg);
		return;
	}
	int rc = 0;
	if ( (rc = http_parser(buf, len, response, carg)) ) {}
	else if ( (rc = plain_parser(buf, len, carg)) ) {}
}
