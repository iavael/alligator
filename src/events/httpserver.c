#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <uv.h>
#include "events/uv_alloc.h"
#include "events/context_arg.h"
#include "common/selector.h"

typedef struct http_info
{
	uv_stream_t *client;
	char *answ;
} http_info;

//typedef struct tcp_entrypoint_info
//{
//	void *parser_handler;
//	context_arg->carg;
//} tcp_entrypoint_info;

#define CONNECTIONS_COUNT (128)

void http_after_connect(uv_handle_t* handle)
{
	free(handle);
}

void socket_write_cb(uv_write_t *req, int status)
{
	if (status) {
		fprintf(stderr, "Write error %s\n", uv_strerror(status));
	}

	http_info *hinfo = req->data;
	uv_stream_t *client = hinfo->client;
	uv_close((uv_handle_t *)client, http_after_connect);
	free(hinfo->answ);
	free(hinfo);
	free(req);
}

void socket_read_cb(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf)
{
	http_info *hinfo = malloc(sizeof(*hinfo));
	if (nread < 0) {
		if (nread != UV_EOF)
			fprintf(stderr, "Read error %s\n", uv_err_name(nread));

		uv_close((uv_handle_t*) client, http_after_connect);
	} else if (nread > 0) {
		context_arg *carg = client->data;
		string *str = string_init(6553500);
		alligator_multiparser(buf->base, nread, carg->parser_handler, str, carg);
		char *answ = str->s;
		size_t answ_len = str->l;
		//printf("answ: %s\n", answ);
		//data->response = answ; //strdup("HTTP/1.1 200 OK\n\nHello World, work's done\n\n");

		uv_write_t *req = (uv_write_t *) malloc(sizeof(uv_write_t));
		uv_buf_t write_buf = uv_buf_init(answ, answ_len);
		hinfo->answ = answ;
		hinfo->client = client;
		req->data = hinfo;

		free(str);
		uv_write(req, client, &write_buf, 1, socket_write_cb);
	}

	if (buf->base)
		free(buf->base);
}

void accept_connection_cb(uv_stream_t *server, int status)
{
	if (status < 0) {
		fprintf(stderr, "New connection error %s\n", uv_strerror(status));
		return;
	}

	uv_tcp_t *client = (uv_tcp_t*) malloc(sizeof(uv_tcp_t));
	uv_tcp_init(uv_default_loop(), client);
	client->data = server->data;

	if (uv_accept(server, (uv_stream_t*) client) == 0) {
		uv_read_start((uv_stream_t *) client, alloc_buffer, socket_read_cb);
	} else {
		uv_close((uv_handle_t*) client, http_after_connect);
	}
}

void tcp_server_handler(char *addr, uint16_t port, void* handler, context_arg *carg)
{
	if (!carg)
		carg = malloc(sizeof(*carg));
	carg->parser_handler = handler;
	uv_tcp_t *server = calloc(1, sizeof(uv_tcp_t));
	server->data = carg;
	struct sockaddr_in *saddr = calloc(1, sizeof(*saddr));

	uv_tcp_init(uv_default_loop(), server);

	uv_ip4_addr(addr, port, saddr);
	uv_tcp_bind(server, (const struct sockaddr*)saddr, 0);

	int ret = uv_listen((uv_stream_t*) server, CONNECTIONS_COUNT, accept_connection_cb);
	if(ret) {
		fprintf(stderr, "Listen error %s\n", uv_strerror(ret));
		return;
	}
}
