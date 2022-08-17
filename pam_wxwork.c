#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <curl/curl.h>
#include <unistd.h>
#include <security/pam_modules.h>
#include <security/pam_ext.h>
#include <sys/utsname.h>
#include "wxwork.h"
#include "qrcode.h"
#include "conf.h"

#ifdef __GNUC__
#define UNUSED __attribute__((unused))
#else
#define UNUSED
#endif

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 255
#endif
#define CONF_LINE_MAX 100

int auth_main(pam_handle_t *pamh, int flags, int argc, const char **argv);
int get_pam_item(pam_handle_t *pamh, char *target, char *line);

// 用户认证的PAM函数，在auth字段中使用
int pam_sm_authenticate(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
    return auth_main(pamh, flags, argc, argv);
}
int pam_sm_acct_mgmt(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
    return auth_main(pamh, flags, argc, argv);
}
int pam_sm_setcred(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
    return PAM_SUCCESS;
}
// int pam_sm_close_session (pam_handle_t *pamh , int flags , int argc , const char **argv )
// {
//   return PAM_SUCCESS;
// }

int get_pam_item(pam_handle_t *pamh, char *target, char *line)
{

    struct utsname u;
    const void *str = NULL;
    int k = 0;
    for (char *i = target; *i != '\0'; i++)
    {

        if (*i != 37)
        {
            line[k++] = *i;
            continue;
        }
        i++;
        if (*i < 96 || *i > 111)
            continue;

        switch (*i)
        {
        case 'a':
            pam_get_item(pamh, PAM_SERVICE, &str);
            break;

        case 'b':
            pam_get_item(pamh, PAM_USER, &str);
            break;

        case 'c':
            pam_get_item(pamh, PAM_TTY, &str);
            break;
        case 'd':
            pam_get_item(pamh, PAM_SERVICE, &str);
            if (strcmp(str, "login") == 0)
            {
                pam_syslog(pamh, LOG_INFO, "When the service is LOGIN, the variable cannot be \%d");
                str = "";
                break;
            }
            pam_get_item(pamh, PAM_RHOST, &str);

            break;
        case 'e':
            if (uname(&u) < 0)
                str = "";
            else
                str = u.nodename;
            break;
        default:
            pam_syslog(pamh, LOG_INFO, "Unsupported variables:\%%c", *i);
            break;
        }
        strncat(line, str, strlen(str));
        k = strlen(line);
    }

    return PAM_SUCCESS;
}

int auth_main(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
    pam_syslog(pamh, LOG_INFO, "PAM-WXWork v0.2");

    const char *f = NULL;
    int retval;
    if (!strncmp(*argv, "file=", 5))
        f = (5 + *argv);

    if (f == NULL)
    {
        pam_syslog(pamh, LOG_ERR, "The PAM module parameters are incorrectly configured.");
        return PAM_AUTH_ERR;
    }
    pam_syslog(pamh, LOG_INFO, "file=%s", f);

    struct wxwork_config wc;
    wc.login_url = NULL;
    wc.agentid = NULL;
    wc.appid = NULL;
    wc.redirect_uri = NULL;
    wc.auth_value = NULL;

    int a = get_wxwork_config(pamh, &wc, f);
    if (a != 0)
    {
        pam_syslog(pamh, LOG_ERR, "failed to set the struct of wxwork");
        return PAM_AUTH_ERR;
    }

    // Format according to wxwork_auth_value of pam_wxwork.conf
    char auth_value[CONF_LINE_MAX] = {'\0'};
    retval = get_pam_item(pamh, wc.auth_value, auth_value);
    if (retval == PAM_AUTH_ERR)
        return retval;
    pam_syslog(pamh, LOG_INFO,"wxwork_auth_value=%s",auth_value);

    int wxwork_full_url_len = printf("%s?appid=%s&agentid=%s&redirect_uri=%s?%s\n", wc.login_url, wc.appid, wc.agentid, wc.redirect_uri, auth_value);
    char *wxwork_full_url = (char *)malloc(sizeof(char) * (wxwork_full_url_len + 1));
    sprintf(wxwork_full_url, "%s?appid=%s&agentid=%s&redirect_uri=%s?%s", wc.login_url, wc.appid, wc.agentid, wc.redirect_uri, auth_value);

    char *key = get_wxwork_key(pamh, wxwork_full_url);
    if (key == NULL)
    {
        pam_syslog(pamh, LOG_ERR, "%s", "failed to get the key of wxwork.");
        return PAM_AUTH_ERR;
    }

    // printf("wxwork key:%s\n", key);
    // pam_syslog(pamh, LOG_INFO, "wxwork qrcode key:%s", key);

    char *qrcode_url = get_wxwork_qrcode(key);
    if (qrcode_url == NULL)
    {
        pam_syslog(pamh, LOG_ERR, "fialed to get the qrcode of wxwork");
        return PAM_AUTH_ERR;
    }
    // printf("wxwork qrcode url:%s\n", qrcode_url);
    // pam_syslog(pamh, LOG_INFO, "wxwork qrcode url:%s", qrcode_url);

    int r = displayQRCode(pamh, qrcode_url);
    if (r != 0)
    {
        pam_syslog(pamh, LOG_ERR, "Failure to generate QR code");
        return PAM_AUTH_ERR;
    }

    char *auth_code = wait_auth_wxwork_qrcode(pamh, key, wxwork_full_url, wc.appid);
    // printf("auth_code: %s\n", auth_code);
    // pam_syslog(pamh, LOG_INFO, ("auth_code: %s", auth_code));

    char *redirect_url = create_redirect_url(pamh, wxwork_full_url, auth_code, wc.appid);
    // printf("redirect_url: %s\n", redirect_url);
    // pam_syslog(pamh, LOG_INFO, "redirect_url: %s", redirect_url);
    int ret = req_auth_server(pamh, redirect_url);

    free(key);
    free(qrcode_url);
    free(auth_code);
    free(redirect_url);

    free(wc.login_url);
    free(wc.agentid);
    free(wc.appid);
    free(wc.redirect_uri);
    free(wc.auth_value);
    free(wxwork_full_url);

    if (ret == 0)
        return PAM_SUCCESS;
    else
        return PAM_AUTH_ERR;
}
