
#include "url.h"
#include "conf.h"
#include "file.h"
#include "strings.h"
int enable_short_url(pam_handle_t *pamh, char * smaller_qrcode, char *url)
{
    const void *str = NULL;
    pam_get_item(pamh, PAM_SERVICE, &str);
    char *line = calloc(sizeof(char), strlen(smaller_qrcode) + 1);
    for (int i = 0, j = 0; i < strlen(smaller_qrcode) + 1; i++)
    {
        if (!(smaller_qrcode[i] == ',' || smaller_qrcode[i] == '\0'))
        {
            line[j++] = smaller_qrcode[i];
            continue;
        }
        line[j + 1] = '\0';

        char *l = line;
        for (char *service = (char*)str; service != NULL;)
        {
            if ((*service++ == *l++))
            {
                free(line);
                return 0;
            }
            break;
        }
        j = 0;
    }
    free(line);
    return 1;
}
char * get_short_url(pam_handle_t *pamh, char *short_url_api, char *url)
{
    char * get_url = calloc(sizeof (char),strlen(short_url_api) +strlen(url)+1 );
    strncat(get_url,short_url_api,strlen(short_url_api));
    strncat(get_url,url,strlen(url));
    int retval = 0;
    CURL *curl;
    CURLcode res;
    FILE *curl_tmp_file = fopen("/tmp/pam_wxwork", "rw+");
    struct curl_slist *http_header = NULL;

    curl = curl_easy_init();
    if (!curl)
    {
        pam_syslog(pamh,LOG_ERR,"curl init failed");
        return NULL;
    }

    char *errbuf = (char *)malloc(10200);
    curl_easy_setopt(curl, CURLOPT_URL, get_url);          // url地址
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1); //不检查ssl，可访问https
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1); //不检查ssl，可访问https
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);        //打印调试信息
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, curl_tmp_file);
    // http_header = curl_slist_append(NULL, "Content-Type:application/json;charset=UTF-8");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, http_header);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);

    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1); //设置为非0,响应头信息location
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 20);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20); //接收数据时超时设置，如果10秒内数据未

    res = curl_easy_perform(curl);

    switch (res)
    {
    case CURLE_OK:
        break;
    case CURLE_UNSUPPORTED_PROTOCOL:
        pam_syslog(pamh, LOG_ERR, "url: %s,error: :%s", url, "An unsupported protocol specified by the header of the URL");
        return NULL;

    case CURLE_COULDNT_CONNECT:
        pam_syslog(pamh, LOG_ERR, "url: %s,error: :%s", url, "Unable to connect to a remote host or agent");
        return NULL;

    case CURLE_HTTP_RETURNED_ERROR:
        pam_syslog(pamh, LOG_ERR, "url: %s,error: :%s", url, "HTTP return error");
        return NULL;

    case CURLE_READ_ERROR:
        pam_syslog(pamh, LOG_ERR, "url: %s,error: :%s", url, "read error");
        return NULL;

    case CURLE_WRITE_ERROR:
        pam_syslog(pamh, LOG_ERR, "url: %s,error: :%s", url, "write error");
        return NULL;

    default:
        pam_syslog(pamh, LOG_ERR, "url: %s,return:%d", url, res);
        pam_syslog(pamh, LOG_ERR, "error::%s", errbuf);
        free(errbuf);
        return NULL;
    }
    long buflen = ftell(curl_tmp_file);
    char *body = (char *)malloc(buflen);
    fseek(curl_tmp_file, 0, SEEK_SET);
    fread(body, buflen, 1, curl_tmp_file);
    free(get_url);

    // 由转短链服务器必须是https://开头，否则无法继续
    if (strindex_x(body, "https://",0)!=0 )
    {
        pam_syslog(pamh,LOG_ERR, "API:%sIs not supported,The result of the request must be https://",short_url_api);
        retval=1;
    }
    if (strlen(body)>strlen(url) ){
        pam_syslog(pamh,LOG_ERR, "This body is too long,",short_url_api);
        retval=1;
    }


    if (retval ==1){
        pam_syslog(pamh,LOG_ERR," Keep using %s",url);
    }else{
        url=realloc(url,sizeof (char)*(strlen(body)+1));
        strcpy(url,body);
    }

    // 释放
    free(errbuf);
    free(body);

    curl_easy_cleanup(curl);
    fclose(curl_tmp_file);
    return url;

}