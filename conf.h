
struct wxwork_config
{
    char * appid;
    char * agentid;
    char * redirect_uri;
    char * auth_value;
    char * login_url;
};
void set_wxwork_config(struct wxwork_config * wc,char * key,char * value);
int get_wxwork_config(pam_handle_t *pamh,struct wxwork_config * wc,const char * file);
#define WXWORK_URL "https://open.work.weixin.qq.com/wwopen/sso/qrConnect"

#define WXWORK_APPID "wxwork_appid"
#define WXWORK_AGENTID "wxwork_agentid"
#define WXWORK_REDIRECT_URI "wxwork_redirect_uri"
#define WXWORK_AUTH_VALUE "wxwork_auth_value"
#define WXWORK_LOGIN_URL "wxwork_login_url"