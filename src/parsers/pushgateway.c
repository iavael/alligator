#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "metric/namespace.h"
#define METRIC_LABEL_MAX_SIZE 255

tommy_hashdyn *get_labels_from_url_pushgateway_format(char *uri, size_t uri_size)
{
	if (!uri)
		return 0;

	char *metrics_str  = strstr(uri, "/metrics/");
	if (!metrics_str)
		return 0;

	tommy_hashdyn *lbl = malloc(sizeof(*lbl));
	tommy_hashdyn_init(lbl);
	uint64_t index_get, prev_get;
	char key[METRIC_LABEL_MAX_SIZE];
	char mname[METRIC_LABEL_MAX_SIZE];

	for (index_get = metrics_str - uri + 9; index_get < uri_size;)
	{
		for (; uri[index_get]=='/' && uri[index_get]!=0; ++index_get);
		prev_get = index_get;
		for (; uri[index_get]!='/' && uri[index_get]!=0; ++index_get);
		strlcpy(mname, uri+prev_get, index_get-prev_get+1);

		prev_get = index_get;
		for (; uri[index_get]=='/' && uri[index_get]!=0; ++index_get);
		if (uri[index_get+1] == 0)
			break;

		prev_get = index_get;
		for (; uri[index_get]!='/' && uri[index_get]!=0; ++index_get);
		printf("copy from %s with size %zu\n", uri+prev_get, index_get-prev_get);
		strlcpy(key, uri+prev_get, index_get-prev_get+1);

		printf("insert '%s' with key '%s'\n", mname, key);

		labels_hash_insert_nocache(lbl, mname, key);

		++index_get;
		if (uri[index_get]==0)
			break;
	}

	return lbl;
}