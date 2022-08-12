#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <curl/curl.h>
#include <unistd.h>

#include "wxwork.h"
#include "qrcode.h"

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
    printf("验证进入主函数\n");

    char url[] = "https://open.work.weixin.qq.com/wwopen/sso/qrConnect?appid=ww49cfd580625083c9&agentid=1000014&redirect_uri=http://zhiweiadserver.xunyang.site:16823/server_auth?state=666";
    char *appid = "ww49cfd580625083c9";
    char *key = get_wxwork_key(pamh, url);
    if (key == NULL)
    {
        pam_syslog(pamh, LOG_ERR, "%s", "获取企业微信key失败");
        return PAM_AUTH_ERR;
    }

        printf("企业微信二维码链接:");

    char *qrcode_url = get_wxwork_qrcode(key);
        printf("%s\n", qrcode_url);

    if(qrcode_url ==NULL){
        pam_syslog(pamh, LOG_ERR, "企业微信二维码获取失败");
        return PAM_AUTH_ERR;
    }
    printf("%s\n",qrcode_url);

    if (displayQRCode(pamh,"https://open.work.weixin.qq.com/wwopen/sso/confirm2?k=1d647a379b4a92ba") == 0)
    {
        // printf("企业微信二维码链接:%s\n", qrcode_url);
        return PAM_SUCCESS;
    }else{
        pam_syslog(pamh, LOG_ERR, "%s", "二维码生成失败");
        return PAM_AUTH_ERR;
    }

    // free(qrcode_url);
    // free(key);
    // free (appid);
    
    return PAM_SUCCESS;

    // char *auth_code = wait_auth_wxwork_qrcode(key, url, appid);

    // char *redirect_url = create_redirect_url(url, auth_code, appid);

   
    // free(auth_code);
    // free(redirect_url);

    // if (req_auth_server(pamh, redirect_url) == 1)
    //     return PAM_SUCCESS;
    // else
    //     return PAM_AUTH_ERR;
}

// int check(pam_handle_t *pamh, int flags, int argc, const char **argv)
// {

//     char file[] = "/tmp/file";

//     pam_syslog(pamh, LOG_INFO, "%s 验证开始",file);
//     pam_syslog(pamh, LOG_INFO, "%s 验证开始了",file);

//     char * content = (char *)malloc(1024);
//     FILE *f = fopen(file, "r");
//     if (f == NULL)
//     {
//         pam_syslog(pamh, LOG_ERR, "%s 打开失败", file);
//         return PAM_AUTHINFO_UNAVAIL;
//     }
//     fgets(content, sizeof content, f);
//     if (content[strlen(content) - 1] == '\n')
//     {
//         content[strlen(content) - 1] = 0;
//     }
//     pam_syslog(pamh, LOG_INFO, "%s 内容为%s",file,content);

//     if (strcmp(content, "666") != 0)
//     {
//         pam_syslog(pamh, LOG_ERR, "%s 验证错误",file);
//         return PAM_AUTH_ERR;
//     }
//     fclose(f);
//     free(content);
//     pam_syslog(pamh, LOG_INFO, "%s 验证结束",file);

//     return PAM_SUCCESS;

// }
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
    // printf("112\n");
    // printf("111\n");
    // printf("111\n");
    return PAM_SUCCESS;
}
// int
// pam_sm_close_session (pam_handle_t *pamh , int flags ,
// 		      int argc , const char **argv )
// {
//     printf("111\n");
//     printf("111\n");
//     printf("111\n");
//   return PAM_SUCCESS;
// }
