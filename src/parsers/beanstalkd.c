#include <stdio.h>
#include <string.h>
#include "common/selector.h"
#include "dstructures/metric.h"
void beanstalkd_handler(char *metrics, size_t size, char *instance, int kind)
{
	selector_split_metric(metrics, size, "\n", 1, ": ", 2, "beanstalkd_", 6, NULL, 0);
}