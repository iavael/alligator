#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>
#include "dstructures/tommyds/tommyds/tommy.h"
#include "main.h"
#include "client.h"
#include "parsers/multiparser.h"
#include "events/uv_alloc.h"
#include "common/rtime.h"
#include "common/url.h"

//void alloc_cb(uv_handle_t* handle, size_t size, uv_buf_t* buf)
//{
//	(void)handle;
//	buf->base = malloc(size);
//	buf->len = size;
//}

void on_close(uv_handle_t* handle)
{
	extern aconf* ac;
	client_info *cinfo = handle->data;
	if (ac->log_level > 2)
		printf("3: on_close cinfo with addr %p(%p:%p) with key %s, hostname %s\n", cinfo, cinfo->socket, cinfo->connect, cinfo->key, cinfo->hostname);
	(void)cinfo;
	free(cinfo->socket);
	free(cinfo->connect);
	cinfo->lock = 0;
}

void on_read(uv_stream_t* tcp, ssize_t nread, const uv_buf_t *buf)
{
	client_info *cinfo = tcp->data;
	extern aconf* ac;
	if (ac->log_level > 2)
		printf("2r: on_read cinfo with addr %p(%p:%p) with key %s, hostname %s\n", cinfo, cinfo->socket, cinfo->connect, cinfo->key, cinfo->hostname);
	if(nread < 0 || !buf || !buf->base)
	{
		if (cinfo)
			uv_close((uv_handle_t*)tcp, on_close);
		else
			uv_close((uv_handle_t*)tcp, NULL);
		if (buf && buf->base)
			free(buf->base);

		return;
	}
	cinfo->read_time_finish = setrtime();
	int64_t connect_time = getrtime_i(cinfo->connect_time, cinfo->connect_time_finish);
	int64_t write_time = getrtime_i(cinfo->write_time, cinfo->write_time_finish);
	int64_t read_time = getrtime_i(cinfo->read_time, cinfo->read_time_finish);
	//printf("CWR %"d64":%"d64":%"d64"\n", connect_time, write_time, read_time);
	metric_labels_add_lbl3("aggregator_duration_time", &connect_time, ALLIGATOR_DATATYPE_INT, 0, "proto", "tcp", "type", "connect", "url", cinfo->hostname);
	metric_labels_add_lbl3("aggregator_duration_time", &write_time, ALLIGATOR_DATATYPE_INT, 0, "proto", "tcp", "type", "write", "url", cinfo->hostname);
	metric_labels_add_lbl3("aggregator_duration_time", &read_time, ALLIGATOR_DATATYPE_INT, 0, "proto", "tcp", "type", "read", "url", cinfo->hostname);
	//fprintf(stderr, "read: %s (%zu)\n", buf->base, strlen(buf->base));
	if (cinfo)
		alligator_multiparser(buf->base, buf->len, cinfo->parser_handler, NULL, cinfo);
	else
		alligator_multiparser(buf->base, buf->len, NULL, NULL, NULL);
	free(buf->base);
	uv_close((uv_handle_t*)tcp, on_close);
}

void on_write(uv_write_t* req, int status)
{
	if (status)
	{
		fprintf(stderr, "uv_write error: %s\n", uv_strerror(status));
		free(req);
		//uv_close((uv_handle_t*)req->handle, on_close);
		//uv_close((uv_handle_t*)req->handle, NULL);
		return;
	}

	extern aconf* ac;

	client_info *cinfo = req->data;
	if (ac->log_level > 2)
		printf("2w: on_write cinfo with addr %p(%p:%p) with key %s, hostname %s\n", cinfo, cinfo->socket, cinfo->connect, cinfo->key, cinfo->hostname);
	cinfo->write_time_finish = setrtime();
	req->handle->data = cinfo;
	uv_read_start(req->handle, alloc_buffer, on_read);
	cinfo->read_time = setrtime();

	free(req);
}

void on_connect(uv_connect_t* connection, int status)
{
	if ( status < 0 )
		return;

	extern aconf* ac;

	client_info *cinfo = connection->data;
	uv_stream_t* stream = connection->handle;
	connection->handle->data = cinfo;
	stream->data = cinfo;
	cinfo->connect_time_finish = setrtime();

	if (ac->log_level > 2)
		printf("1: on_connect cinfo with addr %p(%p:%p) with key %s, hostname %s\n", cinfo, cinfo->socket, cinfo->connect, cinfo->key, cinfo->hostname);
	uv_read_start(stream, alloc_buffer, on_read);
	if (cinfo->write == 1)
	{
		uv_buf_t buffer = uv_buf_init(cinfo->mesg, strlen(cinfo->mesg)+1);
		uv_write_t *request = malloc(sizeof(*request));
		request->data = cinfo;
		uv_write(request, stream, &buffer, 1, on_write);
		cinfo->write_time = setrtime();
	}
	else if (cinfo->write == 2)
	{
		uv_write_t *request = malloc(sizeof(*request));
		request->data = cinfo;

		uv_write(request, stream, cinfo->buffer, cinfo->buflen, on_write);
		cinfo->write_time = setrtime();
	}
}

