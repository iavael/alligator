#include "main.h"
extern aconf *ac;

int namespace_struct_compare(const void* arg, const void* obj)
{
        char *s1 = (char*)arg;
        char *s2 = ((namespace_struct*)obj)->key;
        return strcmp(s1, s2);
}

namespace_struct *insert_namespace(char *key)
{
	if (!key)
		return NULL;

	uint32_t key_hash = tommy_strhash_u32(0, key);
	namespace_struct *ns = tommy_hashdyn_search(ac->_namespace, namespace_struct_compare, key, key_hash);
	if (ns)
		return ns;

	ns = calloc(1, sizeof(*ns));
	ns->key = strdup(key);

	tommy_hashdyn_insert(ac->_namespace, &(ns->node), ns, tommy_strhash_u32(0, ns->key));

	metric_tree *metrictree = calloc(1, sizeof(*metrictree));
	expire_tree *expiretree = calloc(1, sizeof(*expiretree));

	tommy_hashdyn* labels_words_hash = malloc(sizeof(*labels_words_hash));
	tommy_hashdyn_init(labels_words_hash);

	sortplan *sort_plan = malloc(sizeof(*sort_plan));
	sort_plan->plan[0] = "__name__";
	sort_plan->size = 1;

	ns->metrictree = metrictree;
	ns->expiretree = expiretree;
	metrictree->labels_words_hash = labels_words_hash;
	metrictree->sort_plan = sort_plan;

	return ns;
}

namespace_struct *get_namespace(char *key)
{
	if (!key)
		return ac->nsdefault;

	uint32_t key_hash = tommy_strhash_u32(0, key);
	namespace_struct *ns = tommy_hashdyn_search(ac->_namespace, namespace_struct_compare, key, key_hash);
	if (!ns)
		ns = ac->nsdefault;

	return ns;
}

namespace_struct *get_namespace_by_carg(context_arg *carg)
{
	if (!carg || !carg->namespace)
		return ac->nsdefault;

	char *key = carg->namespace;

	uint32_t key_hash = tommy_strhash_u32(0, key);
	namespace_struct *ns = tommy_hashdyn_search(ac->_namespace, namespace_struct_compare, key, key_hash);
	if (!ns)
		ns = ac->nsdefault;

	return ns;
}
