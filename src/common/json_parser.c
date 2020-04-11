#include <stdio.h>
#include <stdlib.h>
#include <jansson.h>
#include <string.h>
#include "dstructures/tommyds/tommyds/tommy.h"
#include "metric/namespace.h"
#include "events/context_arg.h"

#define JSON_PARSER_PRINT 0
#define JSON_PARSER_SUM 1
#define JSON_PARSER_AVG 2
#define JSON_PARSER_LABEL 3
#define JSON_PARSER_PLAINPRINT 4
#define OBJSIZE 400
#define MAX_CHARS 100000
#define JSON_PARSER_SEPARATOR "_"

typedef struct pjson_collector
{
	double collector;
	char *key;
	char *label;
	int action;
	tommy_node node;
} pjson_collector;

typedef struct pjson_node
{
	char *key;
	char *by;
	char *exportname;
	char *replace;
	int action;
	size_t size;
	context_arg *carg;
	tommy_hashdyn *hash;
	tommy_node node;
} pjson_node;

typedef struct jsonparse_kv
{
	char *k;
	char *replace;
	char *v;
	struct jsonparse_kv *n;
} jsonparse_kv;

void selector_replace(char *str, size_t size, char from, char to)
{
	int64_t i;
	for (i=0; i<size; i++)
		if ( str[i] == from )
			str[i] = to;
}

int pjson_collector_compare(const void* arg, const void* obj)
{
	char *s1 = (char*)arg;
	char *s2 = ((pjson_collector*)obj)->key;
	return strcmp(s1, s2);
}

int pjson_compare(const void* arg, const void* obj)
{
	char *s1 = (char*)arg;
	char *s2 = ((pjson_node*)obj)->key;
	return strcmp(s1, s2);
}

void jsonparser_collector_foreach(void *funcarg, void* arg)
{
	pjson_collector *collector = arg;
	if (!collector)
		return;
	pjson_node *node = funcarg;
	if (node->action == JSON_PARSER_PRINT)
		return;

	if (node->action == JSON_PARSER_AVG)
		collector->collector /= node->size;

	metric_add_labels(node->key, &collector->collector, DATATYPE_DOUBLE, node ? node->carg : NULL, "key", collector->label);
}

void print_json_aux(json_t *element, char *buf, tommy_hashdyn *hash, jsonparse_kv *kv, pjson_node *node, pjson_collector *collector, int modeplain_nosearch, context_arg *carg);
void print_json_object(json_t *element, char *buf, tommy_hashdyn *hash, jsonparse_kv *kv, pjson_node *node, int modeplain, context_arg *carg);
void print_json_array(json_t *element, char *buf, tommy_hashdyn *hash, jsonparse_kv *kv, context_arg *carg);


void jsonparser_collector_free_foreach(void *funcarg, void* arg)
{
	pjson_collector *collector = arg;
	if (!collector)
		return;

	free(collector->key);
	free(collector->label);
	free(collector);
}
void jsonparser_free_foreach(void* arg)
{
	pjson_node *obj = arg;
	if ( obj->hash )
	{
		tommy_hashdyn_foreach_arg(obj->hash, jsonparser_collector_free_foreach, obj);
		tommy_hashdyn_done(obj->hash);
	}
	free(obj->hash);
	free(obj->key);
	free(obj->exportname);
	if (obj->replace)
		free(obj->replace);
	free(obj);
}

