#pragma once
#include "metric/metrictree.h"

typedef struct expire_tree 
{
	struct expire_node *root;
	int64_t count;
} expire_tree;

metric_node* metric_insert (metric_tree *tree, labels_t *labels, int8_t type, void* value, expire_tree *expiretree, int64_t ttl);
void metric_set(metric_node *mnode, int8_t type, void* value, expire_tree *expiretree, int64_t ttl);
