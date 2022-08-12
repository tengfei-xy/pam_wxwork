#include <security/pam_ext.h>

extern char * get_wxwork_qrcode(char * url);
char * get_key(char * url);
extern char *get_wxwork_key(pam_handle_t *pamh,char *url);
extern char *get_wxwork_qrcode(char *key);
extern char * wait_auth_wxwork_qrcode(char * key,char * redirect_uri,char * appid);
char * create_redirect_url(char * url,char * auth_code,char * appid);
size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream);
int req_auth_server(pam_handle_t *pamh,char *url);