void print_json_aux(json_t *element, char *buf, tommy_hashdyn *hash, jsonparse_kv *kv, pjson_node *node, pjson_collector *collector, int modeplain_nosearch, context_arg *carg)
{
	int jsontype = json_typeof(element);
	jsonparse_kv *cur = kv;
	if (jsontype==JSON_OBJECT)
	{
		int modeplain = 0;
		if (!modeplain_nosearch)
		{
			pjson_node *pjnode = tommy_hashdyn_search(hash, pjson_compare, buf, tommy_strhash_u32(0, buf));
			if (pjnode && pjnode->action == JSON_PARSER_PLAINPRINT)
			{
				if (kv)
				{
					cur->n = malloc(sizeof(*kv));
					cur->n->k = pjnode->by;
					cur->n->v = NULL;
					kv->replace = NULL;
				}
				else
				{
					kv = malloc(sizeof(jsonparse_kv));
					kv->k = pjnode->by;
					kv->v = NULL;
					kv->n = NULL;
					kv->replace = NULL;
				}
				modeplain = 1;
				print_json_object(element, buf, hash, kv, node, modeplain, carg);
				free(kv);
				return;
			}
		}
		print_json_object(element, buf, hash, kv, node, modeplain, carg);
	}
	else if (jsontype==JSON_ARRAY)
		print_json_array(element, buf, hash, kv, carg);
	else if (jsontype==JSON_STRING && !kv)
	{
		pjson_node *pjnode = tommy_hashdyn_search(hash, pjson_compare, buf, tommy_strhash_u32(0, buf));
		if (pjnode && pjnode->action == JSON_PARSER_LABEL)
		{
			int64_t num = 1;
			metric_add_labels(buf, &num, DATATYPE_INT, node ? node->carg : NULL, pjnode->by, (char*)json_string_value(element));
		}

		double af = atof(json_string_value(element));
		if ( !af )
			return;
	}
	else if (jsontype==JSON_STRING && kv)
	{
		double af = atof(json_string_value(element));
		if ( !af )
			return;
		//for (; cur; cur = cur->n) printf("(%s:%s) ", cur->k, cur->v);

		if (cur && collector && collector->action == JSON_PARSER_PRINT)
		{
			tommy_hashdyn *lbl = malloc(sizeof(*lbl));
			tommy_hashdyn_init(lbl);
			char *name;
			char *key;

			for (; cur; cur = cur->n)
			{
				// for replace naming
				if (cur->replace)
					name = cur->replace;
				else
					name = cur->k;
				key = cur->v;

				if (cur->n)
				{
					labels_hash_insert_nocache(lbl, name, key);
				}
				else
				{
					int64_t num = json_integer_value(element);
					metric_add(buf, lbl, &num, DATATYPE_INT, node ? node->carg : NULL);
				}
			}
		}
		else if (collector) collector->collector += json_integer_value(element);
	}
	else if (jsontype == JSON_INTEGER && !kv)
	{
		int64_t num = json_integer_value(element);
		metric_add_auto(buf, &num, DATATYPE_INT, node ? node->carg : carg);
	}
	else if (jsontype == JSON_INTEGER && kv)
	{
		if (collector && collector->action != JSON_PARSER_PRINT) collector->collector += json_integer_value(element);
		else
		{
			tommy_hashdyn *lbl = malloc(sizeof(*lbl));
			tommy_hashdyn_init(lbl);
			char *name;
			char *key;

			for (; cur; cur = cur->n)
			{
				// for replace naming
				if (cur->replace)
					name = cur->replace;
				else
					name = cur->k;
				key = cur->v;

				if (cur->n)
				{
					labels_hash_insert_nocache(lbl, name, key);
				}
				else
				{
					int64_t num = json_integer_value(element);
					metric_add(buf, lbl, &num, DATATYPE_INT, node ? node->carg : NULL);
				}
			}
		}
	}
	else if (jsontype == JSON_REAL && !kv)
	{
		double num = json_real_value(element);
		metric_add_auto(buf, &num, DATATYPE_DOUBLE, node ? node->carg : NULL);
	}
	else if (jsontype == JSON_REAL && kv)
	{
		double num = json_real_value(element);
		//for (; cur; cur = cur->n) printf("(%s:%s) ", cur->k, cur->v);
		if (collector && collector->action != JSON_PARSER_PRINT) collector->collector += num;
		else
		{
			tommy_hashdyn *lbl = malloc(sizeof(*lbl));
			tommy_hashdyn_init(lbl);
			char *name;
			char *key;
			for (; cur; cur = cur->n)
			{
				// for replace naming
				if (cur->replace)
					name = cur->replace;
				else
					name = cur->k;
				key = cur->v;

				if (cur->n)
				{
					labels_hash_insert_nocache(lbl, name, key);
				}
				else
				{
					metric_add(buf, lbl, &num, DATATYPE_DOUBLE, node ? node->carg : NULL);
				}
			}
		}
	}
	else if (jsontype == JSON_TRUE && !kv)
	{
		int64_t num = 1;
		metric_add_auto(buf, &num, DATATYPE_INT, node ? node->carg : NULL);
	}
	else if (jsontype == JSON_TRUE && kv)
	{
		//for (; cur; cur = cur->n) printf("(%s:%s) ", cur->k, cur->v);
		if (collector && collector->action != JSON_PARSER_PRINT) collector->collector += 1;
		else
		{
			tommy_hashdyn *lbl = malloc(sizeof(*lbl));
			tommy_hashdyn_init(lbl);
			char *name;
			char *key;

			for (; cur; cur = cur->n)
			{
				// for replace naming
				if (cur->replace)
					name = cur->replace;
				else
					name = cur->k;
				key = cur->v;

				if (cur->n)
				{
					labels_hash_insert_nocache(lbl, name, key);
				}
				else
				{
					int64_t num = 1;
					metric_add(buf, lbl, &num, DATATYPE_INT, node ? node->carg : NULL);
				}
			}
		}
	}
	else if (jsontype == JSON_FALSE && !kv)
	{
		int64_t num = 0;
		metric_add_auto(buf, &num, DATATYPE_INT, node ? node->carg : NULL);
	}
	else if (jsontype == JSON_FALSE && kv)
	{
		//for (; cur; cur = cur->n) printf("(%s:%s) ", cur->k, cur->v);
	}
}

