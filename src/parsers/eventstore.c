#include <stdio.h>
#include <inttypes.h>
#include "common/selector.h"
#include "metric/namespace.h"
#include "events/context_arg.h"
void eventstore_stats_handler(char *metrics, size_t size, context_arg *carg)
{
	char **parsestring = malloc(sizeof(void*)*1);
	parsestring[0] = strdup("plainprint::eventstore_es_queue(name)");
	json_parser_entry(metrics, 1, parsestring, "eventstore", carg);
	free(parsestring[0]);
	free(parsestring);
}

void eventstore_projections_handler(char *metrics, size_t size, context_arg *carg)
{
	char **parsestring = malloc(sizeof(void*)*1);
	parsestring[0] = strdup("print::eventstore_projections(name)");
	json_parser_entry(metrics, 1, parsestring, "eventstore", carg);
	free(parsestring[0]);
	free(parsestring);
}

void eventstore_info_handler(char *metrics, size_t size, context_arg *carg)
{
	char **parsestring = malloc(sizeof(void*)*1);
	parsestring[0] = strdup("label::eventstore_state(state)");
	json_parser_entry(metrics, 1, parsestring, "eventstore", carg);
	free(parsestring[0]);
	free(parsestring);
}
