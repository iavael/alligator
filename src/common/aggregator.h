#pragma once
#include "dstructures/tommy.h"

typedef struct aggregate_handler {
	void (*name)(char*, size_t, context_arg*);
	int8_t (*validator)(char*, size_t);
	string* (*mesg_func)(host_aggregator_info*, void *arg);
	uint8_t headers_pass;
	char key[255];
} aggregate_handler;

typedef struct aggregate_context
{
	uint64_t handlers;
	aggregate_handler *handler;
	void *data;

	tommy_node node;
	char *key;
} aggregate_context;

int actx_compare(const void* arg, const void* obj);