void print_json_object(json_t *element, char *buf, tommy_hashdyn *hash, jsonparse_kv *kv, pjson_node *node, int modeplain, context_arg *carg)
{
	const char *key;
	json_t *value;

	json_object_foreach(element, key, value)
	{
		//puts("EXPIRE");
		//if ( !node )
		//	return;
		//puts("HIT");

		//char *nby = NULL;

		//if ( kv )
		//{
		//	printf("LL '%s'\n", node->by);
		//	nby = node->by;
		//}
		char skey[OBJSIZE];
		int freecur = 0;
		if (modeplain)
		{
			snprintf(skey, OBJSIZE, "%s", buf);
			jsonparse_kv *cur = kv;
			for (; cur; cur = cur->n)
			{
				cur->v = strdup(key);
				freecur = 1;
			}
		}
		else
			snprintf(skey, OBJSIZE, "%s%s%s", buf, JSON_PARSER_SEPARATOR, key);
		char colkey[OBJSIZE];
		snprintf(colkey, OBJSIZE, "%s", skey);
		jsonparse_kv *cur = kv;
		for (; cur && cur->n; cur = cur->n)
		{
			size_t colsize = strlen(colkey);
			snprintf(colkey+colsize, OBJSIZE-colsize, "(%s:%s)", cur->k, cur->v);
		}

		pjson_collector *collector = NULL;
		if (node && node->hash)
		{
			collector = tommy_hashdyn_search(node->hash, pjson_collector_compare, colkey, tommy_strhash_u32(0, colkey));
			if (!collector)
			{
				collector = malloc(sizeof(*collector));
				collector->collector = 0;
				collector->key = strdup(colkey);
				collector->label = strdup(key);
				collector->action = node->action;
				tommy_hashdyn_insert(node->hash, &(collector->node), collector, tommy_strhash_u32(0, collector->key));
			}
		}
		print_json_aux(value, skey, hash, kv, NULL, collector, modeplain, carg);
		if (freecur)
			free(cur->v);
	}

}

