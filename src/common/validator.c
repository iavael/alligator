#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int validate_domainname(char *domainname, size_t len)
{
	if ( len > 30 )
		return 0;
	int64_t point = 0, i;
	for ( i=0; i<len; i++ )
	{
		if ( domainname[i] == '.' )
			point = i;
		else if ( isdigit(domainname[i]) )
			{}
		else if ( !isalpha(domainname[i]) )
			return 0;
	}
	if ( point == 0 )
		return 0;
	if ( len-2 <= point )
		return 0;
	return 1;
}

int validate_path(char *path, size_t len)
{
	if ( len > 1000 )
		return 0;
	if ( path[0] != '/' )
		return 0;
	return 1;
}
