void alligator_multiparser(char *buf, size_t len, void (*handler)(char*, size_t, char*, int), char *response);
void clickhouse_system_handler(char *metrics, size_t size, char *instance, int kind);
void redis_handler(char *metrics, size_t size, char *instance, int kind);
void http_proto_handler(char *metrics, size_t size, char *instance, int kind);
