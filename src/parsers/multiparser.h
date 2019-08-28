#define MAX_RESPONSE_SIZE 6553500
#include "events/context_arg.h"
void alligator_multiparser(char *buf, size_t slen, void (*handler)(char*, size_t, context_arg*), string *response, context_arg *carg);
void redis_handler(char *metrics, size_t size, context_arg *carg);
void sentinel_handler(char *metrics, size_t size, context_arg *carg);
void aerospike_statistics_handler(char *metrics, size_t size, context_arg *carg);
void aerospike_get_namespaces_handler(char *metrics, size_t size, context_arg *carg);
void aerospike_status_handler(char *metrics, size_t size, context_arg *carg);
void aerospike_namespace_handler(char *metrics, size_t size, context_arg *carg);
void http_proto_handler(char *metrics, size_t size, context_arg *carg);
char* http_proto_proxer(char *metrics, size_t size, context_arg *carg);
void zookeeper_isro_handler(char *metrics, size_t size, context_arg *carg);
void zookeeper_wchs_handler(char *metrics, size_t size, context_arg *carg);
void zookeeper_mntr_handler(char *metrics, size_t size, context_arg *carg);

void clickhouse_system_handler(char *metrics, size_t size, context_arg *carg);
void clickhouse_columns_handler(char *metrics, size_t size, context_arg *carg);
void clickhouse_dictionary_handler(char *metrics, size_t size, context_arg *carg);
void clickhouse_merges_handler(char *metrics, size_t size, context_arg *carg);
void clickhouse_replicas_handler(char *metrics, size_t size, context_arg *carg);
void beanstalkd_handler(char *metrics, size_t size, context_arg *carg);
void memcached_handler(char *metrics, size_t size, context_arg *carg);
void mssql_handler(char *metrics, size_t size, context_arg *carg);
void gearmand_handler(char *metrics, size_t size, context_arg *carg);
void haproxy_info_handler(char *metrics, size_t size, context_arg *carg);
void haproxy_pools_handler(char *metrics, size_t size, context_arg *carg);
void haproxy_stat_handler(char *metrics, size_t size, context_arg *carg);
void haproxy_sess_handler(char *metrics, size_t size, context_arg *carg);
void haproxy_table_handler(char *metrics, size_t size, context_arg *carg);
void uwsgi_handler(char *metrics, size_t size, context_arg *carg);
void php_fpm_handler(char *metrics, size_t size, context_arg *carg);
void nats_varz_handler(char *metrics, size_t size, context_arg *carg);
void nats_subsz_handler(char *metrics, size_t size, context_arg *carg);
void nats_connz_handler(char *metrics, size_t size, context_arg *carg);
void nats_routez_handler(char *metrics, size_t size, context_arg *carg);
void riak_handler(char *metrics, size_t size, context_arg *carg);
void rabbitmq_overview_handler(char *metrics, size_t size, context_arg *carg);
void rabbitmq_nodes_handler(char *metrics, size_t size, context_arg *carg);
void rabbitmq_connections_handler(char *metrics, size_t size, context_arg *carg);
void rabbitmq_channels_handler(char *metrics, size_t size, context_arg *carg);
void rabbitmq_exchanges_handler(char *metrics, size_t size, context_arg *carg);
void rabbitmq_queues_handler(char *metrics, size_t size, context_arg *carg);
void rabbitmq_vhosts_handler(char *metrics, size_t size, context_arg *carg);
void elasticsearch_nodes_handler(char *metrics, size_t size, context_arg *carg);
void elasticsearch_cluster_handler(char *metrics, size_t size, context_arg *carg);
void elasticsearch_health_handler(char *metrics, size_t size, context_arg *carg);
void elasticsearch_index_handler(char *metrics, size_t size, context_arg *carg);
void elasticsearch_settings_handler(char *metrics, size_t size, context_arg *carg);
void eventstore_stats_handler(char *metrics, size_t size, context_arg *carg);
void eventstore_projections_handler(char *metrics, size_t size, context_arg *carg);
void eventstore_info_handler(char *metrics, size_t size, context_arg *carg);
void flower_handler(char *metrics, size_t size, context_arg *carg);
void powerdns_handler(char *metrics, size_t size, context_arg *carg);
void opentsdb_handler(char *metrics, size_t size, context_arg *carg);
void log_handler(char *metrics, size_t size, context_arg *carg);
void nginx_upstream_check_handler(char *metrics, size_t size, context_arg *carg);
void monit_handler(char *metrics, size_t size, context_arg *carg);
void dummy_handler(char *metrics, size_t size, context_arg *carg);
void rsyslog_impstats_handler(char *metrics, size_t size, context_arg *carg);
