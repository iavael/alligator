#include <stdio.h>
#include "metric/namespace.h"
#include "events/context_arg.h"
#include "main.h"

#define SYSLOGNG_NAME_SIZE 1000

void syslog_ng_handler(char *metrics, size_t size, context_arg *carg)
{
	extern aconf *ac;
	char *cur = metrics;
	uint64_t msize;
	char source_name[SYSLOGNG_NAME_SIZE];
	char source_id[SYSLOGNG_NAME_SIZE];
	char source_instance[SYSLOGNG_NAME_SIZE];
	char state[SYSLOGNG_NAME_SIZE];
	char type[SYSLOGNG_NAME_SIZE];
	uint64_t value;
	while (cur-metrics < size)
	{
		cur += strcspn(cur, "\n");
		++cur;

		msize = strcspn(cur, ";");
		strlcpy(source_name, cur, msize+1);
		if (strstr(source_name, ".\n"))
			return;
		cur += msize;
		++cur;

		msize = strcspn(cur, ";");
		strlcpy(source_id, cur, msize+1);
		cur += msize;
		++cur;

		msize = strcspn(cur, ";");
		strlcpy(source_instance, cur, msize+1);
		cur += msize;
		++cur;

		msize = strcspn(cur, ";");
		strlcpy(state, cur, msize+1);
		cur += msize;
		++cur;

		msize = strcspn(cur, ";");
		strlcpy(type, cur, msize+1);
		cur += msize;
		++cur;

		value = strtoull(cur, &cur, 10);

		if (ac->log_level > 2)
			printf("source_name: '%s', source_id: '%s', source_instance: '%s', state: '%s', type: '%s' ::: %"PRIu64"\n", source_name, source_id, source_instance, state, type, value);
		if (!*source_id)
			metric_add_labels4("syslogng_stats", &value, DATATYPE_UINT, carg, "source_name", source_name, "source_instance", source_instance, "state", state, "type", type);
		else if (!*source_instance)
			metric_add_labels4("syslogng_stats", &value, DATATYPE_UINT, carg, "source_name", source_name, "source_id", source_id, "state", state, "type", type);
		else
			metric_add_labels5("syslogng_stats", &value, DATATYPE_UINT, carg, "source_name", source_name, "source_id", source_id, "source_instance", source_instance, "state", state, "type", type);
	}
}
