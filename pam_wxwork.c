#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <curl/curl.h>
#include <unistd.h>

#include "wxwork.h"
#include "qrcode.h"
#include "conf.h"

/* 为 pam_sm_authenticate
   提供原型*/
#include <security/pam_modules.h>
#include <security/pam_ext.h>

#ifdef __GNUC__
#define UNUSED __attribute__((unused))
#else
#define UNUSED
#endif
int auth_main(pam_handle_t *pamh, int flags, int argc, const char **argv);
/*
    程序说明，
    pam_syslog 输出的日志位于/var/log/secure
*/


int auth_main(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
    pam_syslog(pamh, LOG_INFO, "PAM-wxwork v0.1");

    const char *f = NULL;

     if (!strncmp(*argv, "file=", 5))
        f = (5 + *argv);

    if (f == NULL)
    {
        pam_syslog(pamh, LOG_ERR,"The PAM module parameters are incorrectly configured.");
        return PAM_AUTH_ERR;
    }
        pam_syslog(pamh, LOG_INFO,"file=%s",f);

    struct wxwork_config wc;
    wc.login_url = NULL;
    wc.agentid = NULL;
    wc.appid = NULL;
    wc.redirect_uri = NULL;
    wc.auth_value = NULL;
    wc.login_url = NULL;

    int a = get_wxwork_config(pamh,&wc,f);
    if (a!=0){
        pam_syslog(pamh, LOG_ERR,"failed to set the struct of wxwork");
        return PAM_AUTH_ERR;
    }
    
    int wxwork_full_url_len = printf("%s?appid=%s&agentid=%s&redirect_uri=%s?%s\n",wc.login_url,wc.appid,wc.agentid,wc.redirect_uri,wc.auth_value);
    char * wxwork_full_url = (char *)malloc(sizeof (char) *(wxwork_full_url_len+1));
    sprintf(wxwork_full_url,"%s?appid=%s&agentid=%s&redirect_uri=%s?%s",wc.login_url,wc.appid,wc.agentid,wc.redirect_uri,wc.auth_value);
    
    char *key = get_wxwork_key(pamh, wxwork_full_url);
    if (key == NULL)
    {
        pam_syslog(pamh, LOG_ERR, "%s", "failed to get the key of wxwork.");
        return PAM_AUTH_ERR;
    }

    printf("wxwork key:%s\n", key);
    pam_syslog(pamh,LOG_INFO,"wxwork qrcode url:%s\n", key);

    char *qrcode_url = get_wxwork_qrcode(key);
    if (qrcode_url == NULL)
    {
        pam_syslog(pamh, LOG_ERR, "fialed to get the qrcode of wxwork");
        return PAM_AUTH_ERR;
    }
    printf("wxwork qrcode url:%s\n", qrcode_url);
    pam_syslog(pamh,LOG_INFO,"wxwork qrcode url:%s\n", qrcode_url);

    int r = displayQRCode(pamh, qrcode_url);
    if ( r!= 0)
    {
        pam_syslog(pamh, LOG_ERR, "%s", "二维码生成失败");
        return PAM_AUTH_ERR;
    }


    char *auth_code = wait_auth_wxwork_qrcode(key, wxwork_full_url, wc.appid);
    char *redirect_url = create_redirect_url(wxwork_full_url, auth_code, wc.appid);
    printf("redirect_url: %s\n", redirect_url);
pam_syslog(pamh,LOG_INFO,"redirect_url: %s\n", redirect_url);
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
    return PAM_SUCCESS;
    if (ret == 0)
        return PAM_SUCCESS;
    else
        return PAM_AUTH_ERR;
}


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
