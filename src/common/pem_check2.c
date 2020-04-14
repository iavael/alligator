#include <limits.h>
#include <printf.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "mbedtls/x509_crt.h"
#include "common/rtime.h"
#include "main.h"
uint64_t get_timestamp_from_mbedtls(mbedtls_x509_time mbed_tm)
{
	struct tm gmtime_time;
	memset(&gmtime_time, 0, sizeof(gmtime_time));
	gmtime_time.tm_year = mbed_tm.year - 1900;
	gmtime_time.tm_mon = mbed_tm.mon - 1;
	gmtime_time.tm_mday = mbed_tm.day;
	gmtime_time.tm_hour = mbed_tm.hour;
	gmtime_time.tm_min = mbed_tm.min;
	gmtime_time.tm_sec = mbed_tm.sec;

	char buf[255];
	uint64_t timestamp;
	strftime(buf, 255, "%s", &gmtime_time);
	timestamp = strtoull(buf, NULL, 10);
	

	return timestamp;
}

char *read_file(char *name)
{
	char *pem_cert = malloc(2048);
	FILE *fd = fopen(name, "r");
	if (!fd)
		return 0;

	fread(pem_cert, 1, 2048, fd);
	fclose(fd);

	return pem_cert;
}

void parse_cert_info(mbedtls_x509_crt *cert_ctx, char *cert)
{
	if (!cert_ctx)
		return;

	extern aconf *ac;

	char dn_subject[1000];
	int dn_subject_size = mbedtls_x509_dn_gets( dn_subject, 1000, &cert_ctx->subject );
	char country_name[1000];
	char county[1000];
	char organization_name[1000];
	char organization_unit[1000];
	char common_name[1000];
	tommy_hashdyn *lbl = malloc(sizeof(*lbl));
	tommy_hashdyn_init(lbl);
	labels_hash_insert_nocache(lbl, "cert", cert);
	for(int i=0; i<dn_subject_size;)
	{
		if (!strncmp(dn_subject+i, "C=", 2))
		{
			i += strcspn(dn_subject+i, "= ");
			i += strspn(dn_subject+i, "= ");
			int size = strcspn(dn_subject+i, ", ");
			strlcpy(country_name, dn_subject+i, size+1);
			if (ac->log_level > 2)
				printf("cert: %s, country=%s\n", cert, country_name);
			labels_hash_insert_nocache(lbl, "country", country_name);
			i += size;
		}
		else if (!strncmp(dn_subject+i, "ST=", 3))
		{
			i += strcspn(dn_subject+i, "= ");
			i += strspn(dn_subject+i, "= ");
			int size = strcspn(dn_subject+i, ", ");
			strlcpy(county, dn_subject+i, size+1);
			if (ac->log_level > 2)
				printf("cert: %s, county=%s\n", cert, county);
			labels_hash_insert_nocache(lbl, "county", county);
			i += size;
		}
		else if (!strncmp(dn_subject+i, "O=", 2))
		{
			i += strcspn(dn_subject+i, "= ");
			i += strspn(dn_subject+i, "= ");
			int size = strcspn(dn_subject+i, ", ");
			strlcpy(organization_name, dn_subject+i, size+1);
			if (ac->log_level > 2)
				printf("cert: %s, organization_name=%s\n", cert, organization_name);
			labels_hash_insert_nocache(lbl, "organization_name", organization_name);
			i += size;
		}
		else if (!strncmp(dn_subject+i, "OU=", 3))
		{
			i += strcspn(dn_subject+i, "= ");
			i += strspn(dn_subject+i, "= ");
			int size = strcspn(dn_subject+i, ", ");
			strlcpy(organization_unit, dn_subject+i, size+1);
			if (ac->log_level > 2)
				printf("cert: %s, organization_unit=%s\n", cert, organization_unit);
			labels_hash_insert_nocache(lbl, "organization_unit", organization_unit);
			i += size;
		}
		else if (!strncmp(dn_subject+i, "CN=", 3))
		{
			i += strcspn(dn_subject+i, "= ");
			i += strspn(dn_subject+i, "= ");
			int size = strcspn(dn_subject+i, ", ");
			strlcpy(common_name, dn_subject+i, size+1);
			if (ac->log_level > 2)
				printf("cert: %s, common_name=%s\n", cert, common_name);
			labels_hash_insert_nocache(lbl, "common_name", common_name);
			i += size;
		}
		i += strcspn(dn_subject+i, ",");
		i += strspn(dn_subject+i, ", \t");
	}
	
	char dn_issuer[1000];
	mbedtls_x509_dn_gets(dn_issuer, 1000, &cert_ctx->issuer);
	labels_hash_insert_nocache(lbl, "issuer", dn_issuer);
	if (ac->log_level > 2)
		printf("cert: %s, issuer: %s\n", cert, dn_issuer);

	char serial[100];
	mbedtls_x509_serial_gets(serial, 100, &cert_ctx->serial);
	if (ac->log_level > 2)
		printf("cert: %s, serial: %s\n", cert, serial);
	labels_hash_insert_nocache(lbl, "serial", serial);

	int64_t valid_from = get_timestamp_from_mbedtls(cert_ctx->valid_from);
	int64_t valid_to = get_timestamp_from_mbedtls(cert_ctx->valid_to);
	r_time now = setrtime();

	int64_t expdays =  (valid_to-now.sec)/86400; 
	if (ac->log_level > 2)
	{
		printf("cert: %s, certsubject: %s\n", cert, dn_subject);
		printf("cert: %s, complete for: %u.\n", cert, now.sec);
		printf("cert: %s, valid from: %"d64".\n", cert, valid_from);
		printf("cert: %s, %"d64" exp\n", cert, expdays);
		printf("cert: %s, version: %d\n", cert, cert_ctx->version);
	}
	tommy_hashdyn *notafter_lbl = labels_dup(lbl);
	tommy_hashdyn *expiredays_lbl = labels_dup(lbl);
	metric_add("x509_cert_not_before", lbl, &valid_from, DATATYPE_INT, NULL);
	metric_add("x509_cert_not_after", notafter_lbl, &valid_to, DATATYPE_INT, NULL);
	metric_add("x509_cert_expire_days", expiredays_lbl, &expdays, DATATYPE_INT, NULL);
}

