#include "main.h"
#include "metrictree.h"
#include "expiretree.h"
#include "labels.h"
#define EXPIRE_DEFAULT_SECONDS 300

int is_red ( metric_node *node )
{
	return node != NULL && node->color == RED;
}

metric_node *metric_single ( metric_node *root, int dir )
{
	metric_node *save = root->steam[!dir];
 
	root->steam[!dir] = save->steam[dir];
	save->steam[dir] = root;
 
	root->color = RED;
	save->color = BLACK;
 
	return save;
}
 
metric_node *metric_double ( metric_node *root, int dir )
{
	root->steam[!dir] = metric_single ( root->steam[!dir], !dir );
	return metric_single ( root, dir );
}

metric_node *make_node (metric_tree *tree, labels_t *labels, int8_t type, void *value, expire_tree *expiretree)
{
	metric_node *rn = malloc ( sizeof *rn );
  
	if ( rn != NULL )
	{
		labels_cache_fill(labels, tree);
		rn->labels = labels;
		//print_labels(labels);
		rn->color = RED;
		rn->steam[LEFT] = NULL;
		rn->steam[RIGHT] = NULL;
		rn->type = type;

		if (type == DATATYPE_INT)
			rn->i = *(int64_t*)value;
		else if (type == DATATYPE_UINT)
			rn->u = *(uint64_t*)value;
		else if (type == DATATYPE_DOUBLE)
			rn->d = *(double*)value;
		else if (type == DATATYPE_STRING)
			rn->s = value;
		else if (type == DATATYPE_LIST_UINT)
		{
			rn->list = calloc(METRIC_STORAGE_BUFFER_DEFAULT, sizeof(metric_list));
			rn->list[0].u = *(uint64_t*)value;
			rn->index_element_list = 1;
		}
		else if (type == DATATYPE_LIST_INT)
		{
			rn->list = calloc(METRIC_STORAGE_BUFFER_DEFAULT, sizeof(metric_list));
			rn->list[0].i = *(int64_t*)value;
			rn->index_element_list = 1;
		}
		else if (type == DATATYPE_LIST_DOUBLE)
		{
			rn->list = calloc(METRIC_STORAGE_BUFFER_DEFAULT, sizeof(metric_list));
			rn->list[0].d = *(double*)value;
			rn->index_element_list = 1;
		}
		else if (type == DATATYPE_LIST_STRING)
		{
			rn->list = calloc(METRIC_STORAGE_BUFFER_DEFAULT, sizeof(metric_list));
			rn->list[0].s = value;
			rn->index_element_list = 1;
		}
	}
	return rn;
}


metric_node* metric_insert (metric_tree *tree, labels_t *labels, int8_t type, void* value, expire_tree *expiretree)
{
	metric_node *ret = NULL;
	if ( tree->root == NULL )
	{
	        tree->root = ret = make_node(tree, labels, type, value, expiretree);
		tree->count++;
		if (tree->root == NULL)
			return NULL;
	}
	else
	{
		metric_node head = {{0}};
		metric_node *g, *t;
		metric_node *p, *q;
		int dir = 0, last = 0;
	
		t = &head;
		g = p = NULL;
		q = t->steam[RIGHT] = tree->root;
		int flag = 0;
	
		for (;;) 
		{
			if ( q == NULL )
			{
				p->steam[dir] = q = ret = make_node(tree, labels, type, value, expiretree);
				tree->count++;
				flag = 1;
				if ( q == NULL )
					return NULL;
			}
			else if ( is_red ( q->steam[LEFT] ) && is_red ( q->steam[RIGHT] ) ) 
			{
				q->color = RED;
				q->steam[LEFT]->color = BLACK;
				q->steam[RIGHT]->color = BLACK;
			}
			else
			{
			}
			if ( is_red ( q ) && is_red ( p ) ) 
			{
				int dir2 = t->steam[RIGHT] == g;
	
				if ( q == p->steam[last] )
			       		t->steam[dir2] = metric_single ( g, !last );
				else
			       		t->steam[dir2] = metric_double ( g, !last );
			}
			if (flag)
				break;
	
			last = dir;
			dir = labels_cmp(labels, q->labels) > 0;
	
			if ( g != NULL )
				t = g;
			g = p, p = q;
			q = q->steam[dir];
		}
		tree->root = head.steam[RIGHT];
	}
	tree->root->color = BLACK;

	r_time time = setrtime();
	expire_insert(expiretree, time.sec+EXPIRE_DEFAULT_SECONDS, ret);
	return ret;
}

void metric_delete (metric_tree *tree, labels_t *labels, expire_tree *expiretree)
{
	if ( tree->root != NULL ) 
	{
		metric_node head = {{0}};
		metric_node *q, *p, *g;
		metric_node *f = NULL;
		int dir = 1;
 
		q = &head;
		g = p = NULL;
		q->steam[RIGHT] = tree->root;
 
		while ( q->steam[dir] != NULL )
		{
			int last = dir;
 
			g = p, p = q;
			q = q->steam[dir];
			dir = labels_cmp(labels, q->labels) > 0;

			if ( !labels_cmp(q->labels, labels) )
				f = q;
 
			if ( !is_red ( q ) && !is_red ( q->steam[dir] ) ) {
				if ( is_red ( q->steam[!dir] ) )
					p = p->steam[last] = metric_single ( q, dir );
				else if ( !is_red ( q->steam[!dir] ) ) {
					metric_node *s = p->steam[!last];
 

					if ( s != NULL ) {
						if ( !is_red ( s->steam[!last] ) && !is_red ( s->steam[last] ) ) {
							p->color = BLACK;
							s->color = RED;
							q->color = RED;
						}
						else {
							int dir2 = g->steam[RIGHT] == p;
 
							if ( is_red ( s->steam[last] ) )
								g->steam[dir2] = metric_double ( p, last );
							else if ( is_red ( s->steam[!last] ) )
								g->steam[dir2] = metric_single ( p, last );
 
							q->color = g->steam[dir2]->color = RED;
							g->steam[dir2]->steam[LEFT]->color = BLACK;
							g->steam[dir2]->steam[RIGHT]->color = BLACK;
						}
					}
				}
			}
		}
 
		if ( f != NULL )
		{
			expire_delete(expiretree, q->expire_node->key, q);
			tree->count--;
			labels_free(f->labels, tree);
			//free(f->labels);
			f->labels = q->labels;
			p->steam[p->steam[RIGHT] == q] = q->steam[q->steam[LEFT] == NULL];
			free ( q );
		}
 
		tree->root = head.steam[RIGHT];
		if ( tree->root != NULL )
			tree->root->color = BLACK;
	}
}

void metric_set(metric_node *mnode, int8_t type, void* value, expire_tree *expiretree)
{
	if (type == DATATYPE_INT)
		mnode->i = *(int64_t*)value;
	else if (type == DATATYPE_UINT)
		mnode->u = *(uint64_t*)value;
	else if (type == DATATYPE_DOUBLE)
		mnode->d = *(double*)value;
	else if (type == DATATYPE_STRING)
		mnode->s = value;
	else if (type == DATATYPE_LIST_UINT)
	{
		mnode->list = calloc(METRIC_STORAGE_BUFFER_DEFAULT, sizeof(metric_list));
		mnode->list[mnode->index_element_list++].u = *(uint64_t*)value;
	}
	else if (type == DATATYPE_LIST_INT)
	{
		mnode->list = calloc(METRIC_STORAGE_BUFFER_DEFAULT, sizeof(metric_list));
		mnode->list[mnode->index_element_list++].i = *(int64_t*)value;
	}
	else if (type == DATATYPE_LIST_DOUBLE)
	{
		mnode->list = calloc(METRIC_STORAGE_BUFFER_DEFAULT, sizeof(metric_list));
		mnode->list[mnode->index_element_list++].d = *(double*)value;
	}
	else if (type == DATATYPE_LIST_STRING)
	{
		mnode->list = calloc(METRIC_STORAGE_BUFFER_DEFAULT, sizeof(metric_list));
		mnode->list[mnode->index_element_list++].s = value;
	}

	r_time time = setrtime();
	expire_delete(expiretree, mnode->expire_node->key, mnode);
	expire_insert(expiretree, time.sec+EXPIRE_DEFAULT_SECONDS, mnode);
}

void metrictree_show(metric_node *x)
{
	labels_print(x->labels, 0);
	puts("");
	if ( x->steam[LEFT] )
		metrictree_show(x->steam[LEFT]);
	if ( x->steam[RIGHT] )
		metrictree_show(x->steam[RIGHT]);
}

void metric_show ( metric_tree *tree )
{
	if ( tree && tree->root )
		metrictree_show(tree->root);
}


void metric_build (char *namespace);
uint64_t metrictree_build(metric_node *x, uint64_t l)
{
	l++;

	if ( x->steam[LEFT] )
		l = metrictree_build(x->steam[LEFT], l++);

	labels_print(x->labels, l);
	puts("");

	if ( x->steam[RIGHT] )
		l = metrictree_build(x->steam[RIGHT], l++);
	l--;

	return l;
}