void on_connect_handler(void* arg)
{
	extern aconf* ac;
	uv_loop_t *loop = ac->loop;
	client_info *cinfo = arg;
	if (cinfo->lock)
		return;
	cinfo->lock = 1;
	uv_tcp_t *socket = cinfo->socket = malloc(sizeof(*socket));
	uv_tcp_init(loop, socket);
	uv_tcp_keepalive(socket, 1, 60);

	uv_connect_t *connect = cinfo->connect = malloc(sizeof(*connect));
	if (ac->log_level > 2)
		printf("0: on_connect_handler cinfo with addr %p(%p:%p) with key %s, hostname %s\n", cinfo, cinfo->socket, cinfo->connect, cinfo->key, cinfo->hostname);
	connect->data = cinfo;
	uv_tcp_connect(connect, socket, (struct sockaddr *)cinfo->dest, on_connect);
	cinfo->connect_time = setrtime();
}


static void timer_cb(uv_timer_t* handle) {
	(void)handle;
	extern aconf* ac;
	tommy_hashdyn_foreach(ac->aggregator, on_connect_handler);
}

void tcp_on_resolved(uv_getaddrinfo_t *resolver, int status, struct addrinfo *res)
{
	extern aconf* ac;
	client_info *cinfo = resolver->data;

	if (status == -1 || !res) {
		fprintf(stderr, "getaddrinfo callback error\n");
		return;
	}
	else
		printf("resolved %s\n", cinfo->hostname);

	char *addr = calloc(17, sizeof(*addr));
	uv_ip4_name((struct sockaddr_in*)res->ai_addr, addr, 16);
	cinfo->dest = (struct sockaddr_in*)res->ai_addr;
	cinfo->key = malloc(64);
	snprintf(cinfo->key, 64, "%s:%u:%d", addr, cinfo->dest->sin_port, cinfo->dest->sin_family);

	tommy_hashdyn_insert(ac->aggregator, &(cinfo->node), cinfo, tommy_strhash_u32(0, cinfo->key));
}

void do_tcp_client(char *addr, char *port, void *handler, char *mesg)
{
	extern aconf* ac;
	uv_loop_t *loop = ac->loop;
	client_info *cinfo = calloc(1, sizeof(*cinfo));
	if (ac->log_level > 2)
		printf("allocated CINFO with addr %p with hostname '%s' with mesg '%s'\n", cinfo, addr, mesg);
	cinfo->mesg = mesg;
	if (mesg)
		cinfo->write = 1;
	else
		cinfo->write = 0;
	cinfo->parser_handler = handler;
	cinfo->hostname = addr;
	cinfo->proto = APROTO_TCP;
	cinfo->port = port;

	uv_getaddrinfo_t *resolver = malloc(sizeof(*resolver));
	resolver->data = cinfo;
	int r = uv_getaddrinfo(loop, resolver, tcp_on_resolved, addr, port, NULL);
	if (r)
	{
		fprintf(stderr, "%s\n", uv_strerror(r));
		return;
	}
	else
	{
		(ac->tcp_client_count)++;
	}
}

void do_tcp_client_buffer(char *addr, char *port, void *handler, uv_buf_t* buffer, size_t buflen)
{
	extern aconf* ac;
	uv_loop_t *loop = ac->loop;
	client_info *cinfo = calloc(1, sizeof(*cinfo));
	if (ac->log_level > 2)
		printf("allocated CINFO with addr %p with hostname '%s' with buf '%p'\n", cinfo, addr, buffer);
	cinfo->buffer = buffer;
	cinfo->buflen = buflen;
	cinfo->write = 2;
	cinfo->parser_handler = handler;
	cinfo->hostname = addr;
	cinfo->proto = APROTO_TCP;
	cinfo->port = port;

	uv_getaddrinfo_t *resolver = malloc(sizeof(*resolver));
	resolver->data = cinfo;
	int r = uv_getaddrinfo(loop, resolver, tcp_on_resolved, addr, port, NULL);
	if (r)
	{
		fprintf(stderr, "%s\n", uv_strerror(r));
		return;
	}
	else
	{
		(ac->tcp_client_count)++;
	}
}

void tcp_client_handler()
{
	extern aconf* ac;
	uv_loop_t *loop = ac->loop;

	uv_timer_t *timer1 = calloc(1, sizeof(*timer1));
	uv_timer_init(loop, timer1);
	uv_timer_start(timer1, timer_cb, ac->aggregator_startup, ac->aggregator_repeat);
}