void pem_check_cert(char *pem_cert, size_t cert_size, void *data, char *filename)
{
	mbedtls_x509_crt cert;
	mbedtls_x509_crt_init(&cert);


	int ret = mbedtls_x509_crt_parse(&cert, (const unsigned char *)pem_cert, cert_size+1);
	if (ret)
	{
		//printf("error parse certificate: '%s' with size %zu\n", pem_cert, strlen(pem_cert)+1);
		free(data);
		return;
	}

	parse_cert_info(&cert, filename);

	free(filename);
	mbedtls_x509_crt_free(&cert);
	free(data);
}

void fs_cert_check(char *fname, char *match)
{
	read_from_file(fname, 0, pem_check_cert, NULL);
}

//int min(int a, int b) { return (a < b)? a : b;  }
int get_file_count_read(char *path, char *match)
{
	uv_fs_t readdir_req;

	uv_fs_opendir(NULL, &readdir_req, path, NULL);

	uv_dirent_t dirents[1024];
	uv_dir_t* rdir = readdir_req.ptr;
	rdir->dirents = dirents;
	rdir->nentries = 1024;//min(1024,chunklen);

	int acc = 0;

	char fullname[1024];
	strcpy(fullname, path);
	char * filebase = fullname+strlen(path)+1;
	*(filebase-1)='/';

	for(;;)
	{
		int r = uv_fs_readdir(uv_default_loop(), &readdir_req, readdir_req.ptr, NULL);
		if (r <= 0)
			break;

		for (int i=0; i<r; i++)
		{
			if (dirents[i].type == UV_DIRENT_DIR)
			{
				strcpy(filebase, dirents[i].name);
				acc += get_file_count_read(fullname, match);
			}
			else if (dirents[i].type == UV_DIRENT_FILE)
			{
				//printf("\t%s/%s\n", path, dirents[i].name);
				acc += 1;
				fs_cert_check(path, match);
			}
		}
	}

	uv_fs_closedir(NULL, &readdir_req, readdir_req.ptr, NULL);
	return acc;
}

void tls_fs_recurse(char *dir, char *match)
{
	//printf("dir: %s, match: %s\n", dir, match);
	get_file_count_read(dir, match);
}