void print_json_array(json_t *element, char *buf, tommy_hashdyn *hash, jsonparse_kv *root_kv, context_arg *carg)
{
	size_t i;
	size_t size = json_array_size(element);

	pjson_node *node = NULL;
	//printf("finding %s\n", buf);
	node = tommy_hashdyn_search(hash, pjson_compare, buf, tommy_strhash_u32(0, buf));
	//printf("result: %p\n", node);
	if (!node)
	{
		return;
	}
	jsonparse_kv *kv = NULL;
	node->size = size;
	for (i = 0; i < size; i++)
	{
		int kvflag = 0;
		jsonparse_kv *cur = root_kv;
		json_t *arr_obj = json_array_get(element, i);
		if ( json_typeof(arr_obj) == JSON_OBJECT )
		{
			kv = NULL;

			//printf("\nffinding '%s'\n", node->by);
			json_t *objobj = json_object_get(arr_obj, node->by);
			if (!objobj)
			{
				return;
			}
			//printf("result objobj %p\n", objobj);
			kv = malloc(sizeof(*kv));
			kv->v = malloc(OBJSIZE);
			kv->n = NULL;
			if (json_typeof(objobj) == JSON_INTEGER)
			{
				kv->k = node->by;
				kv->replace = node->replace;
				snprintf(kv->v, OBJSIZE, "%lld", json_integer_value(objobj));
			}
			else if (json_typeof(objobj) == JSON_STRING)
			{
				kv->k = node->by;
				kv->replace = node->replace;
				strlcpy(kv->v, json_string_value(objobj), OBJSIZE+1);
			}

			kvflag = 1;
			if (cur)
			{
				for (; cur->n; cur=cur->n);
				cur->n = kv;
			}
			else
				root_kv = kv;
		}
		print_json_aux(arr_obj, buf, hash, root_kv, node, NULL, 0, carg);
		if ( kvflag )
		{
			if ( cur )
			{
				free(cur->n->v);
				free(cur->n);
				cur->n = NULL;
			}
			else
			{
				free(root_kv->v);
				free(root_kv);
				root_kv = NULL;
			}
		}
	}
	switch(node->action)
	{
		case JSON_PARSER_SUM:
			tommy_hashdyn_foreach_arg(node->hash, jsonparser_collector_foreach, node);
			break;
		case JSON_PARSER_AVG:
			tommy_hashdyn_foreach_arg(node->hash, jsonparser_collector_foreach, node);
			break;
		default:
			tommy_hashdyn_foreach_arg(node->hash, jsonparser_collector_foreach, node);
			break;
	}
}

uint8_t json_check(const char *text)
{
	json_t *root;
	json_error_t error;

	root = json_loads(text, 0, &error);
	//printf("JSONPARSER: %p\n%s\n", root, text);

	if (root)
	{
		json_decref(root);
		return 1;
	}
	else
	{
		json_decref(root);
		return 0;
	}
}

json_t *load_json(const char *text) {
	json_t *root;
	json_error_t error;

	root = json_loads(text, 0, &error);

	if (root) {
		return root;
	} else {
		fprintf(stderr, "json error on line %d: %s\n", error.line, error.text);
		return (json_t *)0;
	}
}

