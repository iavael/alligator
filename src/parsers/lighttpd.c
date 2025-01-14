#include <stdio.h>
#include <inttypes.h>
#include "common/selector.h"
#include "metric/namespace.h"
#include "events/context_arg.h"
#include "common/http.h"
#include "main.h"
#define LIGHTTPD_LABEL_SIZE 100
void lighttpd_status_handler(char *metrics, size_t size, context_arg *carg)
{
	json_parser_entry(metrics, 0, NULL, "lighttpd", carg);
}

void lighttpd_statistics_handler(char *metrics, size_t size, context_arg *carg)
{
	char metric_name[LIGHTTPD_LABEL_SIZE];
	strlcpy(metric_name, "lighttpd_", 10);
	char module[LIGHTTPD_LABEL_SIZE];
	char type[LIGHTTPD_LABEL_SIZE];
	char target[LIGHTTPD_LABEL_SIZE];
	char backend[LIGHTTPD_LABEL_SIZE];
	char debug_string[LIGHTTPD_LABEL_SIZE];
	tommy_hashdyn *lbl = NULL;

	char *tmp = metrics;
	uint64_t sz;
	uint64_t val;
	for (uint64_t i = 0; i < size; i++)
	{
		if (carg->log_level > 0)
		{
			puts("=========");
			strlcpy(debug_string, tmp, strcspn(tmp, "\n")+1);
			printf("metric '%s', %p\n", debug_string, tmp);
		}

		lbl = calloc(1, sizeof(*lbl));
		tommy_hashdyn_init(lbl);
		module[0] = 0;
		type[0] = 0;
		target[0] = 0;
		backend[0] = 0;

		sz = strcspn(tmp, ".:");
		strlcpy(module, tmp, sz+1);
		labels_hash_insert_nocache(lbl, "module", module);

		if (carg->log_level > 0)
			printf("module: '%s', %p, %"u64"\n", module, tmp, sz);

		tmp += sz;
		if (*tmp == '.')
		{
			tmp += strspn(tmp, ".:");
			sz = strcspn(tmp, ".:");
			strlcpy(type, tmp, sz+1);

			if (carg->log_level > 0)
				printf("type: '%s', %p, %"u64"\n", type, tmp, sz);

			tmp += sz;
			if (!strcmp(type, "requests") || !strcmp(type, "active-requests"))
			{
				type[0] = 0;
				target[0] = 0;
				backend[0] = 0;
			}
			else if (*tmp == '.')
			{
				
				labels_hash_insert_nocache(lbl, "type", type);
				tmp += strspn(tmp, ".:");
				sz = strcspn(tmp, ".:");
				strlcpy(target, tmp, sz+1);

				if (carg->log_level > 0)
					printf("target: '%s', %p, %"u64"\n", target, tmp, sz);

				labels_hash_insert_nocache(lbl, "target", target);
				tmp += sz;

				if (*tmp == '.')
				{
					tmp += strspn(tmp, ".:");
					sz = strcspn(tmp, ".:");
					strlcpy(backend, tmp, sz+1);

					if (carg->log_level > 0)
						printf("backend: '%s', %p, %"u64"\n", backend, tmp, sz);

					labels_hash_insert_nocache(lbl, "type", type);
					if (strcmp(backend, "load"))
					{
						backend[0] = 0;
						tmp += sz;
					}
					else
						labels_hash_insert_nocache(lbl, "backend", backend);
				}
			}
		}

		tmp += strspn(tmp, ".:");
		sz = strcspn(tmp, ".:");

		strlcpy(debug_string, tmp, strcspn(tmp, "\n"));

		if (carg->log_level > 0)
			printf("get metricname from '%s', %p\n", debug_string, tmp);

		strlcpy(metric_name+9, tmp, sz+1);
		metric_name_normalizer(metric_name, sz);
		if (carg->log_level > 0)
			printf("metric_name: '%s', %p, %"u64"\n", metric_name, tmp, sz);
		
		tmp += strcspn(tmp, ":");
		tmp += strspn(tmp, ":");
		val = strtoull(tmp, &tmp, 10);

		metric_add(metric_name, lbl, &val, DATATYPE_UINT, carg);

		tmp += strcspn(tmp, "\n");
		tmp += strspn(tmp, "\n");
		i = tmp - metrics;
	}
}

string* lighttpd_status_mesg(host_aggregator_info *hi, void *arg, void *env, void *proxy_settings)
{
	return string_init_add(gen_http_query(0, hi->query, "?json", hi->host, "alligator", hi->auth, 1, NULL, env, proxy_settings, NULL), 0, 0);
}

string* lighttpd_statistics_mesg(host_aggregator_info *hi, void *arg, void *env, void *proxy_settings)
{
	return string_init_add(gen_http_query(0, hi->query, NULL, hi->host, "alligator", hi->auth, 1, NULL, env, proxy_settings, NULL), 0, 0);
}

void lighttpd_parser_push()
{
	aggregate_context *actx = calloc(1, sizeof(*actx));

	actx->key = strdup("lighttpd_status");
	actx->handlers = 1;
	actx->handler = calloc(1, sizeof(*actx->handler)*actx->handlers);

	actx->handler[0].name = lighttpd_status_handler;
	actx->handler[0].validator = NULL;
	actx->handler[0].mesg_func = lighttpd_status_mesg;
	strlcpy(actx->handler[0].key,"lighttpd_status", 255);

	tommy_hashdyn_insert(ac->aggregate_ctx, &(actx->node), actx, tommy_strhash_u32(0, actx->key));


	actx = calloc(1, sizeof(*actx));

	actx->key = strdup("lighttpd_statistics");
	actx->handlers = 1;
	actx->handler = calloc(1, sizeof(*actx->handler)*actx->handlers);

	actx->handler[0].name = lighttpd_statistics_handler;
	actx->handler[0].validator = NULL;
	actx->handler[0].mesg_func = lighttpd_statistics_mesg;
	strlcpy(actx->handler[0].key,"lighttpd_statistics", 255);

	tommy_hashdyn_insert(ac->aggregate_ctx, &(actx->node), actx, tommy_strhash_u32(0, actx->key));
}
