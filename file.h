#include <stdio.h>
#include <security/pam_modules.h>
#include <syslog.h>
#include <security/pam_ext.h>
#include <string.h>

#define ORD_LEN 50
#define URL_LEN 300
size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream);
int write_url(char * key,char *url,char *appid);
int read_url(char * key,char *url,char *appid);