void json_parse(char *line, tommy_hashdyn *hash, char *name, context_arg *carg)
{
	json_t *root = load_json(line);
	if (!root)
	{
		printf("cannot parse json string:\n%s\n", line);
		FILE *fd = fopen("1", "w");
		fwrite(line, strlen(line), 1, fd);
		fclose(fd);
		return;
	}

	print_json_aux(root, name, hash, NULL, NULL, NULL, 0, carg);
	json_decref(root);
}
void json_parser_entry(char *line, int argc, char **argv, char *name, context_arg *carg)
{
	tommy_hashdyn *hash = malloc(sizeof(*hash));
	tommy_hashdyn_init(hash);
	int i;
	for (i=0; i<argc; i++)
	{
		pjson_node *node = malloc(sizeof(*node));
		node->carg = carg;
		node->replace = NULL;
		if (!strncmp(argv[i], "print::", 7) )
		{
			node->action = JSON_PARSER_PRINT;
			node->exportname = strndup(argv[i]+7, strcspn(argv[i]+7, JSON_PARSER_SEPARATOR));
			node->key = strndup(argv[i]+7, strcspn(argv[i]+7, "("));
			node->by = argv[i]+7 + strcspn(argv[i]+7, "(") +1;
			size_t by_size = strcspn(node->by, ")");
			size_t slash_size = strcspn(node->by, "/");
			if(by_size > slash_size)
			{
				size_t replace_size = by_size-slash_size;
				node->replace = strndup(node->by+slash_size+1, replace_size);
				node->by[slash_size] = 0;
				node->replace[replace_size-1] = 0;
			}
			else
			{
				node->by[by_size] = 0;
			}
			//node->hash = NULL;
			node->hash = malloc(sizeof(tommy_hashdyn));
			tommy_hashdyn_init(node->hash);
		}
		else if (!strncmp(argv[i], "sum::", 5) )
		{
			node->action = JSON_PARSER_SUM;
			node->exportname = strndup(argv[i]+5, strcspn(argv[i]+5, JSON_PARSER_SEPARATOR));
			node->key = strndup(argv[i]+5, strcspn(argv[i]+5, "("));
			node->by = argv[i]+5 + strcspn(argv[i]+5, "(") +1;
			node->by[strcspn(node->by, ")")] = 0;
			node->hash = malloc(sizeof(tommy_hashdyn));
			tommy_hashdyn_init(node->hash);
		}
		else if (!strncmp(argv[i], "avg::", 5) )
		{
			node->action = JSON_PARSER_AVG;
			node->exportname = strndup(argv[i]+5, strcspn(argv[i]+5, JSON_PARSER_SEPARATOR));
			node->key = strndup(argv[i]+5, strcspn(argv[i]+5, "("));
			node->by = argv[i]+5 + strcspn(argv[i]+5, "(") +1;
			node->by[strcspn(node->by, ")")] = 0;
			node->hash = malloc(sizeof(tommy_hashdyn));
			tommy_hashdyn_init(node->hash);
		}
		else if (!strncmp(argv[i], "label::", 7) )
		{
			node->action = JSON_PARSER_LABEL;
			node->exportname = strndup(argv[i]+7, strcspn(argv[i]+7, JSON_PARSER_SEPARATOR));
			node->key = strndup(argv[i]+7, strcspn(argv[i]+7, "("));
			node->by = argv[i]+7 + strcspn(argv[i]+7, "(") +1;
			node->by[strcspn(node->by, ")")] = 0;
			node->hash = malloc(sizeof(tommy_hashdyn));
			tommy_hashdyn_init(node->hash);
		}
		else if (!strncmp(argv[i], "plainprint::", 12) )
		{
			node->action = JSON_PARSER_PLAINPRINT;
			node->exportname = strndup(argv[i]+12, strcspn(argv[i]+12, JSON_PARSER_SEPARATOR));
			node->key = strndup(argv[i]+12, strcspn(argv[i]+12, "("));
			node->by = argv[i]+12 + strcspn(argv[i]+12, "(") +1;
			size_t by_size = strcspn(node->by, ")");
			size_t slash_size = strcspn(node->by, "/");
			if(by_size > slash_size)
			{
				size_t replace_size = by_size-slash_size;
				node->replace = strndup(node->by+slash_size+1, replace_size);
				node->by[slash_size] = 0;
				node->replace[replace_size-1] = 0;
			}
			else
				node->by[by_size] = 0;
			//node->hash = NULL;
			node->hash = malloc(sizeof(tommy_hashdyn));
			tommy_hashdyn_init(node->hash);
		}
		else
		{
			free(node);
			continue;
		}
		tommy_hashdyn_insert(hash, &(node->node), node, tommy_strhash_u32(0, node->key));
	}
	//fclose(fd);

	json_parse(line, hash, name, carg);
	tommy_hashdyn_foreach(hash, jsonparser_free_foreach);
	tommy_hashdyn_done(hash);
	free(hash);
}

int8_t json_validator(char *data)
{
	json_error_t error;
	json_t *root = json_loads(data, 0, &error);
	if (!root)
	{
		fprintf(stderr, "json error on line %d: %s\n", error.line, error.text);
		return 0;
	}
	return 1;
}
