#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>

#include <security/pam_modules.h>
#include <security/pam_ext.h>

#include "conf.h"

void set_wxwork_config(struct wxwork_config *wc, char *key, char *value)
{
    if (strstr(key,WXWORK_APPID)!=NULL){
        strcpy(wc->appid = (char *)malloc(sizeof (char)*(strlen(value))),value);
    }else if (strstr(key,WXWORK_AGENTID)!=NULL){
        strcpy(wc->agentid = (char *)malloc(sizeof (char)*(strlen(value))),value);
    }else if (strstr(key,WXWORK_REDIRECT_URI)!=NULL){
        strcpy(wc->redirect_uri = (char *)malloc(sizeof (char)*(strlen(value))),value);
    }else if (strstr(key,WXWORK_AUTH_VALUE)!=NULL){
        strcpy(wc->auth_value = (char *)malloc(sizeof (char)*(strlen(value))),value);
    }else if (strstr(key,WXWORK_LOGIN_URL)!=NULL){
        strcpy(wc->login_url = (char *)malloc(sizeof (char)*(strlen(value))),value);
    }
}
int get_wxwork_config(pam_handle_t *pamh, struct wxwork_config *wc, const char *file)
{
    FILE *f = fopen(file, "r");
    if (f == NULL)
    {
        pam_syslog(pamh, LOG_ERR, "failed to open %s", file);
        return -1;
    }
    char line[1024];
    char *key;
    char *value;

    key = (char *)malloc(0);
    value = (char *)malloc(0);

    while (fgets(line, 1024, f) != NULL)
    {
        if (line[0] < 99 || line[0] > 122)
            continue;
        if (strstr(line, "=") == NULL)
        {
            pam_syslog(pamh, LOG_ERR, "An incorrect configuration was found:%s", line);
            return -1;
        }
        key = realloc(key, sizeof(char) * (strlen(line) + 1));
        value = realloc(value, sizeof(char) * (strlen(line) + 1));

        for (int i = 0, j = 0, d = 0, k = 0; i < strlen(line) + 1; i++)
        {
            switch (d)
            {
            case 0:
                switch (line[i])
                {
                case ' ':
                    break;
                case '=':
                    d = 1;
                    key[k++] = '\0';
                    break;
                default:
                    key[k++] = line[i];
                }
                break;
            case 1:
                switch (line[i])
                {
                case ' ':
                    break;
                case '"':
                    d = 2;
                    i++;
                    break;
                default:
                    d = 2;
                }
            case 2:
                switch (line[i])
                {
                case ' ':
                    break;
                case '\n':
                    d = 3;
                    value[j] = '\0';
                    break;
                case '"':
                    d = 3;
                    value[j++] = '\0';
                    break;
                default:
                    value[j++] = line[i];
                    break;
                }
            }
        }
        set_wxwork_config(wc,key,value);
    }

    if (wc->login_url==NULL)
        strcpy(wc->login_url = (char *)malloc(sizeof(char) * (strlen(WXWORK_URL))), WXWORK_URL);
    free(key);
    free(value);
    return 0;
}
