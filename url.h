#include <security/pam_modules.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <curl/curl.h>

#include <security/pam_ext.h>
int enable_short_url(pam_handle_t *pamh, char * smaller_qrcode, char *url);
char * get_short_url(pam_handle_t *pamh, char * short_url_api, char *url);
size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream);