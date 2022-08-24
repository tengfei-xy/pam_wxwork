#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <curl/curl.h>
#include <unistd.h>
#include <security/pam_modules.h>
#include <security/pam_ext.h>
#include "wxwork.h"
#include "qrcode.h"
#include "conf.h"
#include "url.h"
#include "pam.h"
#include "file.h"

int watch_qr(pam_handle_t *pamh, const char *f);
int scan_qr(pam_handle_t *pamh);
int auth_main(pam_handle_t *pamh, int flags, int argc, const char **argv);

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

int auth_main(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
    const char *f;

    int t = get_pam_target(pamh, argv, &f);

    switch (t)
    {
    case 0:
        return watch_qr(pamh, f);
        break;
    case 1:
        return scan_qr(pamh);
        break;

    default:
        watch_qr(pamh, f);
        scan_qr(pamh);
        break;
    }
}

int watch_qr(pam_handle_t *pamh, const char *f)
{
    pam_syslog(pamh, LOG_INFO, "PAM-WXWork v0.3");
    int retval;
    struct wxwork_config wc;
    wc.login_url = NULL;
    wc.agentid = NULL;
    wc.appid = NULL;
    wc.redirect_uri = NULL;
    wc.auth_value = NULL;
    wc.smaller_qrcode = NULL;
    wc.short_url_api = NULL;

    pam_syslog(pamh,LOG_ERR,"file=%s",f);

    retval = get_config(pamh, &wc, f);
    if (retval != 0)
    {
        pam_info(pamh, "failed to set the struct of wxwork");
        return PAM_AUTH_ERR;
    }

    // Format according to wxwork_auth_value of pam_wxwork.conf
    char auth_value[CONF_LINE_MAX] = {'\0'};
    retval = get_pam_item(pamh, wc.auth_value, auth_value);
    if (retval != 0)
    {
        pam_syslog(pamh, LOG_ERR, "failed to get the conf of file");
        return PAM_AUTH_ERR;
    }
    // pam_syslog(pamh, LOG_INFO, "wxwork_auth_value=%s", auth_value);

    int wxwork_full_url_len = printf("%s?appid=%s&agentid=%s&redirect_uri=%s?%s\n", wc.login_url, wc.appid, wc.agentid, wc.redirect_uri, auth_value);
    char *wxwork_full_url = (char *)malloc(sizeof(char) * (wxwork_full_url_len + 1));
    sprintf(wxwork_full_url, "%s?appid=%s&agentid=%s&redirect_uri=%s?%s", wc.login_url, wc.appid, wc.agentid, wc.redirect_uri, auth_value);

    char *key = get_wxwork_key(pamh, wxwork_full_url);
    if (key == NULL)
    {
        pam_syslog(pamh, LOG_ERR, "%s", "failed to get the key of wxwork.");
        return PAM_AUTH_ERR;
    }
    // pam_syslog(pamh, LOG_INFO, "wxwork qrcode key:%s", key);

    char *qrcode_url = get_wxwork_qrcode(key);
    if (qrcode_url == NULL)
    {
        pam_syslog(pamh, LOG_ERR, "failed to get the qrcode of wxwork");
        return PAM_AUTH_ERR;
    }

    // 判断服务名是否需要转短链,
    if (enable_short_url(pamh, wc.smaller_qrcode, qrcode_url) == 0)
    {
        qrcode_url = get_short_url(pamh, wc.short_url_api, qrcode_url);
    }
    // pam_syslog(pamh, LOG_INFO, "wxwork qrcode url:%s", qrcode_url);

    retval = write_url(key, wxwork_full_url, wc.appid);
    if (retval != 0)
    {
        pam_syslog(pamh, LOG_ERR, "failed to write data from file");
        return PAM_AUTH_ERR;
    }

    retval = displayQRCode(pamh, qrcode_url);
    if (retval != 0)
    {
        pam_syslog(pamh, LOG_ERR, "Failure to generate QR code");
        return PAM_AUTH_ERR;
    }
    free(key);
    free(qrcode_url);
    free(wc.login_url);
    free(wc.agentid);
    free(wc.appid);
    free(wc.redirect_uri);
    free(wc.auth_value);
    free(wxwork_full_url);

    return PAM_SUCCESS;
}
int scan_qr(pam_handle_t *pamh)
{
    int retval = 0;
    char key[ORD_LEN] = {'\0'};
    char url[URL_LEN] = {'\0'};
    char appid[ORD_LEN] = {'\0'};
    
    retval = read_url(key, url, appid);

    if (retval != 0)
    {
        pam_syslog(pamh, LOG_ERR, "failed to write data from file");
        return PAM_AUTH_ERR;
    }

    char *auth_code = wait_auth_wxwork_qrcode(pamh, key, url, appid);
    // pam_syslog(pamh, LOG_INFO, ("auth_code: %s", auth_code));

    char *redirect_url = create_redirect_url(pamh, url, auth_code, appid);
    // pam_syslog(pamh, LOG_INFO, "redirect_url: %s", redirect_url);

    retval = req_auth_server(pamh, redirect_url);

    free(auth_code);
    free(redirect_url);

    if (retval != 0)
        return PAM_AUTH_ERR;
    return PAM_SUCCESS;
}