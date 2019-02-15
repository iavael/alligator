#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common/selector.h"
#include "common/url.h"
#include "main.h"
#include "context.h"
#include "common/base64.h"
#include "common/http.h"
#include "common/url.h"
#define CONF_MAXLENSIZE 65535

int context_compare(const void* arg, const void* obj)
{
	char *s1 = (char*)arg;
	char *s2 = ((config_context*)obj)->key;
	printf("compare %s and %s\n", s1, s2);
	return strcmp(s1, s2);
}

mtlen* split_char_to_mtlen(char *str)
{
	mtlen *mt = malloc(sizeof(*mt));
	int64_t i, j, k;
	size_t len = strlen(str);
	printf("buf %s with size %zu\n", str, len);
	for (i=0, j=0, k=0; i<len; i++)
	{
		if ( isspace(str[i]) )
			continue;

		k = strcspn(str+i, ";{} \r\n\t\0");
		int64_t z = i+k;
		if ( str[z] == ';' || str[z] == '{' || str[z] == '}' )
			j++;

		i += k;
		j++;
	}
	mt->st = malloc(sizeof(stlen)*j);
	mt->m = j;

	for (i=0, j=0, k=0; i<len; i++)
	{
		if ( isspace(str[i]) )
			continue;

		k = strcspn(str+i, ";{} \r\n\t\0");
		mt->st[j].s = strndup(str+i, k);
		mt->st[j].l = k;
		int64_t z = i+k;
		if ( str[z] == ';' || str[z] == '{' || str[z] == '}' )
		{
			j++;
			mt->st[j].s = strndup(str+z, 1);
			mt->st[j].l = 1;
		}

		i += k;
		j++;
	}

	return mt;
}

void parse_config(mtlen *mt)
{
	extern aconf *ac;
	int64_t i;
	for (i=0;i<mt->m;i++)
	{
		printf("%"d64": %s %zu %p\n", i, mt->st[i].s, tommy_hashdyn_count(ac->config_ctx), ac->config_ctx);
		config_context *ctx = tommy_hashdyn_search(ac->config_ctx, context_compare, mt->st[i].s, tommy_strhash_u32(0, mt->st[i].s));
		if (ctx)
			ctx->handler(mt, &i);
	}
}

int split_config(char *file)
{
	FILE *fd = fopen(file, "r");
	if (!fd)
		return 0;

	fseek(fd, 0, SEEK_END);
	int64_t fdsize = ftell(fd);
	rewind(fd);

#ifdef _WIN64
	int32_t psize = 10000;
#else
	int32_t psize = 0;
#endif

	char *buf = malloc(fdsize+psize);
	size_t rc = fread(buf, 1, fdsize, fd);
#ifndef _WIN64
	if (rc != fdsize)
	{
		fprintf(stderr, "I/O err read %s\n", file);
		return 0;
	}
#endif
	buf[rc] = 0;
	puts("mtlen config");
	mtlen *mt = split_char_to_mtlen(buf);
	puts("mtlen config end");
	//int64_t i;
	//for (i=0;i<mt->m;i++)
	//{
	//	printf("%"d64": %s\n", i, mt->st[i].s);
	//}
	puts("parse config");
	parse_config(mt);
	puts("parse config end");

	fclose(fd);

	return 1;
}