void metric_build (char *namespace)
{
	extern aconf *ac;
	metric_tree *tree = ac->nsdefault->metrictree;
	if (!namespace)
		tree = ac->nsdefault->metrictree;
	else
		return;

	if ( tree && tree->root )
	{
		metrictree_build(tree->root, 0);
	}
}

void metrictree_str_build(metric_node *x, string *str)
{
	if ( x->steam[LEFT] )
		metrictree_str_build(x->steam[LEFT], str);

	labels_t *labels = x->labels;
	string_cat(str, labels->key, labels->key_len);

	int quotes_open = 0;
	int quotes_close = 0;
	labels = labels->next;
	while (labels)
	{
		if (!labels->key)
		{
			labels = labels->next;
			continue;
		}

		if (!quotes_open)
		{
			string_cat(str, " {", 2);
			quotes_open = 1;
		}
		else
			string_cat(str, ", ", 2);
		string_cat(str, labels->name, labels->name_len);
		string_cat(str, "=\"", 2);
		string_cat(str, labels->key, labels->key_len);
		string_cat(str, "\"", 1);

		labels = labels->next;
	}

	if (quotes_open && !quotes_close)
		string_cat(str, "}", 1);

	string_cat(str, " ", 1);

	int8_t type = x->type;
	if (type == DATATYPE_INT)
		string_int(str, x->i);
	else if (type == DATATYPE_UINT)
		string_uint(str, x->u);
	else if (type == DATATYPE_DOUBLE)
		string_double(str, x->d);
	else if (type == DATATYPE_STRING)
		string_cat(str, x->s, strlen(x->s));
	string_cat(str, "\n", 1);

	if ( x->steam[RIGHT] )
		metrictree_str_build(x->steam[RIGHT], str);
}

void metric_str_build (char *namespace, string *str)
{
	extern aconf *ac;
	namespace_struct *ns;

	if (!namespace)
		ns = ac->nsdefault;
	else // add support namespaces
		return;
	metric_tree *tree = ns->metrictree;

	if ( tree && tree->root )
	{
		metrictree_str_build(tree->root, str);
	}
}

metric_tree* metrictree_get ( metric_node *x, labels_t* labels )
{
	if (  x && labels_cmp(labels, x->labels) > 0 )
		return metrictree_get(x->steam[LEFT], labels);
	else if ( x && labels_cmp(labels, x->labels) < 0 )
		return metrictree_get(x->steam[RIGHT], labels);
	else if ( x && !labels_cmp(x->labels, labels) )
	{
		return metrictree_get(x->steam[RIGHT], labels);
	}
	else
		return NULL;
}

metric_node* metric_find ( metric_tree *tree, labels_t* labels )
{
	if ( !tree || !(tree->root) )
		return NULL;
	metric_node *x = tree->root;
	while ( x )
	{
		//if ( x->labels > labels )
		int rc1 = labels_cmp(x->labels, labels);
		//int rc2 = labels_cmp(labels, x->labels);
		if ( rc1 > 0 )
			x = x->steam[LEFT];
		//else if ( x->labels < labels )
		else if ( rc1 < 0 )
			x = x->steam[RIGHT];
		//else if ( !labels_cmp(labels, x->labels) )
		else if ( !rc1 )
		{
			return x;
		}
		else
		{
			return NULL;
		}
	}
	return NULL;
}

void metrictree_free(metric_node *x)
{
	if ( x->steam[LEFT] )
		metrictree_free(x->steam[LEFT]);
	if ( x->steam[RIGHT] )
		metrictree_free(x->steam[RIGHT]);
	free (x);
}

void metric_free ( metric_tree *tree )
{
	if ( tree && tree->root )
		metrictree_free(tree->root);
}

// examples for use
///	sort_plan = malloc(sizeof(*sort_plan));
///	sort_plan->plan[0] = "__name__";
///	sort_plan->size = 1;
///	tree = calloc(1, sizeof(*tree));
///	expiretree = calloc(1, sizeof(*tree));
///	
///	labels_words_hash = malloc(sizeof(*labels_words_hash));
///	tommy_hashdyn_init(labels_words_hash);
///
///	expire_purge(expiretree, 100000000000000);
///	//metric_build(tree);
///	//metric_free(tree);
///	//expire_insert(expiretree, 1, 0);
///	//expire_build(expiretree);
///	//metric_build(tree);
///
///	string *str = string_init(10000000);
///	metric_str_build(tree, str);
///	printf("%s\n", str->s);
///
///	expire_purge(expiretree, 100000000);
///	expire_free(expiretree);
